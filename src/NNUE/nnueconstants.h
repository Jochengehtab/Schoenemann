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

#include <iostream>

constexpr std::uint16_t inputSize = 768;
constexpr std::uint16_t hiddenSize = 1024;
constexpr std::uint16_t outputSize = 8;
constexpr std::uint16_t scale = 400;

constexpr std::uint8_t QA = 255;
constexpr std::uint8_t QB = 64;
constexpr std::uint32_t inputHiddenSize = inputSize * hiddenSize;
constexpr bool activate = true;
constexpr bool deactivate = false;

constexpr std::uint16_t blackSqures = 64 * 6;
constexpr std::uint8_t whiteSquares = 64;
