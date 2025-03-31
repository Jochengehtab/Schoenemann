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

#ifndef TIME_H
#define TIME_H

#include "chess.hpp"
using namespace chess;

class Time
{
private:
    std::uint16_t bestMoveStabilityCount = 0;
    std::uint16_t bestEvalStabilityCount = 0;

    long hardLimit = 0;
    long softLimit = 0;

public:
    void calculateTimeForMove();
    void updateBestMoveStability(Move bestMove, Move previousBestMove);
    void updateEvalStability(int score, int previousScore);

    bool shouldStopSoft(std::chrono::steady_clock::time_point start);
    bool shouldStopID(std::chrono::steady_clock::time_point start);

    int timeLeft = 0;
    int increment = 0;
};

#endif