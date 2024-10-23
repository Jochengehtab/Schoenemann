#pragma once

#include <iostream>

#include "consts.h"
#include "see.h"

using namespace chess;

void orderMoves(Movelist& moveList, Hash* entry, Board& board, int scores[]);
Move sortByScore(Movelist& moveList, int scores[], int i);
