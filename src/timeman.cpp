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

void TimeManagement::calculateTimeForMove() {
    if (moveTime != -1) {
        hardLimit = std::chrono::milliseconds{moveTime};
        softLimit = std::chrono::milliseconds{moveTime};
        return;
    }

    // Only use half of out time as maximum
    timeLeft -= timeLeft / 2;

    hardLimit = std::chrono::milliseconds{timeLeft};
    softLimit = std::chrono::milliseconds{timeLeft};

    // Calculate the base and the max time
    const int baseTime = static_cast<int>(timeLeft * 0.054 + increment * 0.85);
    const int maxTime = static_cast<int>(timeLeft * 0.76);

    int hardMs = std::min(maxTime, static_cast<int>(baseTime * 3.04));

    const double bmFactor = 1.3 - 0.05 * bestMoveStabilityCount;
    const double evalFactor = 1.3 - 0.05 * bestEvalStabilityCount;
    int softMs = std::min(maxTime, static_cast<int>(baseTime * 0.76 * bmFactor * evalFactor));

    // Ensure we never go below 1 ms.
    hardMs = std::max(hardMs, 1);
    softMs = std::max(softMs, 1);

    hardLimit = std::chrono::milliseconds{hardMs};
    softLimit = std::chrono::milliseconds{softMs};
}

void TimeManagement::updateBestMoveStability(const Move bestMove, const Move previousBestMove) {
    if (bestMove == previousBestMove && bestMoveStabilityCount < 10) {
        bestMoveStabilityCount++;
    } else {
        bestMoveStabilityCount = 0;
    }
}

void TimeManagement::updateEvalStability(const int score, const int previousScore) {
    if (score > previousScore - 10 && bestEvalStabilityCount < 10) {
        bestMoveStabilityCount++;
    } else {
        bestMoveStabilityCount = 0;
    }
}

void TimeManagement::reset() {
    bestMoveStabilityCount = 0;
    bestEvalStabilityCount = 0;
    moveTime = -1;

    hardLimit = std::chrono::milliseconds{0};
    softLimit = std::chrono::milliseconds{0};
}


// TODO check if this is really correct
[[nodiscard]] bool TimeManagement::shouldStopSoft(const std::chrono::steady_clock::time_point &start) const noexcept {
    if (isInfiniteSearch) {
        return false;
    }
    return std::chrono::steady_clock::now() - start > hardLimit;
}

[[nodiscard]] bool TimeManagement::shouldStopID(const std::chrono::steady_clock::time_point &start) const noexcept {
    if (isInfiniteSearch) {
        return false;
    }
    return std::chrono::steady_clock::now() - start > softLimit;
}
