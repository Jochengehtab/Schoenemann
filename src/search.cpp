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

#include "search.h"
#include "see.h"
#include "tune.h"

std::chrono::time_point start = std::chrono::steady_clock::now();

DEFINE_PARAM_S(probeCutBetaAdd, 460, 25);
DEFINE_PARAM_S(probeCuteSub, 4, 1);

DEFINE_PARAM_S(iidDepth, 3, 1);

DEFINE_PARAM_S(rfpDepth, 5, 1);
DEFINE_PARAM_S(rfpEvalSub, 80, 6);
DEFINE_PARAM_B(rfpDivisory, 2, 1, 10);

DEFINE_PARAM_S(winningDepth, 6, 1);
DEFINE_PARAM_S(winningEvalSub, 97, 20);
DEFINE_PARAM_S(winningDepthMul, 24, 4);

DEFINE_PARAM_S(probeCutMarginAdd, 76, 10);

DEFINE_PARAM_B(winningDepthDiv, 3, 1, 20);
DEFINE_PARAM_S(winningDepthSub, 4, 1);
DEFINE_PARAM_B(winningCount, 2, 1, 6);

// Null Move Prunning
DEFINE_PARAM_B(nmpDepth, 3, 1, 9);
DEFINE_PARAM_S(nmpDepthAdd, 2, 1);
DEFINE_PARAM_B(nmpDepthDiv, 3, 1, 10);
DEFINE_PARAM_S(nmpTweak, 1, 12);

// Razoring
DEFINE_PARAM_B(razorDepth, 1, 1, 10);
DEFINE_PARAM_S(razorAlpha, 247, 30);
DEFINE_PARAM_S(razorDepthMul, 50, 9);

// PVS - SEE
DEFINE_PARAM_B(pvsSSEDepth, 2, 1, 6);
DEFINE_PARAM_S(pvsSSECaptureCutoff, 92, 10);
DEFINE_PARAM_S(pvsSSENonCaptureCutoff, 18, 10);

// Aspiration Window
DEFINE_PARAM_S(aspDelta, 26, 6);
// DEFINE_PARAM_B(aspDivisor, 2, 2, 8); When tuned this triggers crashes for some reason :(
DEFINE_PARAM_B(aspMul, 134, 1, 450);
DEFINE_PARAM_B(aspDepth, 7, 6, 12);

// Late Move Reductions
DEFINE_PARAM_B(lmrBase, 78, 1, 300);
DEFINE_PARAM_B(lmrDivisor, 240, 1, 700);
DEFINE_PARAM_B(lmrDepth, 2, 1, 7);
DEFINE_PARAM_S(lmrCutNodeMul, 2, 15);

DEFINE_PARAM_S(iirReduction, 2, 1);
DEFINE_PARAM_S(fpCutoff, 2, 1);

// Quiet History
DEFINE_PARAM_S(quietHistoryGravityBase, 31, 5);
DEFINE_PARAM_S(quietHistoryDepthMul, 204, 25);
DEFINE_PARAM_S(quietHistoryBonusCap, 1734, 200);
DEFINE_PARAM_S(quietHistoryMalusBase, 15, 6);
DEFINE_PARAM_S(quietHistoryMalusMax, 1900, 150);
DEFINE_PARAM_S(quietHistoryMalusDepthMul, 171, 25);

// Continuation Hisotry
DEFINE_PARAM_S(continuationHistoryMalusBase, 25, 6);
DEFINE_PARAM_S(continuationHistoryMalusMax, 2172, 150);
DEFINE_PARAM_S(continuationHistoryMalusDepthMul, 185, 25);
DEFINE_PARAM_S(continuationHistoryGravityBase, 26, 5);
DEFINE_PARAM_S(continuationHistoryDepthMul, 208, 25);
DEFINE_PARAM_S(continuationHistoryBonusCap, 1959, 200);

// Material Scaling
DEFINE_PARAM_S(materialScaleKnight, 3, 1);
DEFINE_PARAM_S(materialScaleBishop, 3, 1);
DEFINE_PARAM_S(materialScaleRook, 5, 1);
DEFINE_PARAM_S(materialScaleQueen, 18, 3);
DEFINE_PARAM_S(materialScaleGamePhaseAdd, 169, 25);
DEFINE_PARAM_B(materialScaleGamePhaseDiv, 269, 1, 700);

// Pawn CorrectionHistory
DEFINE_PARAM_S(pawnCorrectionHistoryDepthAdd, 180, 20);
DEFINE_PARAM_B(pawnCorrectionHistoryDepthDiv, 768, 1, 4000);

