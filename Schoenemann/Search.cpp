#include <chrono>
#include <iostream>

#include "Search.h"
#include "evaluate.h"
#include "movegen/chess.hpp"
#include "timeman.h"
#include "Moveorder.h"
#include "consts.h"

using namespace chess;

std::chrono::time_point start = std::chrono::high_resolution_clock::now();

int searcher::pvs(int alpha, int beta, int depth, int ply, Board& board)
{
    if (shouldStop)
    {
        return alpha;
    }

    std::chrono::time_point end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    bool isOver = elapsed.count() >= timeForMove;
    if (isOver && !isNormalSearch)
    {
        shouldStop = true;
    }
    nodes++;

    if (depth == 0)
    {
        return qs(alpha, beta, board, ply);
    }

    int hashedScore = 0;
    int hashedEval = 0;
    short hashedType = 0;
    int hashedDepth = 0;

    const bool pvNode = alpha != beta - 1;
    const bool root = ply == 0;
    bool isNullptr;

    Hash* entry = transpositionTabel.getHash(board);


    if (entry == nullptr)
    {
        isNullptr = true;
    }
    else
    {
        isNullptr = false;
    }

    if (!isNullptr)
    {
        if (board.hash() == entry->key)
        {
            hashedScore = transpositionTabel.ScoreFromTT(entry->score, ply);
            hashedType = entry->type;
            hashedDepth = entry->depth;
            hashedEval = entry->eval;
        }
    }

    if (!isNullptr)
    {
        if (!pvNode && hashedDepth >= depth && transpositionTabel.checkForMoreInformation(hashedType, hashedScore, beta))
        {
            if (hashedType == EXACT)
            {
                transpositions++;
                return hashedScore;
            }
            if (hashedType == UPPER_BOUND && hashedScore <= alpha)
            {
                transpositions++;
                return hashedScore;
            }
            if (hashedType == LOWER_BOUND && hashedScore >= beta)
            {
                transpositions++;
                return hashedScore;
            }
        }
    }

    short type = UPPER_BOUND;

    bool bSearchPv = true;

    Movelist moveList;
    movegen::legalmoves(moveList, board);


    if (moveList.size() == 0)
    {
        if (board.inCheck() == true)
        {
            return -MATE + ply;
        }
        else
        {
            return 0;
        }
    }
    int score = 0;
    int bestScore = -infinity;
    int externsions = 0;
    for (const Move& move : moveList)
    {
        if (!nullptr)
        {
            if (depth >= 5 && !board.inCheck() && ply > 0 && hashedDepth >= depth - 3 && (hashedType == LOWER_BOUND || hashedType == UPPER_BOUND))
            {
                int betaCut = std::min(static_cast<int>(hashedScore - depth * 2), static_cast<int>(beta));
                score = pvs(betaCut - 1, betaCut, depth + 1, ply, board);

                if (score < betaCut)
                {
                    externsions++;
                }
                else if (score >= beta)
                {
                    return score;
                }
                else if (hashedScore >= beta)
                {
                    score = pvs(beta - 1, beta, depth + 4, ply, board);
                    if (score >= beta)
                    {
                        return score;
                    }
                }
            }
        }
        board.makeMove(move);

        if (bSearchPv)
        {
            score = -pvs(-beta, -alpha, depth - 1 + externsions, ply + 1, board);
        }
        else
        {
            score = -pvs(-alpha - 1, -alpha, depth - 1 + externsions, ply + 1, board);
            if (score > alpha && score < beta)
            {
                score = -pvs(-beta, -alpha, depth - 1 + externsions, ply + 1, board);
            }
        }

        board.unmakeMove(move);

        if (score > bestScore)
        {
            bestScore = score;
            if (score > alpha)
            {
                alpha = score;

                bSearchPv = false;
                type = EXACT;

                //If we are at the root we set the bestMove
                if (ply == 0)
                {
                    bestMove = move;
                }
            }

            //Beta cutoff
            if (score >= beta)
            {
                break;
            }
        }
    }

    if (!root)
    {
        short finalType;
        //Calculate the node type
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
        transpositionTabel.storeEvaluation(board.hash(), depth, finalType, transpositionTabel.ScoreToTT(bestScore, ply), bestMove, evaluate(board));
    }

    return bestScore;
}

