#pragma once

#include <iostream>

#include "consts.h"
#include "see.h"
#include "tune.h"

void orderMoves(Movelist &moveList, Hash *entry, Board &board, int scores[], Move killer, int ply);
Move sortByScore(Movelist &moveList, int scores[], int i);
