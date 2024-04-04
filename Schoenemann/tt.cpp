#include <iostream>

#include "tt.h"

void tt::storeEvaluation(int depth, int play, int eval, int nodeType, Move move, Board& board)
{
	entry ent;
	ent.key = board.zobrist();
	ent.eval = eval;
	ent.nodeType = nodeType;
	ent.depth = depth;
	ent.move = move;
	entries[board.zobrist() % ttSize] = ent;
}

int tt::lookUpEvaluation(int depth, int ply, int alpha, int beta, Board& board)
{
	entry ent = entries[board.zobrist() % ttSize];
	if (ent.key == board.zobrist())
	{
		if (ent.depth >= depth)
		{
			int score = ent.eval;
			if (ent.nodeType == exact)
			{
				return score;
			}
			if (ent.nodeType == uppperBound && score <= alpha)
			{
				return score;
			}
			if (ent.nodeType == lowerBound && score >= beta)
			{
				return score;
			}
		}
	}
	return -40000;
}

Move tt::getStoredMove(Board& board)
{
	return entries[board.zobrist() % ttSize].move;
}

void tt::clear(tt obj)
{

}