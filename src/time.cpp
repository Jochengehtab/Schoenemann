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

#include "time.h"

void Time::calculateTimeForMove()
{
    // Only use half of out time as maximum
    timeLeft -= timeLeft / 2;
    
    hardLimit = softLimit = timeLeft;

    // Calculate the base and the max time
    int baseTime = (int)(timeLeft * 0.054 + increment * 0.85);
    int maxTime = (int)(timeLeft * 0.76);

    // Calculate the hard limit when we need to stop
    hardLimit = std::min(maxTime, (int)(baseTime * 3.04));

    // Calculate our soft limit
    double bmFactor = 1.3 - 0.05 * bestMoveStabilityCount;
    double evalFactor = 1.3 - 0.05 * bestEvalStabilityCount;
    softLimit = std::min(maxTime, (int)((baseTime * 0.76 * bmFactor * evalFactor)));

    // Make sure that out time doesn't get below 1
    softLimit = std::max(softLimit, 1.0);
    hardLimit = std::max(hardLimit, 1.0);
}

void Time::updateBestMoveStability(Move bestMove, Move previousBestMove)
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

void Time::updateEvalStability(int score, int previousScore)
{
    if (score > (previousScore - 10) && bestEvalStabilityCount < 10)
    {
        bestMoveStabilityCount++;
    }
    else
    {
        bestMoveStabilityCount = 0;
    }
}

void Time::reset()
{
    bestMoveStabilityCount = 0;
    bestEvalStabilityCount = 0;

    hardLimit = 0;
    softLimit = 0;
}

bool Time::shouldStopSoft(std::chrono::steady_clock::time_point start)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    return elapsed.count() > hardLimit;
}

bool Time::shouldStopID(std::chrono::steady_clock::time_point start)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    return elapsed.count() > softLimit;
}