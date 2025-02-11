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

DEFINE_PARAM_B(baseTimeMul, 54, 1, 1500);
DEFINE_PARAM_B(baseTimeIncrement, 85, 1, 2500);

DEFINE_PARAM_B(maxTimeMul, 76, 1, 2500);

DEFINE_PARAM_B(hardMul, 304, 50, 3000);
DEFINE_PARAM_B(softMul, 76, 10, 1800);

void getTimeForMove()
{
	searcher.timeLeft -= searcher.timeLeft / 2;
	searcher.hardLimit = searcher.softLimit = searcher.timeLeft;

	int baseTime = (int) (searcher.timeLeft * (baseTimeMul / 1000) + searcher.increment * (baseTimeIncrement / 100));
	int maxTime = (int) (searcher.timeLeft * (maxTimeMul / 100));

	searcher.hardLimit = std::min(maxTime, (int) (baseTime * (hardMul / 100)));
	searcher.softLimit = std::min(maxTime, (int) (baseTime * (softMul / 100)));
}
