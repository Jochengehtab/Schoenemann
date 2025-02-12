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

DEFINE_PARAM_B(baseTimeMul, 54, 25, 99);
DEFINE_PARAM_B(baseTimeIncrement, 85, 40, 999);

DEFINE_PARAM_B(maxTimeMul, 76, 40, 999);

DEFINE_PARAM_B(hardMul, 304, 100, 999);
DEFINE_PARAM_B(softMul, 76, 40, 900);

void getTimeForMove()
{
  searcher.timeLeft -= searcher.timeLeft / 2;
  searcher.hardLimit = searcher.softLimit = searcher.timeLeft;

  double b1 = (baseTimeMul / 1000.0);
  double b2 = (baseTimeIncrement / 100.0);
  double m1 = (maxTimeMul / 100.0);
  double h1 = (hardMul / 100.0);
  double s1 = (softMul / 100.0);

  double baseTime = (searcher.timeLeft * b1 + searcher.increment * b2);
  double maxTime = (searcher.timeLeft * m1);

  searcher.hardLimit = std::min(maxTime, (baseTime * h1));
  std::cout << b1 << std::endl;
  searcher.softLimit = std::min(maxTime, (baseTime * s1));
}
