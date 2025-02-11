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

#include "search.h"

std::chrono::time_point start = std::chrono::steady_clock::now();

DEFINE_PARAM_S(probeCutBetaAdder, 460, 25);
DEFINE_PARAM_S(probeCuteSubtractor, 4, 1);

DEFINE_PARAM_S(iidDepth, 3, 1);

DEFINE_PARAM_S(rfpDepth, 5, 1);
DEFINE_PARAM_S(rfpEvalSubtractor, 80, 6);

DEFINE_PARAM_S(winningDepth, 6, 1);
DEFINE_PARAM_S(winningEvalSubtractor, 97, 20);
DEFINE_PARAM_S(winningDepthMultiplier, 24, 4);

DEFINE_PARAM_S(probeCutMarginAdder, 76, 10);

DEFINE_PARAM_B(winningDepthDivisor, 3, 1, 20);
DEFINE_PARAM_S(winningDepthSubtractor, 4, 1);
DEFINE_PARAM_B(winningCount, 2, 1, 6);

DEFINE_PARAM_B(nmpDepth, 3, 1, 9);
DEFINE_PARAM_S(nmpDepthAdder, 2, 1);
DEFINE_PARAM_B(nmpDepthDivisor, 3, 1, 10);

DEFINE_PARAM_B(razorDepth, 1, 1, 10);
DEFINE_PARAM_S(razorAlpha, 247, 30);
DEFINE_PARAM_S(razorDepthMultiplier, 50, 9);

// PVS - SEE
DEFINE_PARAM_B(pvsSSEDepth, 2, 1, 6);
DEFINE_PARAM_S(pvsSSECaptureCutoff, 92, 10);
DEFINE_PARAM_S(pvsSSENonCaptureCutoff, 18, 10);

// Aspiration Window
DEFINE_PARAM_S(aspDelta, 26, 6);
// DEFINE_PARAM_B(aspDivisor, 2, 2, 8); When tuned this triggers crashes for some reason :(
DEFINE_PARAM_B(aspMultiplier, 134, 1, 450);
DEFINE_PARAM_B(aspEntryDepth, 7, 6, 12);

// Late Move Reductions
DEFINE_PARAM_B(lmrBase, 78, 1, 300);
DEFINE_PARAM_B(lmrDivisor, 240, 1, 700);
DEFINE_PARAM_B(lmrDepth, 2, 1, 7);

DEFINE_PARAM_S(iirReduction, 2, 1);
DEFINE_PARAM_S(fpCutoff, 2, 1);

// Quiet History
DEFINE_PARAM_S(quietHistoryGravityBase, 31, 5);
DEFINE_PARAM_S(quietHistoryDepthMultiplier, 204, 25);
DEFINE_PARAM_S(quietHistoryBonusCap, 1734, 200);
DEFINE_PARAM_B(quietHistoryDivisor, 28711, 10000, 50000);
DEFINE_PARAM_S(quietHistoryMalusBase, 15, 6);
DEFINE_PARAM_S(quietHistoryMalusMax, 1900, 150);
DEFINE_PARAM_S(quietHistoryMalusDepthMultiplier, 171, 25);

// Continuation Hisotry
DEFINE_PARAM_B(continuationHistoryDivisor, 28156, 10000, 50000);
DEFINE_PARAM_S(continuationHistoryMalusBase, 25, 6);
DEFINE_PARAM_S(continuationHistoryMalusMax, 2172, 150);
DEFINE_PARAM_S(continuationHistoryMalusDepthMultiplier, 185, 25);
DEFINE_PARAM_S(continuationHistoryGravityBase, 26, 5);
DEFINE_PARAM_S(continuationHistoryDepthMultiplier, 208, 25);
DEFINE_PARAM_S(continuationHistoryBonusCap, 1959, 200);

// Material Scaling
DEFINE_PARAM_S(materialScaleKnight, 3, 1);
DEFINE_PARAM_S(materialScaleBishop, 3, 1);
DEFINE_PARAM_S(materialScaleRook, 5, 1);
DEFINE_PARAM_S(materialScaleQueen, 18, 3);
DEFINE_PARAM_S(materialScaleGamePhaseAdder, 169, 25);
DEFINE_PARAM_B(materialScaleGamePhaseDivisor, 269, 1, 700);

