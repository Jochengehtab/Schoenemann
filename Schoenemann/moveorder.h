#include <iostream>

#include "tt.h"

#include "chess.hpp"
#include "consts.h"
#include "tt.h"
#include <array>
#include <cassert>

using namespace chess;

void orderMoves(Movelist moveList, Hash* entry, Board& board, int scores[]);
Move sortByScore(Movelist moveList, int scores[], int i);
const int hashMoveScore = 2'147'483'647, promotion = 1'000'000'000, goodCapture = 1'500'000'000, badCapture = -500'000'000;
const int PIECE_VALUES[7] = {100, 302, 320, 500, 900, 15000, 0};
