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

#include "search.h"
#include "see.h"
#include "tune.h"

std::chrono::time_point start = std::chrono::steady_clock::now();

// Aspiration Window
//DEFINE_PARAM_B(aspDelta, 26, 18, 36);
// DEFINE_PARAM_B(aspDivisor, 2, 2, 8); When tuned this triggers crashes :(
//DEFINE_PARAM_B(aspMul, 134, 100, 163);
//DEFINE_PARAM_B(aspDepth, 7, 6, 8);


DEFINE_PARAM_B(lmrBase, 78, 50, 105);
DEFINE_PARAM_B(lmrDivisor, 240, 200, 280);

// Material Scaling
DEFINE_PARAM_B(materialScaleKnight, 3, 2, 4);
DEFINE_PARAM_B(materialScaleBishop, 3, 2, 4);
DEFINE_PARAM_B(materialScaleRook, 5, 4, 6);
DEFINE_PARAM_B(materialScaleQueen, 18, 12, 24);
DEFINE_PARAM_B(materialScaleGamePhaseAdd, 169, 120, 220);
DEFINE_PARAM_B(materialScaleGamePhaseDiv, 269, 220, 320);

int Search::pvs(int alpha, int beta, int depth, int ply, Board &board) {
    assert(-EVAL_INFINITE <= alpha && alpha < beta && beta <= EVAL_INFINITE);
    // Increment nodes by one
    nodes++;

    const bool root = ply == 0;
    const bool pvNode = beta > alpha + 1;

    // Set the pvLength to zero
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

    if (!root) {
        if (shouldStop || ply >= MAX_PLY - 1 || isDraw(board)) {
            return ply >= MAX_PLY - 1 && !board.inCheck() ? evaluate(board) : 0;
        }
    }

    // Transposition Table lookup
    const Hash *entry = transpositionTable.getHash(board.hash());
    const bool ttHit = entry != nullptr;
    int hashedScore = EVAL_NONE;
    int hashedDepth = 0;
    const int oldAlpha = alpha;
    std::uint8_t hashedType = 4;

    if (ttHit && entry->key == board.hash()) {
        hashedScore = tt::scoreFromTT(entry->score, ply);
        hashedType = static_cast<std::uint8_t>(entry->type);
        hashedDepth = static_cast<int>(entry->depth);
    }

    // Check if we can return our score that we got from the transposition table
    if (!pvNode && !root && hashedDepth >= depth && ((hashedType == UPPER_BOUND && hashedScore <= alpha) ||
                                                              (hashedType == LOWER_BOUND && hashedScore >= beta) ||
                                                              (hashedType == EXACT))) {
        return hashedScore;
    }

    const bool inCheck = board.inCheck();
    const int staticEval = evaluate(board);

    Movelist moveList;
    movegen::legalmoves(moveList, board);

    int scoreMoves[MAX_MOVES] = {};
    // Sort the list
    MoveOrder::orderMoves(&history, moveList, entry, stack[ply].killerMove, stack, board, scoreMoves, ply);

    // Set up values for the search
    int score = 0;
    int bestScore = -EVAL_INFINITE;
    int moveCount = 0;
    Move bestMoveInPVS = Move::NULL_MOVE;

    for (int i = 0; i < moveList.size(); i++) {
        const Move move = MoveOrder::sortByScore(moveList, scoreMoves, i);

        board.makeMove(move);
        moveCount++;

        // PVS
        // We assume our first move is the best move so we search this move with a full window
        if (moveCount == 1) {
            score = -pvs(-beta, -alpha, depth - 1, ply + 1, board);
        } else {
            // Since we assumed that our first move was the best we search every other
            // Move with a zero window
            score = -pvs(-alpha - 1, -alpha, depth - 1, ply + 1, board);

            // If the score is outside the window we need to research with full window
            if (score > alpha && score < beta) {
                score = -pvs(-beta, -alpha, depth - 1, ply + 1, board);
            }
        }

        board.unmakeMove(move);

        assert(score > -EVAL_INFINITE && score < EVAL_INFINITE);

        if (shouldStop) {
            return 0;
        }

        if (score > bestScore) {
            bestScore = score;
            if (score > alpha) {
                alpha = score;
                bestMoveInPVS = move;

                // If we are ate the root we set the bestMove
                if (ply == 0) {
                    rootBestMove = move;
                }

                // Update the pvLine
                if (pvNode) {
                    updatePv(ply, move);
                }
            }

            // Beta cutoff
            if (score >= beta) {
                break;
            }
        }
    }

    if (moveCount == 0) {
        bestScore = inCheck ? matedIn(ply) : 0;
    }

    assert(bestScore > -EVAL_INFINITE && bestScore < EVAL_INFINITE);

    const std::uint8_t failHigh = score >= beta;
    const std::uint8_t failLow = alpha == oldAlpha;
    const std::uint8_t flag = failHigh ? LOWER_BOUND : !failLow ? EXACT : UPPER_BOUND;
    transpositionTable.storeHash(board.hash(), depth, flag, tt::scoreToTT(bestScore, ply), bestMoveInPVS, staticEval);

    return bestScore;
}