// Pawn CorrectionHistory
DEFINE_PARAM_B(correctionValueDiv, 59, 1, 600);
DEFINE_PARAM_S(pawnCorrectionHistoryDepthAdder, 157, 20);
DEFINE_PARAM_B(pawnCorrectionHistoryDepthDiv, 909, 1, 4000);
DEFINE_PARAM_B(pawnCorrectionHistoryGravityDiv, 664, 1, 4000);

int Search::pvs(int alpha, int beta, int depth, int ply, Board &board, bool isCutNode)
{
    if (shouldStop)
    {
        return beta;
    }

    if (nodes % 128 == 0)
    {
        if (shouldStopSoft(start) && !isNormalSearch)
        {
            shouldStop = true;
            return beta;
        }

        if (hasNodeLimit)
        {
            if (nodes >= nodeLimit)
            {
                shouldStop = true;
                return beta;
            }
        }
    }

    // Increment nodes by one
    nodes++;

    // Set the pvLength to zero
    stack[ply].pvLength = 0;

    if (board.isHalfMoveDraw() || board.isRepetition() || board.isInsufficientMaterial())
    {
        return 0;
    }

    // Mate distance Pruning
    int mateValueUpper = infinity - ply;

    if (mateValueUpper < beta)
    {
        beta = mateValueUpper;
        if (alpha >= mateValueUpper)
        {
            return mateValueUpper;
        }
    }

    int mateValueLower = -infinity + ply;

    if (mateValueLower > alpha)
    {
        alpha = mateValueLower;
        if (beta <= mateValueLower)
        {
            return mateValueLower;
        }
    }

    // If depth is 0 we drop into qs to get a neutral position
    if (depth == 0)
    {
        return qs(alpha, beta, board, ply);
    }

    const std::uint64_t zobristKey = board.zobrist();
    int hashedScore = 0;
    short hashedType = 0;
    int hashedDepth = 0;
    int staticEval = NO_VALUE;
    Move hashedMove = Move::NULL_MOVE;

    // Get some important search constants
    const bool pvNode = beta > alpha + 1;
    const bool inCheck = board.inCheck();
    const bool isSingularSearch = stack[ply].exludedMove != Move::NULL_MOVE;
    stack[ply].inCheck = inCheck;

    // Get an potential hash entry
    Hash *entry = transpositionTabel.getHash(zobristKey);

    // Check if we this stored position is valid
    const bool isNullptr = entry == nullptr ? true : false;

    if (!isNullptr && !isSingularSearch)
    {
        // If we have a transposition
        // That means that the current board zobrist key
        // is the same as the hash entry zobrist key
        if (zobristKey == entry->key)
        {
            hashedScore = transpositionTabel.scoreFromTT(entry->score, ply);
            hashedType = entry->type;
            hashedDepth = entry->depth;
            staticEval = entry->eval;
            hashedMove = entry->move;
        }

        // Check if we can return a stored score
        if (!pvNode && hashedDepth >= depth && ply > 0 && zobristKey == entry->key)
        {
            if ((hashedType == EXACT) ||
                (hashedType == UPPER_BOUND && hashedScore <= alpha) ||
                (hashedType == LOWER_BOUND && hashedScore >= beta))
            {
                return hashedScore;
            }
        }
    }

    // Initial Iterative Deepening
    if (!isNullptr && zobristKey != entry->key && !inCheck && depth >= iidDepth)
    {
        depth -= iirReduction;
    }

    if (!isSingularSearch && !isNullptr)
    {
        int probCutBeta = beta + probeCutBetaAdder;
        if (hashedDepth >= depth - probeCuteSubtractor && hashedScore >= probCutBeta && std::abs(beta) < infinity)
        {
            return probCutBeta;
        }
    }

    // If no evaluation was found in the transposition table
    // we perform a static evaluation
    if (staticEval == NO_VALUE)
    {
        staticEval = scaleOutput(net.evaluate((int)board.sideToMove(), board.occ().count()), board);
    }

    int rawEval = staticEval;
    staticEval = std::clamp(correctEval(staticEval, board), -infinity + 150, infinity - 150);

    // Update the static Eval on the stack
    stack[ply].staticEval = staticEval;

    bool improving = false;

    if (inCheck)
    {
        improving = false;
    }
    else if (ply > 1 && !stack[ply - 2].inCheck)
    {
        improving = staticEval > stack[ply - 2].staticEval;
    }
    else if (ply > 3 && !stack[ply - 4].inCheck)
    {
        improving = staticEval > stack[ply - 4].staticEval;
    }
    else
    {
        improving = true;
    }

    // Reverse futility pruning
    if (!isSingularSearch && !inCheck && depth <= rfpDepth && staticEval - rfpEvalSubtractor * (depth - improving) >= beta)
    {
        return (staticEval + beta) / 2;
    }

    // Razoring
    if (!isSingularSearch && !pvNode && !board.inCheck() && depth <= razorDepth)
    {
        const int ralpha = alpha - razorAlpha - depth * razorDepthMultiplier;

        if (staticEval < ralpha)
        {
            int qscore;
            if (depth == 1 && ralpha < alpha)
            {
                qscore = qs(alpha, beta, board, ply);
                return qscore;
            }

            qscore = qs(ralpha, ralpha + 1, board, ply);

            if (qscore <= ralpha)
            {
                return qscore;
            }
        }
    }

    // Idea by Laser
    // If we can make a winning move and can confirm that when we do a lower depth search
    // it causes a beta cutoff we can make that beta cutoff
    if (!isSingularSearch && !pvNode && !inCheck && depth >= winningDepth && staticEval >= beta - winningEvalSubtractor - winningDepthMultiplier * depth && std::abs(beta) < infinity)
    {
        int probCutMargin = beta + probeCutMarginAdder;
        int probCutCount = 0;

        Movelist moveList;
        movegen::legalmoves(moveList, board);

        int scoreMoves[218] = {0};
        // Sort the list
        orderMoves(moveList, entry, board, scoreMoves, ply);

        for (int i = 0; i < moveList.size() && probCutCount < winningCount; i++)
        {
            probCutCount++;
            Move move = sortByScore(moveList, scoreMoves, i);

            // We don't want to prune the hashed move
            if (move == hashedMove)
            {
                continue;
            }

            // Update the the piece and the move for continuationHistory
            stack[ply].previousMovedPiece = board.at(move.from()).type();
            stack[ply].previousMove = move;

            board.makeMove(move);

            int score = -pvs(-probCutMargin, -probCutMargin + 1, depth - depth / winningDepthDivisor - winningDepthSubtractor, ply + 1, board, false);

            board.unmakeMove(move);

            if (score >= probCutMargin)
            {
                return score;
            }
        }
    }

    if (!isSingularSearch && !pvNode && !inCheck && depth >= nmpDepth && staticEval >= beta)
    {
        board.makeNullMove();
        int depthReduction = nmpDepthAdder + depth / nmpDepthDivisor;

        // Update the the piece and the move for continuationHistory
        stack[ply].previousMovedPiece = PieceType::NONE;
        stack[ply].previousMove = Move::NULL_MOVE;

        int score = -pvs(-beta, -alpha, depth - depthReduction, ply + 1, board, !isCutNode);
        board.unmakeNullMove();
        if (score >= beta)
        {
            return score;
        }
    }

    Movelist moveList;
    movegen::legalmoves(moveList, board);

    if (moveList.size() == 0)
    {
        return inCheck ? -infinity + ply : 0;
    }

    int scoreMoves[218] = {0};
    // Sort the list
    orderMoves(moveList, entry, board, scoreMoves, ply);

    // Set up values for the search
    int score = 0;
    int bestScore = -infinity;
    int movesMadeCounter = 0;
    int moveCounter = 0;

    short type = LOWER_BOUND;

    Move bestMoveInPVS = Move::NULL_MOVE;
    std::array<Move, 218> movesMade;

    for (int i = 0; i < moveList.size(); i++)
    {
        Move move = sortByScore(moveList, scoreMoves, i);

        if (move == stack[ply].exludedMove)
        {
            continue;
        }

        bool isQuiet = !board.isCapture(move);

        if (!pvNode && move != hashedMove && bestScore > -infinity && depth <= pvsSSEDepth && !see(board, move, (!isQuiet ? -pvsSSECaptureCutoff : -pvsSSENonCaptureCutoff)))
        {
            continue;
        }

        int extensions = 0;

        if (!isSingularSearch && hashedMove == move && depth >= 6 && hashedDepth >= depth - 3 && (hashedType != UPPER_BOUND) && std::abs(hashedScore) < infinity && !(ply == 0))
        {
            const int singularBeta = hashedScore - depth * 2;
            const std::uint8_t singularDepth = (depth - 1) / 2;

            stack[ply].exludedMove = move;
            int singularScore = pvs(singularBeta - 1, singularBeta, singularDepth, ply, board, isCutNode);
            stack[ply].exludedMove = Move::NULL_MOVE;

            if (singularScore < singularBeta)
            {
                extensions++;
                // If we aren't in a pvNode and our score plus some margin is still less then our singular beta when can extend furthur
                if (!pvNode && singularScore + 5 < singularBeta)
                {
                    extensions++;
                }
            }
            
            // Multicut 
            else if (singularBeta >= beta)
            {
                return singularBeta;
            }
        }

        // Update the the piece and the move for continuationHistory
        stack[ply].previousMovedPiece = board.at(move.from()).type();
        stack[ply].previousMove = move;

        board.makeMove(move);

        if (isQuiet)
        {
            movesMade[movesMadeCounter] = move;
            movesMadeCounter++;
        }

        moveCounter++;

        if (board.inCheck())
        {
            extensions++;
        }

        if (moveCounter == 1)
        {
            score = -pvs(-beta, -alpha, depth - 1 + extensions, ply + 1, board, false);
        }
        else
        {
            int lmr = 0;
            if (depth > lmrDepth)
            {
                lmr = reductions[depth][moveCounter];
                lmr -= pvNode;
                lmr += isCutNode * 2;
                lmr = std::clamp(lmr, 0, depth - 1);
            }

            score = -pvs(-alpha - 1, -alpha, depth - lmr - 1 + extensions, ply + 1, board, true);
            if (score > alpha && (score < beta || lmr > 0))
            {
                score = -pvs(-beta, -alpha, depth - 1 + extensions, ply + 1, board, false);
                isCutNode = false;
            }
        }

        board.unmakeMove(move);

        if (score > bestScore)
        {
            bestScore = score;
            if (score > alpha)
            {
                alpha = score;
                type = EXACT;
                bestMoveInPVS = move;

                // If we are ate the root we set the bestMove
                if (ply == 0)
                {
                    rootBestMove = move;
                }

                // Update the pvLine
                if (pvNode)
                {
                    stack[ply].pvLine[0] = move;
                    stack[ply].pvLength = stack[ply + 1].pvLength + 1;
                    for (int x = 0; x < stack[ply + 1].pvLength; x++)
                    {
                        stack[ply].pvLine[x + 1] = stack[ply + 1].pvLine[x];
                    }
                }
            }

            // Beta cutoff
            if (score >= beta)
            {
                if (isQuiet)
                {
                    stack[ply].killerMove = move;
                    int quietHistoryBonus = std::min(static_cast<int>(quietHistoryGravityBase) + static_cast<int>(quietHistoryDepthMultiplier) * depth, static_cast<int>(quietHistoryBonusCap));
                    updateQuietHistory(board, move, quietHistoryBonus);

                    int continuationHistoryBonus = std::min(static_cast<int>(continuationHistoryGravityBase) + static_cast<int>(continuationHistoryDepthMultiplier) * depth, static_cast<int>(continuationHistoryBonusCap));

                    // Update the continuation History
                    updateContinuationHistory(board.at(move.from()).type(), move, continuationHistoryBonus, ply);

                    int quietHistoryMalus = std::min(static_cast<int>(quietHistoryMalusBase) + static_cast<int>(quietHistoryMalusDepthMultiplier) * depth, static_cast<int>(quietHistoryMalusMax));
                    int continuationHistoryMalus = std::min(static_cast<int>(continuationHistoryMalusBase) + static_cast<int>(continuationHistoryMalusDepthMultiplier) * depth, static_cast<int>(continuationHistoryMalusMax));
                    // History malus
                    for (int x = 0; x < movesMadeCounter; x++)
                    {
                        Move madeMove = movesMade[x];
                        if (madeMove == bestMoveInPVS)
                        {
                            continue;
                        }

                        updateQuietHistory(board, madeMove, -(quietHistoryMalus * movesMadeCounter));
                        updateContinuationHistory(board.at(madeMove.from()).type(), madeMove, -(continuationHistoryMalus * movesMadeCounter), ply);
                    }
                }

                break;
            }
        }
    }

    short finalType;
    // Calculate the node type
    if (bestScore >= beta)
    {
        finalType = LOWER_BOUND;
    }
    else if (pvNode && (type == EXACT))
    {
        finalType = EXACT;
    }
    else
    {
        finalType = UPPER_BOUND;
    }
    
    if (!isSingularSearch)
    {
        transpositionTabel.storeEvaluation(zobristKey, depth, finalType, transpositionTabel.scoreToTT(bestScore, ply), bestMoveInPVS, rawEval);
    }

    if (!inCheck && (bestMoveInPVS == Move::NULL_MOVE || !board.isCapture(bestMoveInPVS)) && (finalType == EXACT || (finalType == UPPER_BOUND && bestScore <= staticEval) || (finalType == LOWER_BOUND && bestScore > staticEval)))
    {
        int bonus = std::clamp((int)(bestScore - staticEval) * depth * pawnCorrectionHistoryDepthAdder / pawnCorrectionHistoryDepthDiv, -CORRHIST_LIMIT / 4, CORRHIST_LIMIT / 4);
        updatePawnCorrectionHistory(bonus, board);
    }

    return bestScore;
}

