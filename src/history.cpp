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

#include <cassert>

#include "tune.h"

DEFINE_PARAM(quietHistoryDiv, 28000, 10000, 50000);
DEFINE_PARAM(continuationHistoryDiv, 28000, 10000, 50000);
DEFINE_PARAM(correctionValueDiv, 30, 1, 600);

int History::getQuietHistory(const Board &board, const Move move) const {
    return quietHistory[board.sideToMove()][board.at(move.from()).type()][move.to().index()];
}

void History::updateQuietHistory(const Board &board, const Move move, const int bonus) {
    quietHistory
            [board.sideToMove()]
            [board.at(move.from()).type()]
            [move.to().index()] +=
            bonus - getQuietHistory(board, move) * std::abs(bonus) / quietHistoryDiv;
}

int History::getContinuationHistory(PieceType piece, const Move move, int ply, const SearchStack *stack) const {
    const int to = move.to().index();
    int score = 0;

    assert(piece != PieceType::NONE);

    if (ply > 0 && stack[ply - 1].previousMovedPiece != PieceType::NONE) {
        score += 2 * continuationHistory[stack[ply - 1].previousMovedPiece]
                [stack[ply - 1].previousMove.to().index()]
                [piece]
                [to];
    }
    if (ply > 1 && stack[ply - 2].previousMovedPiece != PieceType::NONE) {
        score += continuationHistory[stack[ply - 2].previousMovedPiece]
                [stack[ply - 2].previousMove.to().index()]
                [piece]
                [to];
    }


    return score;
}

void History::updateContinuationHistory(const PieceType piece, const Move move, const int bonus, const int ply,
                                        const SearchStack *stack) {
    assert(piece != PieceType::NONE);

    const int current = getContinuationHistory(piece, move, ply, stack);
    const int gravity = bonus - current * std::abs(bonus) / continuationHistoryDiv;

    const int to = move.to().index();

    if (ply > 0 && stack[ply - 1].previousMovedPiece != PieceType::NONE) {
        continuationHistory[stack[ply - 1].previousMovedPiece]
                [stack[ply - 1].previousMove.to().index()]
                [piece]
                [to] += gravity;
    }

    if (ply > 1 && stack[ply - 2].previousMovedPiece != PieceType::NONE) {
        continuationHistory[stack[ply - 2].previousMovedPiece]
                [stack[ply - 2].previousMove.to().index()]
                [piece]
                [to] += gravity;
    }
}

void History::updatePawnCorrectionHistory(const int bonus, const Board &board, const int div) {
    const std::uint64_t pawnHash = getPieceKey(PieceType::PAWN, board);
    // Gravity
    const int scaledBonus = bonus - pawnCorrectionHistory[board.sideToMove()][
                                pawnHash & pawnCorrectionHistorySize - 1] * std::abs(bonus) / div;
    pawnCorrectionHistory[board.sideToMove()][pawnHash & pawnCorrectionHistorySize - 1] += scaledBonus;
}

int History::correctEval(const int rawEval, const Board &board) const {
    const int pawnEntry = pawnCorrectionHistory[board.sideToMove()][
        getPieceKey(PieceType::PAWN, board) & pawnCorrectionHistorySize - 1];

    const int corrHistoryBonus = pawnEntry;

    return rawEval + corrHistoryBonus / correctionValueDiv;
}

std::uint64_t History::getPieceKey(const PieceType piece, const Board &board) {
    std::uint64_t key = 0;
    Bitboard bitboard = board.pieces(piece);
    while (bitboard) {
        const Square square = bitboard.pop();
        key ^= Zobrist::piece(board.at(square), square);
    }
    return key;
}

void History::resetHistories() {
    std::memset(&quietHistory, 0, sizeof(quietHistory));
    std::memset(&continuationHistory, 0, sizeof(continuationHistory));
    std::memset(&pawnCorrectionHistory, 0, sizeof(pawnCorrectionHistory));
}
