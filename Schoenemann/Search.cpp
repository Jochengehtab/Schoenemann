#include "Search.h"
#include "Evaluate.h"
#include "movegen/chess.hpp"

using namespace chess;

const short infinity = 32767;
int count_nodes = 0;
chess::Move bestMove = chess::Move::NULL_MOVE;

int search(int depth, int alpha, int beta, Board& board) {
	int best_score = -infinity;
	if (depth == 0)
	{
		return evaluate(board);
	}
	
	Movelist movelist;
	movegen::legalmoves(movelist, board);

	if (movelist.size() == 0)
	{
		if (board.inCheck())
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
		int evaluation = -search(depth - 1, -beta, -alpha, board);
		board.unmakeMove(move);

		if (evaluation >= beta)
		{
			return evaluation;
		}

		if (evaluation > best_score)
		{
			bestMove = move;
			alpha = best_score;
		}
	}
	return alpha;
}

int getNodes() {
	return count_nodes;
}

chess::Move& getBestMove() {
	return bestMove;
}