int Search::qs(int alpha, int beta, Board &board, int ply)
{
    if (shouldStop)
    {
        return beta;
    }

    if (nodes % 128 == 0)
    {
        // Check for a timeout
        if (shouldStopSoft(start) && !isNormalSearch)
        {
            shouldStop = true;
            return beta;
        }

        if (hasNodeLimit)
        {
            if (nodes >= nodeLimit)
            {
                shouldStop = true;
                return beta;
            }
        }
    }

    // Increment nodes by one
    nodes++;

    // Set the pvLength to zero
    stack[ply].pvLength = 0;

    // Check for a draw
    if (board.isHalfMoveDraw() || board.isRepetition() || board.isInsufficientMaterial())
    {
        return 0;
    }

    const bool pvNode = beta > alpha + 1;
    const std::uint64_t zobristKey = board.zobrist();

    Hash *entry = transpositionTabel.getHash(zobristKey);
    const bool isNullptr = entry == nullptr ? true : false;
    const bool inCheck = board.inCheck();

    int hashedScore = 0;
    int standPat = NO_VALUE;
    short hashedType = 0;

    if (!isNullptr)
    {
        if (zobristKey == entry->key)
        {
            hashedScore = transpositionTabel.scoreFromTT(entry->score, ply);
            hashedType = entry->type;
            standPat = entry->eval;
        }

        if (!pvNode && transpositionTabel.checkForMoreInformation(hashedType, hashedScore, beta))
        {
            if ((hashedType == EXACT) ||
                (hashedType == UPPER_BOUND && hashedScore <= alpha) ||
                (hashedType == LOWER_BOUND && hashedScore >= beta))
            {
                return hashedScore;
            }
        }
    }

    if (!inCheck && transpositionTabel.checkForMoreInformation(hashedType, hashedScore, standPat))
    {
        standPat = hashedScore;
    }

    if (standPat == NO_VALUE)
    {
        standPat = scaleOutput(net.evaluate((int)board.sideToMove(), board.occ().count()), board);
    }

    int rawEval = standPat;
    standPat = std::clamp(correctEval(standPat, board), -infinity + 150, infinity - 150);

    if (standPat >= beta)
    {
        return standPat;
    }

    if (alpha < standPat)
    {
        alpha = standPat;
    }

    Movelist moveList;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moveList, board);

    int bestScore = standPat;
    Move bestMoveInQs = Move::NULL_MOVE;

    for (Move &move : moveList)
    {
        // Fultiy Prunning
        if (!see(board, move, fpCutoff) && standPat + SEE_PIECE_VALUES[board.at(move.to()).type()] <= alpha)
        {
            continue;
        }

        // Static Exchange Evaluation
        if (!see(board, move, 0))
        {
            continue;
        }

        // Update the the piece and the move for continuationHistory
        stack[ply].previousMovedPiece = board.at(move.from()).type();
        stack[ply].previousMove = move;

        board.makeMove(move);

        int score = -qs(-beta, -alpha, board, ply + 1);

        board.unmakeMove(move);
        // Our current Score is better than the previous bestScore so we update it
        if (score > bestScore)
        {
            bestScore = score;

            // Score is greater than alpha so we update alpha to the score
            if (score > alpha)
            {
                alpha = score;

                // Update pvLine
                stack[ply].pvLine[0] = move;
                stack[ply].pvLength = stack[ply + 1].pvLength + 1;
                for (int i = 0; i < stack[ply + 1].pvLength; i++)
                {
                    stack[ply].pvLine[i + 1] = stack[ply + 1].pvLine[i];
                }

                bestMoveInQs = move;
            }

            // Beta cutoff
            if (score >= beta)
            {
                break;
            }
        }
    }

    // Checks for checkmate
    if (inCheck && bestScore == -infinity)
    {
        return -infinity + ply;
    }

    if (stack[ply].exludedMove == Move::NULL_MOVE)
    {
        transpositionTabel.storeEvaluation(zobristKey, 0, bestScore >= beta ? LOWER_BOUND : UPPER_BOUND, transpositionTabel.scoreToTT(bestScore, ply), bestMoveInQs, standPat);
    }
    transpositionTabel.storeEvaluation(zobristKey, 0, bestScore >= beta ? LOWER_BOUND : UPPER_BOUND, transpositionTabel.scoreToTT(bestScore, ply), bestMoveInQs, rawEval);

    return bestScore;
}