int Search::qs(int alpha, int beta, Board &board, int ply) {
    // Increment node counter
    nodes++;

    const bool pvNode = beta > alpha + 1;

    // Set the pvLength to zero
    if (pvNode) {
        stack[ply].pvLength = 0;
    }

    assert(alpha >= -EVAL_INFINITE && alpha < beta && beta <= EVAL_INFINITE);
    if (timeManagement.shouldStopSoft(start) || nodes >= nodeLimit) {
        shouldStop = true;
    }

    if (shouldStop || ply >= MAX_PLY - 1 || isDraw(board)) {
        return ply >= MAX_PLY - 1 && !board.inCheck() ? evaluate(board) : 0;
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

    for (const Move &move: moveList) {
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

    return bestScore;
}

void Search::iterativeDeepening(Board &board, const bool isInfinite) {
    start = std::chrono::steady_clock::now();
    timeManagement.calculateTimeForMove();

    if (isInfinite) {
        setTimeInfinite();
    }

    rootBestMove = Move::NULL_MOVE;
    Move bestMoveThisIteration = Move::NULL_MOVE;

    nodes = 0;

    int alpha = -EVAL_INFINITE;
    int beta = EVAL_INFINITE;

    for (int i = 1; i < MAX_PLY; i++) {
        if ((timeManagement.shouldStopID(start) && !isInfinite) || i == MAX_PLY - 1 || nodes == nodeLimit ||
            shouldStop) {
            std::cout << "bestmove " << uci::moveToUci(bestMoveThisIteration) << std::endl;
            break;
        }

        if (i > 7) {
            previousBestScore = scoreData;
        }

        scoreData = pvs(alpha, beta, i, 0, board);

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
            timeManagement.updateEvalStability(scoreData, previousBestScore);
        }

        std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
        std::cout
                << "info depth "
                << i
                << scoreToUci(scoreData) << " nodes "
                << nodes << " nps "
                << static_cast<std::uint64_t>(nodes / (elapsed.count() + 1) * 1000)
                << " hashfull " << transpositionTable.estimateHashfull()
                << " time " << static_cast<std::uint64_t>(elapsed.count() + 1)
                << " pv " << getPVLine()
                << std::endl;

        // std::cout << "Time for this move: " << timeForMove << " | Time used: " << static_cast<int>(elapsed.count()) << " | Depth: " << i << " | bestmove: " << bestMove << std::endl;
    }

    shouldStop = false;
    nodeLimit = -1;
}

std::string Search::scoreToUci(const int &score) {
    if (score >= EVAL_MATE_IN_MAX_PLY) {
        return " mate " + std::to_string((EVAL_MATE - score) / 2 + 1);
    }
    if (score <= -EVAL_MATE_IN_MAX_PLY) {
        return " mate " + std::to_string(-(EVAL_MATE + score) / 2);
    }
    return " score cp " + std::to_string(score);
}

void Search::initLMR() {
    constexpr double lmrBaseFinal = lmrBase / 100.0;
    constexpr double lmrDivisorFinal = lmrDivisor / 100.0;
    for (int depth = 1; depth < MAX_PLY; depth++) {
        for (int moveCount = 1; moveCount < 218; moveCount++) {
            reductions[depth][moveCount] = static_cast<std::uint8_t>(std::clamp(
                lmrBaseFinal + std::log(depth) * std::log(moveCount) / lmrDivisorFinal, 0.0, 255.0));
        }
    }
}

int Search::scaleOutput(const int rawEval, const Board &board) {
    const int gamePhase = materialScaleKnight * board.pieces(PieceType::KNIGHT).count() +
                          materialScaleBishop * board.pieces(PieceType::BISHOP).count() +
                          materialScaleRook * board.pieces(PieceType::ROOK).count() +
                          materialScaleQueen * board.pieces(PieceType::QUEEN).count();

    const int finalEval = rawEval * (materialScaleGamePhaseAdd + gamePhase) / materialScaleGamePhaseDiv;

    return std::clamp(finalEval, -EVAL_MATE, EVAL_MATE);
}

int Search::evaluate(const Board &board) const {
    return std::clamp(net.evaluate(board.sideToMove(), board.occ().count()), -EVAL_MATE, EVAL_MATE);
}

void Search::updatePv(const int ply, const Move &move) {
    stack[ply].pvLine[0] = move;
    stack[ply].pvLength = stack[ply + 1].pvLength + 1;
    for (std::uint16_t i = 0; i < stack[ply + 1].pvLength; i++) {
        stack[ply].pvLine[i + 1] = stack[ply + 1].pvLine[i];
    }
}

std::string Search::getPVLine() const {
    std::string pvLine;
    for (std::uint16_t i = 0; i < stack[0].pvLength; i++) {
        pvLine += uci::moveToUci(stack[0].pvLine[i]) + " ";
    }
    return pvLine;
}

bool Search::isDraw(const Board &board) {
    return board.isHalfMoveDraw() || board.isRepetition() || board.isInsufficientMaterial();
}

void Search::resetHistory() {
    history.resetHistorys();
}

void Search::setTimeInfinite() {
    timeManagement.hardLimit = 99999999;
    timeManagement.softLimit = 99999999;
}
