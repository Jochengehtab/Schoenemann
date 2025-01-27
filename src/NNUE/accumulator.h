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

#pragma once

#include <array>

#include "nnueconstants.h"

class accumulator
{
public:
    std::array<std::int16_t, hiddenSize> white;
    std::array<std::int16_t, hiddenSize> black;

    accumulator()
    {
        zeroAccumulator();
    }

    inline void loadBias(std::array<std::int16_t, hiddenSize> &bias)
    {
        std::copy(std::begin(bias), std::end(bias), std::begin(white));
        std::copy(std::begin(bias), std::end(bias), std::begin(black));
    }

    inline void zeroAccumulator()
    {
        std::fill(std::begin(white), std::end(white), 0);
        std::fill(std::begin(black), std::end(black), 0);
    }
};
