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

    std::chrono::time_point end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    bool isOver = elapsed.count() >= timeForMove;

    if (isOver && !isNormalSearch)
    {
        shouldStop = true;
    }

    // Mate distance Prunning

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

    //If depth is 0 we drop into qs to get a neutral position
    if (depth == 0)
    {
        return qs(alpha, beta, board, ply);
    }


    const std::uint64_t zobristKey = board.zobrist();
    int hashedScore = 0;
    short hashedType = 0;
    int hashedDepth = 0;
    int staticEval = NO_VALUE;

    //Get some important search constants
    const bool pvNode = (alpha != beta) - 1;
    const bool inCheck = board.inCheck();

    //Get an potential hash entry
    Hash* entry = transpositionTabel.getHash(zobristKey);

    //Check if we this stored position is valid
    const bool isNullptr = entry == nullptr ? true : false;

    if (!isNullptr)
    {
        //If we have a transposition
        //That means that the current board zobrist key 
        //is the same as the hash entry zobrist key 
        if (zobristKey == entry->key)
        {
            hashedScore = transpositionTabel.ScoreFromTT(entry->score, ply);
            hashedType = entry->type;
            hashedDepth = entry->depth;
            staticEval = entry->eval;
        }

        //Check if we can return a stored score
        if (!pvNode && hashedDepth >= depth && transpositionTabel.checkForMoreInformation(hashedType, hashedScore, beta))
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
    if (!isNullptr)
    {
        if (zobristKey != entry->key && !inCheck && depth >= 4)
        {
            depth--;
        }
    }

    if (!isNullptr)
    {
        int probCutBeta = beta + 390;
        if (hashedDepth >= depth - 2 && hashedScore >= probCutBeta && std::abs(beta) < infinity)
        {
            return probCutBeta;
        }
    }

    //If no evaluation was found in the transposition table
    //we perform an static evaulation
    if (staticEval == NO_VALUE)
    {
        staticEval = net.evaluate((int)board.sideToMove());
    }

    //Reverse futility pruning
    if (!pvNode && !inCheck && depth <= 6 && staticEval - 70 * depth >= beta)
    {
        return staticEval;
    }

    if (!pvNode && !inCheck)
    {
        if (depth >= 5 && staticEval >= beta)
        {
            board.makeNullMove();
            int depthReduction = 3 + depth / 3;
            int score = -pvs(-beta, -alpha, depth - depthReduction, ply + 1, board);
            board.unmakeNullMove();
            if (score >= beta)
            {
                return score;
            }
        }
    }

    short type = UPPER_BOUND;

    bool bSearchPv = true;

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

    int scoreMoves[218] = {0};
    //Sort the list
    orderMoves(moveList, entry, board, scoreMoves);

    int score = 0;
    int bestScore = -infinity;
    Move bestMoveInPVS = Move::NULL_MOVE;
    for (int i = 0; i < moveList.size(); i++)
    {
        Move move = sortByScore(moveList, scoreMoves, i);
        board.makeMove(move);

        short checkExtension = 0;

        if (board.inCheck() == true)
        {
            checkExtension = 1;
        }

        if (bSearchPv)
        {
            score = -pvs(-beta, -alpha, depth - 1 + checkExtension, ply + 1, board);
        }
        else
        {
            score = -pvs(-alpha - 1, -alpha, depth - 1 + checkExtension, ply + 1, board);
            if (score > alpha && score < beta)
            {
                score = -pvs(-beta, -alpha, depth - 1 + checkExtension, ply + 1, board);
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
                break;
            }
        }
    }

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
    transpositionTabel.storeEvaluation(zobristKey, depth, finalType, transpositionTabel.ScoreToTT(bestScore, ply), bestMoveInPVS, staticEval);

    return bestScore;
}

int Search::qs(int alpha, int beta, Board& board, int ply)
{
    nodes++;
    const bool pvNode = (alpha != beta) - 1;
    const std::uint64_t zobristKey = board.zobrist();

    Hash* entry = transpositionTabel.getHash(zobristKey);
    const bool isNullptr = entry == nullptr ? true : false;
    const bool inCheck = board.inCheck();

    int hashedScore = 0;
    short hashedType = 0;
    int standPat = NO_VALUE;

    if (!isNullptr)
    {
        if (zobristKey == entry->key)
        {
            hashedScore = transpositionTabel.ScoreFromTT(entry->score, ply);
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
        standPat = net.evaluate((int)board.sideToMove());
    }

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

    for (Move& move : moveList)
    {
        // Fultiy Prunning
        if (!see(board, move, 1) && standPat + SEE_PIECE_VALUES[board.at(move.to()).type()] <= alpha)
        {
            continue;
        }
        
        // Static Exchange Evaluation
        if (!see(board, move, 0))
        {
            continue;
        }
        
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
    if (inCheck && bestScore == -infinity)
    {
        return -infinity + ply;
    }

    transpositionTabel.storeEvaluation(zobristKey, 0, bestScore >= beta ? LOWER_BOUND : UPPER_BOUND, transpositionTabel.ScoreToTT(bestScore, ply), bestMoveInQs, standPat);

    return bestScore;
}

int Search::aspiration(int depth, int score, Board& board)
{
    int delta = 25;
    int alpha = std::max(-infinity, score - delta);
    int beta = std::min(infinity, score + delta);

    while (true)
    {
        score = pvs(alpha, beta, depth, 0, board);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        if (elapsed.count() >= timeForMove) 
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
    timeForMove = getTimeForMove();
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
        score = i >= 6 ? aspiration(i, score, board) : pvs(-infinity, infinity, i, 0, board);
        std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - start;
        // Add one the avoid division by zero
        int timeCount = elapsed.count() + 1;
        bool isOver = timeCount >= timeForMove;
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

        if (isOver && hasFoundMove && !isInfinite)
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
