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

#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include <array>

#include "nnueconsts.h"

class accumulator {
public:
    std::array<std::int16_t, hiddenSize> white{};
    std::array<std::int16_t, hiddenSize> black{};

    accumulator() {
        zeroAccumulator();
    }

    void loadBias(std::array<std::int16_t, hiddenSize> &bias) {
        std::ranges::copy(bias, std::begin(white));
        std::ranges::copy(bias, std::begin(black));
    }

    void zeroAccumulator() {
        std::ranges::fill(white, 0);
        std::ranges::fill(black, 0);
    }
};

#endif