int Search::aspiration(int depth, int score, Board &board)
{
    int delta = aspDelta;
    int alpha = std::max(-infinity, score - delta);
    int beta = std::min(infinity, score + delta);
    double finalASPMultiplier = aspMultiplier / 100.0;

    while (true)
    {
        score = pvs(alpha, beta, depth, 0, board, false);
        if (shouldStopID(start))
        {
            shouldStop = true;
            return score;
        }

        if (score >= beta)
        {
            beta = std::min(beta + delta, infinity);
        }
        else if (score <= alpha)
        {
            beta = (alpha + beta) / 2;
            alpha = std::max(alpha - delta, -infinity);
        }
        else
        {
            break;
        }

        delta *= finalASPMultiplier;
    }

    return score;
}

void Search::iterativeDeepening(Board &board, bool isInfinite)
{
    start = std::chrono::steady_clock::now();
    getTimeForMove();
    rootBestMove = Move::NULL_MOVE;
    Move bestMoveThisIteration = Move::NULL_MOVE;

    isNormalSearch = false;
    bool hasOneLegalMove = Move::NULL_MOVE;

    if (isInfinite)
    {
        isNormalSearch = true;
    }

    nodes = 0;

    for (int i = 1; i <= 256; i++)
    {
        scoreData = i >= aspEntryDepth ? aspiration(i, scoreData, board) : pvs(-infinity, infinity, i, 0, board, false);

        bestMoveThisIteration = rootBestMove;
        hasOneLegalMove = bestMoveThisIteration != Move::NULL_MOVE && bestMoveThisIteration != Move::NO_MOVE;

        // This is ugly but whitout it i get time losses etc
        if (!hasOneLegalMove)
        {
            searcher.hardLimit = 10000;
            searcher.softLimit = 10000;
            isNormalSearch = true;
            shouldStop = false;
            pvs(-infinity, infinity, 1, 0, board, false);
            std::cout << "bestmove " << uci::moveToUci(rootBestMove) << std::endl;
            break;
        }

        if (!hasNodeLimit)
        {
            std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
            std::cout
                << "info depth "
                << i << " score cp "
                << scoreData << " nodes "
                << nodes << " nps "
                << static_cast<int>(searcher.nodes / (elapsed.count() + 1) * 1000) << " pv "
                << getPVLine()
                << std::endl;
        }

        // std::cout << "Time for this move: " << timeForMove << " | Time used: " << static_cast<int>(elapsed.count()) << " | Depth: " << i << " | bestmove: " << bestMove << std::endl;

        if ((hasOneLegalMove && (shouldStopID(start) && !isInfinite)) || i == 255)
        {
            if (!hasNodeLimit)
            {
                std::cout << "bestmove " << uci::moveToUci(bestMoveThisIteration) << std::endl;
            }
            break;
        }
    }
    shouldStop = false;
    isNormalSearch = true;
}

