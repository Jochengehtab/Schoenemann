#include <chrono>
#include "Search.h"
#include "Evaluate.h"
#include "movegen/chess.hpp"
#include "timeman.h"

using namespace chess;

const short infinity = 32767;
int count_nodes = 0;
chess::Move bestMove = chess::Move::NULL_MOVE;

int search(int depth, int alpha, int beta, int ply, Board& board) {
    if (depth == 0)
    {
        return quiescence_search(alpha, beta, board);
    }


    Movelist movelist;
    movegen::legalmoves(movelist, board);

    if (movelist.size() == 0)
    {
        if (board.inCheck() == true) 
        {
            return infinity;
        }
        else
        {
            return 0;
        }
    }

    for (const auto& move : movelist) {
        count_nodes++;
        board.makeMove(move);
        int evaluation = -search(depth - 1, -beta, -alpha, ply + 1, board);
        board.unmakeMove(move);

        if (evaluation > alpha)
        {
            if (ply == 0)
            {
                bestMove = move;
            }
            alpha = evaluation;
        }

        if (evaluation >= beta)
        {
            return beta;
        }
    }
    return alpha;
}

int quiescence_search(int alpha, int beta, Board& board) {
	int stand_pat = evaluate(board);
	if (stand_pat >= beta)
	{
		return beta;
	}
	if (alpha < stand_pat)
	{
		alpha = stand_pat;
	}

	Movelist movelist;
	movegen::legalmoves<movegen::MoveGenType::CAPTURE>(movelist, board);
	for (const auto& move : movelist) {
        board.makeMove(move);
        int score = -quiescence_search(-beta, -alpha, board);
        board.unmakeMove(move);
        if (score >= beta)
        {
            return beta;
        }
        if (score > alpha)
        {
            alpha = score;
        }
	}
	return alpha;
}

void iterative_deepening(Board& board)
{
    auto start = std::chrono::high_resolution_clock::now();
    int time_for_move = get_time_for_move();

    while (true)
    {
        for (int i = 1; i < 256; i++)
        {
            search(i, -32767, 32767, 0, board);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            if (elapsed.count() >= time_for_move)
            {
                std::cout << "bestmove " << bestMove << std::endl;
                return;
            }
        }
    }
}

int getNodes() {
	return count_nodes;
}

chess::Move& getBestMove() {
	return bestMove;
}