int searcher::qs(int alpha, int beta, Board& board, int ply)
{
    Hash* entry = transpositionTabel.getHash(board);

    int hashedScore = 0;
    short hashedType = 0;
    const bool pvNode = alpha != beta - 1;

    bool isNullptr;

    nodes++;

    if (entry == nullptr)
    {
        isNullptr = true;
    }
    else
    {
        isNullptr = false;
    }

    if (!isNullptr)
    {
        if (board.hash() == entry->key)
        {
            hashedScore = transpositionTabel.ScoreFromTT(entry->score, ply);
            hashedType = entry->type;
        }
    }

    if (!isNullptr)
    {
        if (!pvNode && transpositionTabel.checkForMoreInformation(hashedType, hashedScore, beta))
        {
            if (hashedType == EXACT)
            {
                transpositions++;
                return hashedScore;
            }
            if (hashedType == UPPER_BOUND && hashedScore <= alpha)
            {
                transpositions++;
                return hashedScore;
            }
            if (hashedType == LOWER_BOUND && hashedScore >= beta)
            {
                transpositions++;
                return hashedScore;
            }
        }
    }

    int standPat = evaluate(board);

    if (!board.inCheck() && transpositionTabel.checkForMoreInformation(hashedType, hashedScore, standPat))
    {
        standPat = hashedScore;
    }

    if (standPat >= beta)
    {
        return beta;
    }

    if (alpha < standPat)
    {
        alpha = standPat;
    }

    Movelist moveList;
    movegen::legalmoves<movegen::MoveGenType::CAPTURE>(moveList, board);

    int bestScore = standPat;
    Move bestMoveInQs = Move::NULL_MOVE;

    for (const Move& move : moveList)
    {
        board.makeMove(move);

        int score = -qs(-beta, -alpha, board, ply);

        board.unmakeMove(move);

        //Our current Score is better then the previos bestScore so we update it 
        if (score > bestScore)
        {
            bestScore = score;

            //Score is greater than alpha so we update alpha to the score
            if (score > alpha)
            {
                alpha = score;

                bestMoveInQs = move;
            }

            //Beta cutoff
            if (score >= beta)
            {
                break;
            }
        }
    }

    //Checks for checkmate
    if (board.inCheck() && bestScore == -infinity)
    {
        return -MATE + ply;
    }

    transpositionTabel.storeEvaluation(board.hash(), 0, bestScore >= beta ? LOWER_BOUND : UPPER_BOUND, transpositionTabel.ScoreToTT(bestScore, ply), bestMoveInQs, standPat);

    return bestScore;
}

void searcher::iterativeDeepening(Board& board)
{
    start = std::chrono::high_resolution_clock::now();
    timeForMove = getTimeForMove();
    bestMove = Move::NULL_MOVE;
    Move bestMoveThisIteration = Move::NULL_MOVE;
    isNormalSearch = false;
    bool hasFoundMove = false;

    if (timeForMove == -20)
    {
        pvs(-32767, 32767, 1, 0, board);
        if (bestMove != Move::NULL_MOVE)
        {
            std::cout << "bestmove " << bestMove << std::endl;
            return;
        }
    }

    for (int i = 1; i <= 256; i++)
    {
        pvs(-32767, 32767, i, 0, board);

        if (!shouldStop)
        {
            bestMoveThisIteration = bestMove;
        }

        if (bestMoveThisIteration == Move::NULL_MOVE)
        {
            bestMoveThisIteration = bestMove;
        }

        if (bestMove != Move::NULL_MOVE)
        {
            hasFoundMove = true;
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        bool isOver = elapsed.count() >= timeForMove;

        //std::cout << "Time for this move: " << timeForMove << " | Time used: " << static_cast<int>(elapsed.count()) << " | Depth: " << i << " | bestmove: " << bestMove << std::endl;
        if (i == 256 && hasFoundMove)
        {
            std::cout << "bestmove " << bestMove << std::endl;
            break;
        }

        if (isOver && hasFoundMove)
        {
            std::cout << "bestmove " << bestMoveThisIteration << std::endl;
            shouldStop = true;
            break;
        }

    }
    shouldStop = false;
    isNormalSearch = true;
}