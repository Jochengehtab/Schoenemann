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

#include <cstring>

#include "history.h"
#include "tune.h"

DEFINE_PARAM_B(quietHistoryDiv, 28711, 10000, 50000);
DEFINE_PARAM_B(continuationHistoryDiv, 28156, 10000, 50000);
DEFINE_PARAM_B(correctionValueDiv, 30, 1, 600);

int History::getQuietHistory(Board &board, Move move)
{
    return quietHistory[board.sideToMove()][board.at(move.from()).type()][move.to().index()];
}

void History::updateQuietHistory(Board &board, Move move, int bonus)
{
    quietHistory
        [board.sideToMove()]
        [board.at(move.from()).type()]
        [move.to().index()] +=
        (bonus - getQuietHistory(board, move) * std::abs(bonus) / quietHistoryDiv);
}

int History::getContinuationHistory(PieceType piece, Move move, std::int16_t ply, SearchStack *stack)
{
    return continuationHistory[stack[ply].previousMovedPiece][stack[ply].previousMove.to().index()][piece][move.to().index()];
}

void History::updateContinuationHistory(PieceType piece, Move move, int bonus, std::int16_t ply, SearchStack *stack)
{
    // Continuation History is indexed as follows
    // | Ply - 1 Moved Piece From | Ply - 1 Move To Index | Moved Piece From | Move To Index |
    int gravity = (bonus - getContinuationHistory(piece, move, ply - 1, stack));
    int scaledBonus = (gravity * std::abs(bonus) / continuationHistoryDiv);

    if (stack[ply - 1].previousMovedPiece != PieceType::NONE)
    {
        // Continuation History is indexed as follows
        // | Ply - 1 Moved Piece From | Ply - 1 Move To Index | Moved Piece From | Move To Index |
        continuationHistory[stack[ply - 1].previousMovedPiece][stack[ply - 1].previousMove.to().index()][piece][move.to().index()] += scaledBonus;
    }
}

void History::updatePawnCorrectionHistory(int bonus, Board &board, int div)
{
    int pawnHash = getPieceKey(PieceType::PAWN, board);
    // Gravity
    int scaledBonus = bonus - pawnCorrectionHistory[board.sideToMove()][pawnHash & (pawnCorrectionHistorySize - 1)] * std::abs(bonus) / div;
    pawnCorrectionHistory[board.sideToMove()][pawnHash & (pawnCorrectionHistorySize - 1)] += scaledBonus;
}

int History::correctEval(int rawEval, Board &board)
{
    int pawnEntry = pawnCorrectionHistory[board.sideToMove()][getPieceKey(PieceType::PAWN, board) & (pawnCorrectionHistorySize - 1)];

    int corrHistoryBonus = pawnEntry;

    return rawEval + corrHistoryBonus / correctionValueDiv;
}

std::uint64_t History::getPieceKey(PieceType piece, const Board &board)
{
    std::uint64_t key = 0;
    Bitboard bitboard = board.pieces(piece);
    while (bitboard)
    {
        const Square square = bitboard.pop();
        key ^= Zobrist::piece(board.at(square), square);
    }
    return key;
}

void History::resetHistorys()
{
    std::memset(&quietHistory, 0, sizeof(quietHistory));
    std::memset(&continuationHistory, 0, sizeof(continuationHistory));
    std::memset(&pawnCorrectionHistory, 0, sizeof(pawnCorrectionHistory));
}