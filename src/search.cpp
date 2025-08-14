/*
  This file is part of the Schoenemann chess engine written by Jochengehtab

  Copyright (C) 2024-2025 Jochengehtab

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cmath>
#include <chrono>
#include <cassert>
#include <memory>

#include "search.h"
#include "see.h"
#include "tune.h"
#include "tunables.h"

DEFINE_PARAM(lmrBase, 80, 50, 105);
DEFINE_PARAM(lmrDivisor, 250, 200, 280);

int Search::pvs(int alpha, int beta, int depth, const int ply, Board &board, bool cutNode) {
    assert(-EVAL_INFINITE <= alpha && alpha < beta && beta <= EVAL_INFINITE);

    // Setup some search constants
    nodes++;

    const bool root = ply == 0;
    const bool pvNode = beta > alpha + 1;

    if (pvNode) {
        stack[ply].pvLength = 0;
    }

    // We check for a timeout
    if (timeManagement.shouldStopSoft(start) || nodes >= nodeLimit) {
        shouldStop = true;
    }

    // If depth is 0 we drop into qs to get a neutral position
    if (depth <= 0) {
        return qs(alpha, beta, board, ply);
    }

    // Make sure that depth is always lower than MAX_PLY
    if (depth >= MAX_PLY - 1) {
        depth = MAX_PLY - 1;
    }

    if (!root && shouldExit(board, ply)) {
        return ply >= MAX_PLY - 1 && !board.inCheck() ? evaluate(board) : 0;
    }

    const bool isSingularSearch = stack[ply].excludedMove != Move::NULL_MOVE;

    // Transposition Table lookup
    const Hash *entry = transpositionTable.getHash(board.hash());
    bool ttHit = false;
    int hashedScore = EVAL_NONE;
    int hashedDepth = 0;
    Move hashedMove = Move::NULL_MOVE;
    const int oldAlpha = alpha;
    Bound hashedType = Bound::NONE;

    if (!isSingularSearch && entry != nullptr && entry->key == board.hash()) {
        ttHit = true;
        hashedScore = tt::scoreFromTT(entry->score, ply);
        hashedType = entry->type;
        hashedDepth = entry->depth;
        hashedMove = entry->move;
    }

    // Check if we can return our score that we got from the transposition table
    if (!pvNode && !root && hashedDepth >= depth && ((hashedType == Bound::UPPER && hashedScore <= alpha) ||
                                                     (hashedType == Bound::LOWER && hashedScore >= beta) ||
                                                     hashedType == EXACT)) {
        return hashedScore;
    }

    const bool inCheck = board.inCheck();

    int staticEval;

    // We check if we have the static eval already stored in the transposition table.
    // If that is the case, we use this eval elsewhere we have to evaluate the position
    if (ttHit) {
        staticEval = entry->eval;
    } else {
        staticEval = evaluate(board);
    }

    const int rawEval = staticEval;
    staticEval = std::clamp(history.correctEval(staticEval, board), -EVAL_INFINITE + MAX_PLY, EVAL_INFINITE - MAX_PLY);

    // Save statick eval into the SearchStack. This is important for the improving flag
    if (!inCheck) {
        stack[ply].staticEval = staticEval;
    } else {
        stack[ply].staticEval = EVAL_NONE;
    }

    bool improving = false;

    // Check if we improved over one move
    // That means we check if our evaluation is greater than two plies ago
    if (ply > 2 && staticEval > stack[ply - 2].staticEval && stack[ply - 2].staticEval != EVAL_NONE) {
        improving = true;
    } else if (ply > 4 && staticEval > stack[ply - 4].staticEval && stack[ply - 4].staticEval != EVAL_NONE) {
        improving = true;
    }

    // Reverse Futility Pruning
    // If we subtract a margin from our static evaluation, and it is still far
    // above beta, we can assume that the node will fail high (beta cutoff) and prune it
    if (!isSingularSearch && !inCheck && !pvNode && depth < 9 && staticEval - rfpSub * (depth - improving) >= beta) {
        // By tweaking the return value with beta, we try to adjust it more to the window.
        // As we do this, we make the value more inaccurate, but we are potentially adjusting
        // it more to our window which can probably produce a fail high
        return (staticEval + beta) / 2;
    }

    // Razoring
    if (!isSingularSearch && !pvNode && !board.inCheck() && depth <= 1) {
        const int ralpha = alpha - razorAlpha - depth * razorDepthMul;

        if (staticEval < ralpha) {
            int qscore;
            if (depth == 1 && ralpha < alpha) {
                qscore = qs(alpha, beta, board, ply);
                return qscore;
            }

            qscore = qs(ralpha, ralpha + 1, board, ply);

            if (qscore <= ralpha) {
                return qscore;
            }
        }
    }

    // Null Move Pruning
    // If our position is excellent we pass a move to our opponent.
    // We search this with a full window and a reduced search depth.
    // If the search returns a score above beta we can cut that off.
    if (!isSingularSearch && !pvNode && depth > 3 && !inCheck && staticEval >= beta) {
        const int nmpDepthReduction = nmpBase + depth / nmpDiv;
        stack[ply].previousMovedPiece = PieceType::NONE;
        stack[ply].previousMove = Move::NULL_MOVE;

        board.makeNullMove();
        const int score = -pvs(-beta, -alpha, depth - nmpDepthReduction, ply + 1, board, !cutNode);
        board.unmakeNullMove();

        if (score >= beta) {
            return score;
        }
    }

    // Internal Iterative Reduction
    // If we have no hashed move, we expect that our move ordering is worse
    // so we reduce our depth
    if (!isSingularSearch && hashedMove == Move::NULL_MOVE && pvNode && hashedDepth > depth && !inCheck && depth > 3) {
        depth--;
    }

    Movelist moveList;
    movegen::legalmoves(moveList, board);

    int scoreMoves[MAX_MOVES] = {};
    // Sort the list
    MoveOrder::orderMoves(&history, moveList, entry, stack[ply].killerMove, stack, board, scoreMoves, ply);

    // Set up values for the search
    int score = 0;
    int bestScore = -EVAL_INFINITE;
    int moveCount = 0;
    int quietMoveCount = 0;
    Move bestMoveInPVS = Move::NULL_MOVE;
    Move quietMoves[MAX_MOVES] = {};

    for (int i = 0; i < moveList.size(); i++) {
        const Move move = MoveOrder::sortByScore(moveList, scoreMoves, i);

        if (move == stack[ply].excludedMove) {
            continue;
        }

        // We consider a move quiet if it isn't a capture or a promotion
        const bool isQuiet = !board.isCapture(move) && move.typeOf() != Move::PROMOTION;

        // Move Pruning
        if (!root && bestScore > -EVAL_MATE_IN_MAX_PLY) {
            // Late Move Pruning
            // If we have a quiet position, and we already have made almost
            // all of our moves we skip the move
            if (!pvNode && isQuiet && !inCheck && moveCount >= 4 + 3 * depth * depth) {
                continue;
            }

            // Futility Pruning
            // We skip quiet moves that have less potential to raise alpha
            if (!inCheck && isQuiet && staticEval + fpAdd + fpMul * depth < alpha && depth < 6) {
                continue;
            }

            // Static Exchange evaluation (SEE)
            // We look at a move if it returns a negative result form SEE.
            // That means when the result is positive the opponent is winning the exchange on
            // the target square of the move. If the move is not a capture then we make a bigger cutoff.
            if (!pvNode && depth < 4 && !SEE::see(board, move, isQuiet ? seeQuiet : seeNonQuiet)) {
                continue;
            }
        }

        int extensions = 0;

        if (!isSingularSearch &&
            hashedMove == move &&
            depth > 5 &&
            hashedDepth >= depth - seDepthSub &&
            hashedType != Bound::UPPER &&
            std::abs(hashedScore) < EVAL_MATE_IN_MAX_PLY &&
            !root) {
            const int singularBeta = hashedScore - depth * 2;
            const std::uint8_t singularDepth = (depth - seNewDepthSub) / 2;

            stack[ply].excludedMove = move;
            const int singularScore = pvs(singularBeta - 1, singularBeta, singularDepth, ply, board, cutNode);
            stack[ply].excludedMove = Move::NULL_MOVE;

            if (singularScore < singularBeta) {
                extensions++;
                // If we aren't in a pvNode and our score plus some margin
                // is still less than our singular beta when can extend further
                if (!pvNode && singularScore + 10 < singularBeta) {
                    extensions++;
                }
            } else if (singularBeta >= beta) {
                return singularBeta;
            } else if (hashedScore >= beta) {
                extensions -= 1;
            }
        }

        stack[ply].previousMovedPiece = board.at(move.from()).type();
        stack[ply].previousMove = move;

        board.makeMove(move);
        moveCount++;

        extensions += board.inCheck();

        if (isQuiet) {
            quietMoves[quietMoveCount] = move;
            quietMoveCount++;
        }

        // PVS
        // We assume our first move is the best move so we search this move with a full window
        if (moveCount == 1) {
            score = -pvs(-beta, -alpha, depth - 1 + extensions, ply + 1, board, !cutNode);
        } else {
            int depthReduction = 0;

            // Late Move Reductions (LMR)
            // Since our assumption is that the first move is the best move we search all other
            // moves with a lower depth. And we also assume that our move ordering is good the
            // more moves we made the higher our depth reduction will go
            if (depth > 2) {
                // Get the initial reduction from the reduction table
                depthReduction = reductions[depth][moveCount];

                // When we are in a pv node we want to search with a higher depth
                // so we decrease the depth reduction
                depthReduction -= pvNode;

                depthReduction += cutNode;

                depthReduction = std::clamp(depthReduction, 0, depth - 1);
            }

            // Since we assumed that our first move was the best we search every other
            // move with a zero window
            score = -pvs(-alpha - 1, -alpha, depth - depthReduction - 1 + extensions, ply + 1, board, true);

            // If the score is outside the window we need to research with full window
            if (score > alpha && (score < beta || depthReduction > 0)) {
                score = -pvs(-beta, -alpha, depth - 1 + extensions, ply + 1, board, !cutNode);
            }
        }

        board.unmakeMove(move);

        assert(score > -EVAL_INFINITE && score < EVAL_INFINITE);

        if (shouldStop && rootBestMove != Move::NULL_MOVE) {
            return 0;
        }

        if (score > bestScore) {
            bestScore = score;
            if (score > alpha) {
                alpha = score;
                bestMoveInPVS = move;

                // If we are ate the root we set the bestMove
                if (root) {
                    // Update the score of the root move
                    for (int x = 0; x < rootMoveListSize; x++) {
                        if (rootMoveList[x].move == move) {
                            rootMoveList[x].score = score;
                            break;
                        }
                    }
                    rootBestMove = move;
                }

                // Update the pvLine
                if (pvNode) {
                    updatePv(ply, move);
                }
            }

            // Beta cutoff
            if (score >= beta) {
                if (isQuiet) {
                    // Killer Move
                    // If the move is quiet but still causes a fail high which is very unusual,
                    // we store the move and later rank it high up in the move ordering
                    stack[ply].killerMove = move;

                    // Quiet History
                    const int quietHistoryBonus = std::min(qhBB + qhBM * depth, static_cast<int>(qhBC));
                    const int quietHistoryMalus = std::min(qhMB + qhMM * depth, static_cast<int>(qhMC));

                    history.updateQuietHistory(board, move, quietHistoryBonus);

                    // Continuation History
                    const int continuationHistoryBonus = std::min(chBB + chBM * depth, static_cast<int>(chBC));
                    const int continuationHistoryMalus = std::min(chMB + chMM * depth, static_cast<int>(chMC));

                    history.updateContinuationHistory(board.at(move.from()).type(), move, continuationHistoryBonus, ply,
                                                      stack);

                    // History malus
                    // Since we don't want the history scores to be over saturated, and we want to
                    // penalize all other quiet moves since they are not promising, we apply a negative
                    // bonus to all other quiet moves so they get lower ranked in move ordering
                    for (int x = 0; x < quietMoveCount; x++) {
                        Move madeMove = quietMoves[x];
                        if (madeMove == bestMoveInPVS) {
                            continue;
                        }

                        history.updateQuietHistory(board, madeMove, -quietHistoryMalus);
                        history.updateContinuationHistory(board.at(madeMove.from()).type(), madeMove,
                                                          -continuationHistoryMalus, ply, stack);
                    }
                }
                break;
            }
        }
    }

    if (moveCount == 0) {
        if (isSingularSearch) {
            return -EVAL_INFINITE;
        }
        bestScore = inCheck ? matedIn(ply) : 0;
    }

    assert(bestScore > -EVAL_INFINITE && bestScore < EVAL_INFINITE);

    const bool failHigh = score >= beta;
    const bool failLow = alpha == oldAlpha;
    const Bound flag = failHigh ? Bound::LOWER : !failLow ? Bound::EXACT : Bound::UPPER;
    if (!isSingularSearch) {
        transpositionTable.storeHash(board.hash(), depth, flag, tt::scoreToTT(bestScore, ply), bestMoveInPVS,
                                     rawEval);
    }

    if (!inCheck && (bestMoveInPVS == Move::NULL_MOVE || !board.isCapture(bestMoveInPVS)) && (
            (flag == Bound::EXACT) || (flag == Bound::UPPER && bestScore <= staticEval) || (
                flag == Bound::LOWER && bestScore > staticEval))) {
        const int bonus = std::clamp((int) (bestScore - staticEval) * depth * 180 / 768, -CORRHIST_LIMIT / 4,
                                     CORRHIST_LIMIT / 4);
        history.updatePawnCorrectionHistory(bonus, board, 768);
    }

    return bestScore;
}

int Search::qs(int alpha, int beta, Board &board, const int ply) {
    assert(alpha >= -EVAL_INFINITE && alpha < beta && beta <= EVAL_INFINITE);

    nodes++;

    const bool pvNode = beta > alpha + 1;

    if (pvNode) {
        stack[ply].pvLength = 0;
    }

    if (timeManagement.shouldStopSoft(start) || nodes >= nodeLimit) {
        shouldStop = true;
    }

    if (shouldExit(board, ply)) {
        return ply >= MAX_PLY - 1 && !board.inCheck() ? evaluate(board) : 0;
    }

    // Transposition Table lookup
    const Hash *entry = transpositionTable.getHash(board.hash());
    int hashedScore = EVAL_NONE;
    Bound hashedType = Bound::NONE;

    if (entry != nullptr) {
        hashedScore = tt::scoreFromTT(entry->score, ply);
        hashedType = entry->type;
    }

    // Check if we can return our score that we got from the transposition table
    if (!pvNode && ((hashedType == Bound::UPPER && hashedScore <= alpha) ||
                    (hashedType == Bound::LOWER && hashedScore >= beta) ||
                    hashedType == EXACT)) {
        return hashedScore;
    }

    const int standPat = evaluate(board);

    if (standPat >= beta) {
        return standPat;
    }

    if (alpha < standPat) {
        alpha = standPat;
    }

    Movelist moveList;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moveList, board);

    int bestScore = standPat;
    Move bestMoveInQs = Move::NULL_MOVE;
    int moveCount = 0;
    const bool isSingularSearch = stack[ply].excludedMove != Move::NULL_MOVE;

    for (const Move &move: moveList) {
        // Static Exchange evaluation (SEE)
        // We look at a move if it returns a negative result form SEE.
        // That means when the result is positive the opponent is winning the exchange on
        // the target square of the move.
        if (!SEE::see(board, move, 0)) {
            continue;
        }

        stack[ply].previousMovedPiece = board.at(move.from()).type();
        stack[ply].previousMove = move;

        board.makeMove(move);
        moveCount++;

        const int score = -qs(-beta, -alpha, board, ply + 1);
        assert(score < EVAL_INFINITE && score > -EVAL_INFINITE);

        board.unmakeMove(move);

        // Our current Score is better than the previous bestScore so we update it
        if (score > bestScore) {
            bestScore = score;

            // Score is greater than alpha so we update alpha to the score
            if (score > alpha) {
                alpha = score;

                // Update pvLine
                if (pvNode) {
                    updatePv(ply, move);
                }

                bestMoveInQs = move;
            }

            // Beta cutoff
            if (score >= beta) {
                break;
            }
        }
    }

    // Checks for checkmate
    if (bestScore == -EVAL_INFINITE) {
        assert(moveCount == 0);
        bestScore = matedIn(ply);
    }

    if (!isSingularSearch) {
        const bool failHigh = bestScore >= beta;
        transpositionTable.storeHash(board.hash(), 0, failHigh ? Bound::LOWER : Bound::UPPER,
                                     tt::scoreToTT(bestScore, ply), bestMoveInQs, standPat);
    }

    return bestScore;
}

void Search::iterativeDeepening(Board &board, const SearchParams &params) {
    start = std::chrono::steady_clock::now();
    timeManagement.calculateTimeForMove();

    if (params.isInfinite || nodeLimit != NO_NODE_LIMIT) {
        timeManagement.isInfiniteSearch = true;
    }

    rootBestMove = Move::NULL_MOVE;
    Move bestMoveThisIteration = Move::NULL_MOVE;

    nodes = 0;

    int alpha = -EVAL_INFINITE;
    int beta = EVAL_INFINITE;
    int delta = aspBase;

    // Generate all legal root moves to later report the correct score
    Movelist moveList;
    movegen::legalmoves(moveList, board);

    // Initialize the rootMoveList
    rootMoveList = std::make_unique<RootMove[]>(moveList.size());

    // Fill every move into the rootMoveList
    for (int i = 0; i < moveList.size(); i++) {
        rootMoveList[i].move = moveList[i];
    }

    // We keep track of the size
    rootMoveListSize = moveList.size();
    const int finalDepth = params.depth == MAX_PLY ? MAX_PLY : params.depth + 1;
    for (int i = 1; i < finalDepth; i++) {
        if ((timeManagement.shouldStopID(start) && !params.isInfinite) || i == MAX_PLY - 1 || nodes >= nodeLimit ||
            shouldStop) {
            break;
        }

        if (i > 7) {
            previousBestScore = currentScore;
        }

        if (i > 3) {
            // Set up the initial aspiration window
            delta = aspBase;
            alpha = std::max(currentScore - delta, -EVAL_INFINITE);
            beta = std::min(currentScore + delta, EVAL_INFINITE);
        }

        while (true) {
            const int newScore = pvs(alpha, beta, i, 0, board, false);

            // Our score did fall inside our bounds so we exit the search
            if (newScore > alpha && newScore < beta) {
                currentScore = newScore;
                break;
            }

            // Fail low
            if (newScore <= alpha) {
                // We narrow beta down to make a fail high more likely
                beta = (alpha + beta) / 2;

                // We make alpha wider to lower the chance of a fail low
                alpha = std::max(alpha - delta, -EVAL_INFINITE);
            }

            // Fail High
            else {
                // We make beta bigger to decrease the chance of another fail high
                // Since fail highs on PV nodes are very strange
                beta = std::min(beta + delta, EVAL_INFINITE);
            }

            // We want to widen the window for the next iteration
            // to increase the chance that our score is inside our bounds
            delta *= 2;
        }

        if (i > 6) {
            // Update the previous best move
            previousBestMove = bestMoveThisIteration;
        }

        // Get the new best move
        bestMoveThisIteration = rootBestMove;

        if (i > 6) {
            timeManagement.updateBestMoveStability(bestMoveThisIteration, previousBestMove);
        }

        if (i > 7) {
            timeManagement.updateEvalStability(currentScore, previousBestScore);
        }

        std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
        if (!params.minimal) {
            std::cout
                << "info depth " << i
                << scoreToUci()
                << " nodes " << nodes
                << " nps " << static_cast<std::uint64_t>(nodes / (elapsed.count() + 1) * 1000)
                << " hashfull " << transpositionTable.estimateHashfull()
                << " time " << static_cast<std::uint64_t>(elapsed.count() + 1)
                << " pv " << getPVLine()
                << std::endl;
        }

        // std::cout << "Time for this move: " << timeForMove << " | Time used: " << static_cast<int>(elapsed.count()) << " | Depth: " << i << " | bestmove: " << bestMove << std::endl;
    }
    if (!params.minimal) {
        std::cout << "bestmove " << uci::moveToUci(bestMoveThisIteration) << std::endl;
    }
    shouldStop = false;
    nodeLimit = NO_NODE_LIMIT;
}

std::string Search::scoreToUci() const {
    int score = EVAL_NONE;

    // Get the score of the best root move
    for (int i = 0; i < rootMoveListSize; i++) {
        if (rootMoveList[i].move == rootBestMove) {
            score = rootMoveList[i].score;
            break;
        }
    }
    if (score >= EVAL_MATE_IN_MAX_PLY) {
        return " mate " + std::to_string((EVAL_MATE - score) / 2 + 1);
    }
    if (score <= -EVAL_MATE_IN_MAX_PLY) {
        return " mate " + std::to_string(-(EVAL_MATE + score) / 2);
    }
    assert(score != EVAL_NONE);
    return " score cp " + std::to_string(score);
}

void Search::initLMR() {
    const double lmrBaseFinal = lmrBase / 100.0;
    const double lmrDivisorFinal = lmrDivisor / 100.0;
    for (int depth = 1; depth < MAX_PLY; depth++) {
        for (int moveCount = 1; moveCount < MAX_MOVES; moveCount++) {
            reductions[depth][moveCount] = static_cast<std::uint8_t>(std::clamp(
                lmrBaseFinal + std::log(depth) * std::log(moveCount) / lmrDivisorFinal, 0.0, 255.0));
        }
    }
}

int Search::scaleOutput(const int rawEval, const Board &board) {
    const int gamePhase = 3 * board.pieces(PieceType::KNIGHT).count() +
                          3 * board.pieces(PieceType::BISHOP).count() +
                          5 * board.pieces(PieceType::ROOK).count() +
                          9 * board.pieces(PieceType::QUEEN).count();

    const int finalEval = rawEval * (materialBase + gamePhase) / materialDiv;

    return std::clamp(finalEval, -EVAL_MATE, EVAL_MATE);
}

int Search::evaluate(const Board &board) const {
    const int rawEval = net.evaluate(board.sideToMove(), board.occ().count());
    const int scaledEval = scaleOutput(rawEval, board);
    return std::clamp(scaledEval, -EVAL_MATE, EVAL_MATE);
}

void Search::updatePv(const int ply, const Move &move) {
    stack[ply].pvLine[0] = move;
    stack[ply].pvLength = stack[ply + 1].pvLength + 1;
    for (int i = 0; i < stack[ply + 1].pvLength; i++) {
        stack[ply].pvLine[i + 1] = stack[ply + 1].pvLine[i];
    }
}

std::string Search::getPVLine() const {
    std::string pvLine;
    for (int i = 0; i < stack[0].pvLength; i++) {
        pvLine += uci::moveToUci(stack[0].pvLine[i]) + " ";
    }
    return pvLine;
}

bool Search::isDraw(const Board &board) {
    return board.isHalfMoveDraw() || board.isRepetition() || board.isInsufficientMaterial();
}

bool Search::shouldExit(const Board &board, const int ply) const {
    return (shouldStop || ply >= MAX_PLY - 1 || isDraw(board)) && rootBestMove != Move::NULL_MOVE;
}

void Search::resetHistory() {
    history.resetHistories();
}