void Search::initLMR()
{
    double lmrBaseFinal = lmrBase / 100.0;
    double lmrDivisorFinal = lmrDivisor / 100.0;
    for (int depth = 1; depth < 150; depth++)
    {
        for (int moveCount = 1; moveCount < 218; moveCount++)
        {
            reductions[depth][moveCount] = static_cast<std::uint8_t>(std::clamp(lmrBaseFinal + std::log(depth) * std::log(moveCount) / lmrDivisorFinal, 0.0, 255.0));
        }
    }
}

int Search::scaleOutput(int rawEval, Board &board)
{
    int gamePhase = materialScaleKnight * board.pieces(PieceType::KNIGHT).count() + materialScaleBishop * board.pieces(PieceType::BISHOP).count() + materialScaleRook * board.pieces(PieceType::ROOK).count() + materialScaleQueen * board.pieces(PieceType::QUEEN).count();
    return rawEval * (materialScaleGamePhaseAdder + gamePhase) / materialScaleGamePhaseDivisor;
}

std::string Search::getPVLine()
{
    std::string pvLine;
    for (int i = 0; i < stack[0].pvLength; i++)
    {
        pvLine += uci::moveToUci(stack[0].pvLine[i]) + " ";
    }
    return pvLine;
}

int Search::getQuietHistory(Board &board, Move move)
{
    return quietHistory[board.sideToMove()][board.at(move.from()).type()][move.to().index()];
}

