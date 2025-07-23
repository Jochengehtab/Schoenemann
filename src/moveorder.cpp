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

DEFINE_PARAM(mvaLvvMultiplyer, 103, 83, 123);

void MoveOrder::orderMoves(const History *history, Movelist &moveList, const Hash *entry, const Move &killer,
                           const SearchStack *stack, const Board &board, int *scores, const int &ply) {
    const bool isNullptr = entry == nullptr;
    const std::uint64_t key = board.zobrist();

    for (int i = 0; i < moveList.size(); i++) {
        Move move = moveList[i];
        if (!isNullptr) {
            if (entry->key == key && move == entry->move) {
                scores[i] = hashMoveScore;
                continue;
            }
        }
        if (board.isCapture(move)) {
            const PieceType captured = board.at<PieceType>(move.to());
            const PieceType capturing = board.at<PieceType>(move.from());

            int captureScore = SEE::see(board, move, 0) ? goodCapture : badCapture;

            // MVA - LVV
            captureScore += mvaLvvMultiplyer * (*PIECE_VALUES[captured]) - (*PIECE_VALUES[capturing]);

            scores[i] = captureScore;
        } else if (move == killer && killer != Move::NULL_MOVE) {
            scores[i] = killerScore;
        } else {
            scores[i] += history->getQuietHistory(board, move);
            scores[i] += history->getContinuationHistory(board.at(move.from()).type(), move, ply, stack);
        }
    }
}

Move MoveOrder::sortByScore(Movelist &moveList, int scores[], const int &i) {
    for (int j = i + 1; j < moveList.size(); j++) {
        if (scores[j] > scores[i]) {
            std::swap(moveList[i], moveList[j]);
            std::swap(scores[i], scores[j]);
        }
    }
    return moveList[i];
}
