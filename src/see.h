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

#ifndef SEE_H
#define SEE_H

#include "chess.hpp"
using namespace chess;
#include "consts.h"
#include "tune.h"

int getPieceValue(const Board &board, Move &move);
bool see(const Board &board, Move &move, int cutoff);
PieceType getLeastValuableAttacker(const Board &board, Bitboard &occ, Bitboard attackers, Color color);

#endif