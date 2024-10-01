#include <chrono>
#include <iostream>
#include <cmath>
#include <cassert>

#include "search.h"
#include "chess.hpp"
#include "timeman.h"
#include "moveorder.h"
#include "consts.h"
#include "nnue.h"
#include "see.h"

using namespace chess;

std::chrono::time_point start = std::chrono::high_resolution_clock::now();

int Search::pvs(int alpha, int beta, int depth, int ply, Board& board)
{
    //Increment nodes by one
    nodes++;

    if (shouldStop)
    {
        return beta;
    }

    if (shouldStopSoft(start) && !isNormalSearch)
    {
        shouldStop = true;
    }

    //If depth is 0 we drop into qs to get a neutral position
    if (depth == 0)
    {
        return net.evaluate((int)board.sideToMove());
    }

    const bool inCheck = board.inCheck();

    Movelist moveList;
    movegen::legalmoves(moveList, board);

    if (moveList.size() == 0)
    {
        if (inCheck == true)
        {
            return -infinity + ply;
        }
        else
        {
            return 0;
        }
    }
    else if(board.isHalfMoveDraw() || board.isRepetition() || board.isInsufficientMaterial())
    {
        return 0;
    }

    //int scoreMoves[218] = {0};
    //Sort the list
    //orderMoves(moveList, entry, board, scoreMoves);

    int score = 0;
    int bestScore = -infinity;
    Move bestMoveInPVS = Move::NULL_MOVE;
    for (int i = 0; i < moveList.size(); i++)
    {
        Move move = moveList[i];
        board.makeMove(move);
        
        if (i == 0)
        {
            score = -pvs(-beta, -alpha, depth - 1, ply + 1, board);
        }
        else
        {
            score = -pvs(-alpha - 1, -alpha, depth - 1, ply + 1, board);
            if (score > alpha && beta - alpha > 1)
            {
                score = -pvs(-beta, -alpha, depth - 1, ply + 1, board);
            }
        }
        

        board.unmakeMove(move);

        if (score > bestScore)
        {
            bestScore = score;
            if (score > alpha)
            {
                alpha = score;
                bestMoveInPVS = move;

                //If we are ate the root we set the bestMove
                if (ply == 0)
                {
                    rootBestMove = move;
                }
            }

            //Beta cutoff
            if (score >= beta)
            {
                return beta;
            }
        }
    }
    return alpha;
}

int Search::qs(int alpha, int beta, Board& board, int ply)
{
    //Check for a timeout
    if (shouldStopSoft(start) && !isNormalSearch) 
    {
        return beta;
    }
    return 0;
}

int Search::aspiration(int depth, int score, Board& board)
{
    int delta = 25;
    int alpha = std::max(-infinity, score - delta);
    int beta = std::min(infinity, score + delta);

    while (true)
    {
        score = pvs(alpha, beta, depth, 0, board);
        if (shouldStopSoft(start)) 
        {
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

        delta *= 1.5;
    }

    return score;
}


void Search::iterativeDeepening(Board& board, bool isInfinite)
{
    start = std::chrono::high_resolution_clock::now();
    getTimeForMove();
    rootBestMove = Move::NULL_MOVE;
    Move bestMoveThisIteration = Move::NULL_MOVE;
    isNormalSearch = false;
    bool hasFoundMove = false;
    int score = 0;

    if (isInfinite)
    {
        timeForMove = 0;
        isNormalSearch = true;
    }

    nodes = 0;

    for (int i = 1; i <= 256; i++)
    {
        score = pvs(-infinity, infinity, i, 0, board);
        std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - start;
        // Add one the avoid division by zero
        int timeCount = elapsed.count() + 1;
        if (!shouldStop)
        {
            bestMoveThisIteration = rootBestMove;
        }

        if (bestMoveThisIteration == Move::NULL_MOVE)
        {
            bestMoveThisIteration = rootBestMove;
        }

        if (bestMoveThisIteration != Move::NULL_MOVE)
        {
            hasFoundMove = true;
        }

        std::cout << "info depth " << i << " score cp " << score << " nodes " << nodes << " nps " << static_cast<int>(seracher.nodes / timeCount * 1000) << " pv " << uci::moveToUci(rootBestMove) << std::endl;


        //std::cout << "Time for this move: " << timeForMove << " | Time used: " << static_cast<int>(elapsed.count()) << " | Depth: " << i << " | bestmove: " << bestMove << std::endl;
        if (i == 256 && hasFoundMove)
        {
            std::cout << "bestmove " << uci::moveToUci(rootBestMove) << std::endl;
            break;
        }

        if (shouldStopID(start) && hasFoundMove && !isInfinite)
        {
            std::cout << "bestmove " << uci::moveToUci(bestMoveThisIteration) << std::endl;
            shouldStop = true;
            break;
        }

        if (shouldStop && hasFoundMove)
        {
            std::cout << "bestmove " << uci::moveToUci(bestMoveThisIteration) << std::endl;
            shouldStop = true;
            break;
        }

    }
    shouldStop = false;
    isNormalSearch = true;
}
