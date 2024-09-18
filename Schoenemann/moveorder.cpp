#include "moveorder.h"

#include "consts.h"
#include "tt.h"
#include "see.h"

void orderMoves(Movelist moveList, Hash* entry, Board& board, int scores[])
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
			}
		}
		else if (board.isCapture(move))
		{
			PieceType captured = board.at<PieceType>(move.to());
            PieceType capturing = board.at<PieceType>(move.from());

			int captureScore = see(board, move, 0) ? goodCapture : badCapture;

			// MVA - LVV
			captureScore += 100 * PIECE_VALUES[captured] - PIECE_VALUES[capturing];

			scores[i] = captureScore;
		}
		else if (move.typeOf() == Move::PROMOTION)
		{
			scores[i] = promotion;
		}
	}
	
}

Move sortByScore(Movelist moveList, int scores[], int i)
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
