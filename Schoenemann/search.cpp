#include "search.h"

std::chrono::time_point start = std::chrono::high_resolution_clock::now();

DEFINE_PARAM_S(probeCutBetaAddition, 390, 20);
DEFINE_PARAM_S(probeCuteSubtractor, 2, 1);

DEFINE_PARAM_S(iidDepth, 3, 1);

DEFINE_PARAM_S(rfpDepth, 5, 1);
DEFINE_PARAM_S(rfpEvalSubtractor, 69, 5);

DEFINE_PARAM_S(winningDepth, 5, 1);
DEFINE_PARAM_S(winningEvalSubtractor, 102, 1);
DEFINE_PARAM_S(winningDepthMultiplyer, 19 , 2);

DEFINE_PARAM_S(probeCutMarginAdder, 66, 10);

DEFINE_PARAM_S(winningDepthDivisor, 4, 1);
DEFINE_PARAM_S(winningDepthSubtractor, 3, 1);


DEFINE_PARAM_S(nmpDepth, 3, 1);
DEFINE_PARAM_S(nmpDepthAdder, 3, 1);
DEFINE_PARAM_S(nmpDepthDivisor, 3, 1);

DEFINE_PARAM_S(razorDepth, 1, 1);
DEFINE_PARAM_S(razorAlpha, 341, 1);
DEFINE_PARAM_S(razorDepthMultiplyer, 73, 1);

int Search::pvs(int alpha, int beta, int depth, int ply, Board& board)
{
    //Increment nodes by one
    nodes++;

    if (shouldStop)
    {
        return beta;
    }

    if(nodes % 4096 == 0)
    {
        if (shouldStopSoft(start) && !isNormalSearch)
        {
            shouldStop = true;
        }
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
    Move hashedMove = Move::NULL_MOVE;

    //Get some important search constants
    const bool pvNode = beta > alpha + 1;
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
            hashedScore = transpositionTabel.scoreFromTT(entry->score, ply);
            hashedType = entry->type;
            hashedDepth = entry->depth;
            staticEval = entry->eval;
            hashedMove = entry->move;
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
        if (zobristKey != entry->key && !inCheck && depth >= iidDepth)
        {
            depth--;
        }
    }

    if (!isNullptr)
    {
        int probCutBeta = beta + probeCutBetaAddition;
        if (hashedDepth >= depth - probeCuteSubtractor && hashedScore >= probCutBeta && std::abs(beta) < infinity)
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
    if (!inCheck && depth <= rfpDepth && staticEval - rfpEvalSubtractor * depth >= beta)
    {
        return staticEval;
    }

    //Razoring
    if (!pvNode && !board.inCheck() && depth <= razorDepth)
    {
        const int ralpha = alpha - razorAlpha - depth * razorDepthMultiplyer;

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

    //Idea by Laser
    //If we can make a winning move and can confirm that when we do a lower depth search
    //it causes a beta cuttoff we can make that beta cutoff
    if (!pvNode && !inCheck && depth >= winningDepth && staticEval >= beta - winningEvalSubtractor - winningDepthMultiplyer * depth && std::abs(beta) < infinity)
    {
        int probCutMargin = beta + probeCutMarginAdder;
        int probCutCount = 0;

        Movelist moveList;
        movegen::legalmoves(moveList, board);

        int scoreMoves[218] = {0};
        //Sort the list
        orderMoves(moveList, entry, board, scoreMoves);
        for (int i = 0; i < moveList.size() && probCutCount < 3; i++)
        {
            probCutCount++;
            Move move = sortByScore(moveList, scoreMoves, i);

            //We don't want to prune the hashed move
            if (move == hashedMove)
            {
                continue;
            }

            board.makeMove(move);

            int score = -pvs(-probCutMargin, -probCutMargin + 1, depth - depth / winningDepthDivisor - winningDepthSubtractor, ply + 1, board);

            board.unmakeMove(move);
            
            if (score >= probCutMargin)
            {
                return score;
            }
        }

    }

    if (!pvNode && !inCheck)
    {
        if (depth >= nmpDepth && staticEval >= beta)
        {
            board.makeNullMove();
            int depthReduction = nmpDepthAdder + depth / nmpDepthDivisor;
            int score = -pvs(-beta, -alpha, depth - depthReduction, ply + 1, board);
            board.unmakeNullMove();
            if (score >= beta)
            {
                return score;
            }
        }
    }

    short type = UPPER_BOUND;
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

        if (i == 0)
        {
            score = -pvs(-beta, -alpha, depth - 1 + checkExtension, ply + 1, board);
        }
        else
        {
            score = -pvs(-alpha - 1, -alpha, depth - 1 + checkExtension, ply + 1, board);
            if (score > alpha && beta - alpha > 1)
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
                if (!board.isCapture(move))
                {
                    countinuationButterfly[move.from().index()][move.to().index()] = move;
                }
                
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
    transpositionTabel.storeEvaluation(zobristKey, depth, finalType, transpositionTabel.scoreToTT(bestScore, ply), bestMoveInPVS, staticEval);

    return bestScore;
}

int Search::qs(int alpha, int beta, Board& board, int ply)
{

    if (shouldStop)
    {
        return beta;
    }

    if(nodes % 4096 == 0)
    {
        //Check for a timeout
        if (shouldStopSoft(start) && !isNormalSearch) 
        {
            return beta;
        }
    }

    nodes++;
    const bool pvNode = beta > alpha + 1;
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

    transpositionTabel.storeEvaluation(zobristKey, 0, bestScore >= beta ? LOWER_BOUND : UPPER_BOUND, transpositionTabel.scoreToTT(bestScore, ply), bestMoveInQs, standPat);

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
        score = i >= 6 ? aspiration(i, score, board) : pvs(-infinity, infinity, i, 0, board);
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
