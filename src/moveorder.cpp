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

#include "moveorder.h"
#include "see.h"
#include "tune.h"

DEFINE_PARAM_S(mvaLvvMultiplyer, 103, 20);

void MoveOrder::orderMoves(Movelist &moveList, Hash *entry, Board &board, int scores[], int ply)
{
	const bool isNullptr = entry == nullptr ? true : false;
	const std::uint64_t key = board.zobrist();
	const Move killer = searcher.stack[ply].killerMove;

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
		else if (move == killer && killer != Move::NULL_MOVE)
		{
			scores[i] = killerScore;
		}
		else if (move.typeOf() == Move::PROMOTION)
		{
			scores[i] = promotion;
		}
		else
		{
			scores[i] += history.getQuietHistory(board, move);
			if (ply > 0 && searcher.stack[ply - 1].previousMovedPiece != PieceType::NONE)
			{
				scores[i] += history.getContinuationHistory(board.at(move.from()).type(), move, ply - 1, searcher.stack);
			}
		}
	}
}

Move MoveOrder::sortByScore(Movelist &moveList, int scores[], int i)
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
