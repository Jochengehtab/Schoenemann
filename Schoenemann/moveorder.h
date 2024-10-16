#pragma once

#include <iostream>
#include <array>
#include <cassert>

#include "chess.hpp"
#include "tune.h"
#include "see.h"

using namespace chess;

void orderMoves(Movelist& moveList, Hash* entry, Board& board, int scores[]);
Move sortByScore(Movelist& moveList, int scores[], int i);
