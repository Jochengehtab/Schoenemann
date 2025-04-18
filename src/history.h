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

class History
{
    std::array<std::array<std::array<int, 64>, 6>, 2> quietHistory;
    std::array<std::array<std::array<std::array<int, 6>, 64>, 6>, 64> continuationHistory;
    std::array<std::array<int, 2>, 16384> pawnCorrectionHistory;

private:
    std::uint64_t getPieceKey(PieceType piece, const Board &board);
    const std::uint16_t pawnCorrectionHistorySize = 16384;

public:
    int getQuietHistory(Board &board, Move move);
    int getContinuationHistory(PieceType piece, Move move, std::int16_t ply, SearchStack *stack);
    int correctEval(int rawEval, Board &board);
    void updateQuietHistory(Board &board, Move move, int bonus);
    void updatePawnCorrectionHistory(int bonus, Board &board, int div);
    void updateContinuationHistory(PieceType piece, Move move, int bonus, std::int16_t ply, SearchStack *stack);
    void resetHistorys();
};

#endif