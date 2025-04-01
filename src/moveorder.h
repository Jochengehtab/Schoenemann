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

#ifndef MOVEORDER_H
#define MOVEORDER_H

#include "search_fwd.h"
#include "tt.h"
#include "history.h"

class MoveOrder
{
private:
public:
    void orderMoves(History* history, Movelist &moveList, Hash *entry, Move& killer, SearchStack* stack, Board &board, int* scores, int ply);
    Move sortByScore(Movelist &moveList, int scores[], int i);
};

#endif
