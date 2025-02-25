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

#include "timeman.h"

int bestMoveStabilityCount = 0;

void getTimeForMove()
{
  searcher.timeLeft -= searcher.timeLeft / 2;
  searcher.hardLimit = searcher.softLimit = searcher.timeLeft;

  int baseTime = (int)(searcher.timeLeft * 0.054 + searcher.increment * 0.85);
  int maxTime = (int)(searcher.timeLeft * 0.76);

  searcher.hardLimit = std::min(maxTime, (int)(baseTime * 3.04));
  double bmFactor = 1.3 - 0.05 * bestMoveStabilityCount;
  searcher.softLimit = std::min(maxTime, (int)((baseTime * 0.76 * bmFactor)));
}

void updateBestMoveStability(Move bestMove, Move previousBestMove)
{
  if (bestMove == previousBestMove && bestMoveStabilityCount < 10)
  {
    bestMoveStabilityCount++;
  }
  else
  {
    bestMoveStabilityCount = 0;
  }
  
}

bool shouldStopSoft(std::chrono::steady_clock::time_point start)
{
  std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
  return elapsed.count() > searcher.hardLimit;
}

bool shouldStopID(std::chrono::steady_clock::time_point start)
{
  std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
  return elapsed.count() > searcher.softLimit;
}