// Singular Extension
DEFINE_PARAM_B(singularMinDepth, 6, 1, 15);
DEFINE_PARAM_B(singularHashDepthReuction, 3, 1, 8);
DEFINE_PARAM_B(singularBetaDepthMul, 2, 1, 6);
DEFINE_PARAM_B(singularBetaDoubleExtensionMargin, 5, 1, 50);
DEFINE_PARAM_S(singularDepthSub, 1, 15);
DEFINE_PARAM_B(singularDepthDiv, 2, 1, 20);
DEFINE_PARAM_S(singularTTSub, 2, 10);

int Search::pvs(std::int16_t alpha, std::int16_t beta, std::int16_t depth, std::int16_t ply, Board &board, bool isCutNode)
{
    if (shouldStop)
    {
        return beta;
    }

    // Every 128 we check for a timeout
    if (nodes % 128 == 0)
    {
        if (timeManagement.shouldStopSoft(start) && !isNormalSearch)
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
    if (depth <= 0)
    {
        return qs(alpha, beta, board, ply);
    }

    // Make at least zero to avoid wrong depths in the transposition table
    depth = std::max(static_cast<std::int16_t>(0), depth);

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
        int probCutBeta = beta + probeCutBetaAdd;
        if (hashedDepth >= depth - probeCuteSub && hashedScore >= probCutBeta && std::abs(beta) < infinity)
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
    staticEval = std::clamp(history.correctEval(staticEval, board), -infinity + MAX_PLY, infinity - MAX_PLY);

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
    if (!isSingularSearch && !inCheck && depth <= rfpDepth && staticEval - rfpEvalSub * (depth - improving) >= beta)
    {
        return (staticEval + beta) / rfpDivisory;
    }

    // Razoring
    if (!isSingularSearch && !pvNode && !board.inCheck() && depth <= razorDepth)
    {
        const int ralpha = alpha - razorAlpha - depth * razorDepthMul;

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
    if (!isSingularSearch && !pvNode && !inCheck && depth >= winningDepth && staticEval >= beta - winningEvalSub - winningDepthMul * depth && std::abs(beta) < infinity)
    {
        int probCutMargin = beta + probeCutMarginAdd;
        int probCutCount = 0;

        Movelist moveList;
        movegen::legalmoves(moveList, board);

        int scoreMoves[218] = {0};
        // Sort the list
        moveOrder.orderMoves(&history, moveList, entry, stack[ply].killerMove, stack, board, scoreMoves, ply);

        for (int i = 0; i < moveList.size() && probCutCount < winningCount; i++)
        {
            probCutCount++;
            Move move = moveOrder.sortByScore(moveList, scoreMoves, i);

            // We don't want to prune the hashed move
            if (move == hashedMove)
            {
                continue;
            }

            // Update the the piece and the move for continuationHistory
            stack[ply].previousMovedPiece = board.at(move.from()).type();
            stack[ply].previousMove = move;

            board.makeMove(move);

            int score = -pvs(-probCutMargin, -probCutMargin + 1, depth - depth / winningDepthDiv - winningDepthSub, ply + 1, board, false);

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
        int depthReduction = nmpDepthAdd + depth / nmpDepthDiv;

        // Small tweak
        depthReduction += nmpTweak;

        // Update the the piece and the move for continuationHistory
        stack[ply].previousMovedPiece = PieceType::NONE;
        stack[ply].previousMove = Move::NULL_MOVE;

        int score = -pvs(-beta, -alpha, depth - depthReduction, ply + 1, board, !isCutNode);
        board.unmakeNullMove();
        if (score >= beta)
        {
            if (depth < 12)
            {
                return score;
            }
            score = pvs(beta - 1, beta, depth - depthReduction, ply, board, false);

            if (score >= beta)
            {
                return score;
            }
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
    moveOrder.orderMoves(&history, moveList, entry, stack[ply].killerMove, stack, board, scoreMoves, ply);

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
        Move move = moveOrder.sortByScore(moveList, scoreMoves, i);

        if (move == stack[ply].exludedMove)
        {
            continue;
        }

        bool isQuiet = !board.isCapture(move);

        if (!pvNode && move != hashedMove && bestScore > -infinity && depth <= pvsSSEDepth && !see(board, move, (!isQuiet ? -pvsSSECaptureCutoff : -pvsSSENonCaptureCutoff)))
        {
            continue;
        }

        // Idea
        if (!pvNode && bestScore > -infinity && moveCounter >= 3 * depth && staticEval < alpha - 500)
        {
            continue;
        }

        // Late move prunning
        if (!pvNode && isQuiet && bestScore > -infinity && moveCounter > (6 + 2 * depth * depth) && depth <= 3)
        {
            break;
        }

        int extensions = 0;

        if (!isSingularSearch && hashedMove == move && depth >= singularMinDepth && hashedDepth >= depth - singularHashDepthReuction && (hashedType != UPPER_BOUND) && std::abs(hashedScore) < infinity && !(ply == 0))
        {
            const int singularBeta = hashedScore - depth * singularBetaDepthMul;
            const std::uint8_t singularDepth = (depth - singularDepthSub) / singularDepthDiv;

            stack[ply].exludedMove = move;
            int singularScore = pvs(singularBeta - 1, singularBeta, singularDepth, ply, board, isCutNode);
            stack[ply].exludedMove = Move::NULL_MOVE;

            if (singularScore < singularBeta)
            {
                extensions++;
                // If we aren't in a pvNode and our score plus some margin is still less then our singular beta when can extend furthur
                if (!pvNode && singularScore + singularBetaDoubleExtensionMargin < singularBeta)
                {
                    extensions++;
                }
            }

            // Multicut
            else if (singularBeta >= beta)
            {
                return singularBeta;
            }

            else if (hashedScore >= beta)
            {
                extensions -= singularTTSub;
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
            std::uint8_t lmr = 0;
            if (depth > lmrDepth)
            {
                lmr = reductions[depth][moveCounter];
                lmr -= pvNode;
                lmr += isCutNode * lmrCutNodeMul;
                lmr = std::clamp(lmr, static_cast<std::uint8_t>(0), static_cast<std::uint8_t>(depth - 1));
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
                    if (stack[ply].pvLength < 245)
                    {
                        stack[ply].pvLine[0] = move;
                        stack[ply].pvLength = stack[ply + 1].pvLength + 1;
                        for (std::uint16_t x = 0; x < stack[ply + 1].pvLength; x++)
                        {
                            stack[ply].pvLine[x + 1] = stack[ply + 1].pvLine[x];
                        }
                    }
                }
            }

            // Beta cutoff
            if (score >= beta)
            {
                if (isQuiet)
                {
                    stack[ply].killerMove = move;
                    int quietHistoryBonus = std::min(
                        static_cast<int>(quietHistoryGravityBase) +
                            static_cast<int>(quietHistoryDepthMul) * depth,
                        static_cast<int>(quietHistoryBonusCap));

                    history.updateQuietHistory(board, move, quietHistoryBonus);

                    int continuationHistoryBonus = std::min(
                        static_cast<int>(continuationHistoryGravityBase) +
                            static_cast<int>(continuationHistoryDepthMul) * depth,
                        static_cast<int>(continuationHistoryBonusCap));

                    // Update the continuation History
                    history.updateContinuationHistory(board.at(move.from()).type(), move, continuationHistoryBonus, ply, stack);

                    int quietHistoryMalus = std::min(
                        static_cast<int>(quietHistoryMalusBase) +
                            static_cast<int>(quietHistoryMalusDepthMul) * depth,
                        static_cast<int>(quietHistoryMalusMax));

                    int continuationHistoryMalus = std::min(
                        static_cast<int>(continuationHistoryMalusBase) +
                            static_cast<int>(continuationHistoryMalusDepthMul) * depth,
                        static_cast<int>(continuationHistoryMalusMax));

                    // History malus
                    for (int x = 0; x < movesMadeCounter; x++)
                    {
                        Move madeMove = movesMade[x];
                        if (madeMove == bestMoveInPVS)
                        {
                            continue;
                        }

                        history.updateQuietHistory(board, madeMove, -quietHistoryMalus);
                        history.updateContinuationHistory(board.at(madeMove.from()).type(), madeMove, -continuationHistoryMalus, ply, stack);
                    }
                }

                break;
            }
        }
    }

    std::uint8_t finalType;
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
        int bonus = std::clamp((int)(bestScore - staticEval) * depth * pawnCorrectionHistoryDepthAdd / pawnCorrectionHistoryDepthDiv, -CORRHIST_LIMIT / 4, CORRHIST_LIMIT / 4);
        history.updatePawnCorrectionHistory(bonus, board, pawnCorrectionHistoryDepthDiv);
    }

    return bestScore;
}

int Search::qs(std::int16_t alpha, std::int16_t beta, Board &board, std::int16_t ply)
{
    if (shouldStop)
    {
        return beta;
    }

    if (nodes % 128 == 0)
    {
        // Check for a timeout
        if (timeManagement.shouldStopSoft(start) && !isNormalSearch)
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
    std::uint8_t hashedType = 0;

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
    standPat = std::clamp(history.correctEval(standPat, board), -infinity + MAX_PLY, infinity - MAX_PLY);

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
                if (stack[ply].pvLength < 245)
                {
                    stack[ply].pvLine[0] = move;
                    stack[ply].pvLength = stack[ply + 1].pvLength + 1;
                    for (std::uint16_t i = 0; i < stack[ply + 1].pvLength; i++)
                    {
                        stack[ply].pvLine[i + 1] = stack[ply + 1].pvLine[i];
                    }
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

int Search::aspiration(std::int16_t depth, std::int16_t score, Board &board)
{
    std::int16_t delta = aspDelta;
    std::int16_t alpha = std::max(static_cast<int>(-infinity), score - delta);
    std::int16_t beta = std::min(static_cast<int>(infinity), score + delta);
    double finalASPMultiplier = aspMul / 100.0;

    while (true)
    {
        score = pvs(alpha, beta, depth, 0, board, false);
        if (timeManagement.shouldStopID(start))
        {
            shouldStop = true;
            return score;
        }

        if (score >= beta)
        {
            beta = std::min(beta + delta, static_cast<int>(infinity));
        }
        else if (score <= alpha)
        {
            beta = (alpha + beta) / 2;
            alpha = std::max(alpha - delta, static_cast<int>(-infinity));
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
    timeManagement.calculateTimeForMove();
    rootBestMove = Move::NULL_MOVE;
    Move bestMoveThisIteration = Move::NULL_MOVE;

    isNormalSearch = false;

    if (isInfinite)
    {
        isNormalSearch = true;
    }

    nodes = 0;

    for (std::uint8_t i = 1; i < 255; i++)
    {
        if (i > 7)
        {
            previousBestScore = scoreData;
        }

        scoreData = i >= aspDepth ? aspiration(i, scoreData, board) : pvs(-infinity, infinity, i, 0, board, false);

        if (i > 6)
        {
            // Update the previous best move
            previousBestMove = bestMoveThisIteration;
        }

        // Get the new best move
        bestMoveThisIteration = rootBestMove;

        if (i > 6)
        {
            timeManagement.updateBestMoveStability(bestMoveThisIteration, previousBestMove);
        }

        if (i > 7)
        {
            timeManagement.updateEvalStability(scoreData, previousBestScore);
        }

        // Only report statistic if we are not in a fixed node search beacuse of datagen
        if (!hasNodeLimit)
        {
            std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
            std::cout
                << "info depth "
                << static_cast<int>(i) << " score cp "
                << scoreData << " nodes "
                << nodes << " nps "
                << static_cast<std::uint64_t>(nodes / (elapsed.count() + 1) * 1000) << " pv "
                << getPVLine()
                << std::endl;
        }

        // std::cout << "Time for this move: " << timeForMove << " | Time used: " << static_cast<int>(elapsed.count()) << " | Depth: " << i << " | bestmove: " << bestMove << std::endl;

        if ((timeManagement.shouldStopID(start) && !isInfinite) || i == 254)
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
    for (int depth = 1; depth < MAX_PLY; depth++)
    {
        for (int moveCount = 1; moveCount < 218; moveCount++)
        {
            reductions[depth][moveCount] = static_cast<std::uint8_t>(std::clamp(lmrBaseFinal + std::log(depth) * std::log(moveCount) / lmrDivisorFinal, 0.0, 255.0));
        }
    }
}

int Search::scaleOutput(int rawEval, Board &board)
{
    int gamePhase = materialScaleKnight * board.pieces(PieceType::KNIGHT).count() +
                    materialScaleBishop * board.pieces(PieceType::BISHOP).count() +
                    materialScaleRook * board.pieces(PieceType::ROOK).count() +
                    materialScaleQueen * board.pieces(PieceType::QUEEN).count();

    return rawEval * (materialScaleGamePhaseAdd + gamePhase) / materialScaleGamePhaseDiv;
}

std::string Search::getPVLine()
{
    std::string pvLine;
    for (std::uint16_t i = 0; i < stack[0].pvLength; i++)
    {
        pvLine += uci::moveToUci(stack[0].pvLine[i]) + " ";
    }
    return pvLine;
}

void Search::resetHistory()
{
    history.resetHistorys();
}
