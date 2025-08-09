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
#ifndef HISTORY_H
#define HISTORY_H

#include "search_fwd.h"

class History {
    int quietHistory[2][7][64] = {};
    int continuationHistory[6][64][6][64] = {};
    int pawnCorrectionHistory[2][16384] = {};

private:
    static std::uint64_t getPieceKey(PieceType piece, const Board &board);

    const std::uint16_t pawnCorrectionHistorySize = 16384;

public:
    [[nodiscard]] int getQuietHistory(const Board &board, Move move) const;

    int getContinuationHistory(PieceType piece, Move move, int ply, const SearchStack *stack) const;

    int correctEval(int rawEval, const Board &board) const;

    void updateQuietHistory(const Board &board, Move move, int bonus);

    void updatePawnCorrectionHistory(int bonus, const Board &board, int div);

    void updateContinuationHistory(PieceType piece, Move move, int bonus, int ply, const SearchStack *stack);

    void resetHistories();
};

#endif