void Search::updateQuietHistory(Board &board, Move move, int bonus)
{
    quietHistory
        [board.sideToMove()]
        [board.at(move.from()).type()]
        [move.to().index()] +=
        (bonus - getQuietHistory(board, move) * std::abs(bonus) / quietHistoryDivisor);
}

int Search::getContinuationHistory(PieceType piece, Move move, int ply)
{
    return continuationHistory[stack[ply].previousMovedPiece][stack[ply].previousMove.to().index()][piece][move.to().index()];
}

void Search::updateContinuationHistory(PieceType piece, Move move, int bonus, int ply)
{
    // Continuation History is indexed as follows
    // | Ply - 1 Moved Piece From | Ply - 1 Move To Index | Moved Piece From | Move To Index |
    int scaledBonus = (bonus - getContinuationHistory(piece, move, ply - 1) * std::abs(bonus) / continuationHistoryDivisor);

    if (stack[ply - 1].previousMovedPiece != PieceType::NONE)
    {
        // Continuation History is indexed as follows
        // | Ply - 1 Moved Piece From | Ply - 1 Move To Index | Moved Piece From | Move To Index |
        continuationHistory[stack[ply - 1].previousMovedPiece][stack[ply - 1].previousMove.to().index()][piece][move.to().index()] += scaledBonus;
    }
}

void Search::updatePawnCorrectionHistory(int bonus, Board &board)
{
    int pawnHash = getPieceKey(PieceType::PAWN, board);
    // Gravity
    int scaledBonus = bonus - pawnCorrectionHistory[board.sideToMove()][pawnHash & (pawnCorrectionHistorySize - 1)] * std::abs(bonus) / pawnCorrectionHistoryGravityDiv;
    pawnCorrectionHistory[board.sideToMove()][pawnHash & (pawnCorrectionHistorySize - 1)] += scaledBonus;
}

int Search::correctEval(int rawEval, Board &board)
{
    int pawnEntry = pawnCorrectionHistory[board.sideToMove()][getPieceKey(PieceType::PAWN, board) & (pawnCorrectionHistorySize - 1)];

    int corrHistoryBonus = pawnEntry; // Later here come minor Corr Hist all multipled

    return rawEval + corrHistoryBonus / correctionValueDiv;
}

std::uint64_t Search::getPieceKey(PieceType piece, const Board& board)
{
    std::uint64_t key = 0;
    Bitboard bitboard = board.pieces(piece);
    while (bitboard)
    {
        const Square square = bitboard.pop();
        key ^= Zobrist::piece(board.at(square), square);
    }
    return key;
}
