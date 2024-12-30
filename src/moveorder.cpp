#include "moveorder.h"

DEFINE_PARAM_S(mvaLvvMultiplyer, 103, 20);

void orderMoves(Movelist &moveList, Hash *entry, Board &board, int scores[], Move killer, int ply)
{
	const bool isNullptr = entry == nullptr ? true : false;
	const std::uint64_t key = board.zobrist();

	for (int i = 0; i < moveList.size(); i++)
	{
		Move move = moveList[i];
		if (!isNullptr)
		{
			if (entry->key == key && move == entry->move)
			{
				scores[i] = hashMoveScore;
				continue;
			}
		}
		if (board.isCapture(move))
		{
			PieceType captured = board.at<PieceType>(move.to());
			PieceType capturing = board.at<PieceType>(move.from());

			int captureScore = see(board, move, 0) ? goodCapture : badCapture;

			// MVA - LVV
			captureScore += mvaLvvMultiplyer * PIECE_VALUES[captured] - PIECE_VALUES[capturing];

			scores[i] = captureScore;
		}
		else if (move == killer)
		{
			scores[i] = killerScore;
		}
		else if (move.typeOf() == Move::PROMOTION)
		{
			scores[i] = promotion;
		}
		else
		{
			scores[i] += searcher.quietHistory[board.sideToMove()][board.at(move.from()).type()][move.to().index()] + (searcher.continuationHistory[searcher.stack[ply - 1].previousMovedPiece][searcher.stack[ply - 1].previousMove.to().index()][board.at(move.from()).type()][move.to().index()]);
		}
	}
}

Move sortByScore(Movelist &moveList, int scores[], int i)
{
	for (int j = i + 1; j < moveList.size(); j++)
	{
		if (scores[j] > scores[i])
		{
			std::swap(moveList[i], moveList[j]);
			std::swap(scores[i], scores[j]);
		}
	}
	return moveList[i];
}
