#include <iostream>
#include <array>
#include <cassert>

#include "chess.hpp"
#include "tune.h"
#include "see.h"
#include "tune.h"

using namespace chess;

void orderMoves(Movelist& moveList, Hash* entry, Board& board, int scores[]);
Move sortByScore(Movelist& moveList, int scores[], int i);
const int hashMoveScore = 2'147'483'647, promotion = 1'000'000'000, goodCapture = 1'500'000'000, badCapture = -500'000'000;
