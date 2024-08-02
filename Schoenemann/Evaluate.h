#pragma once

#include <iostream>

#include "consts.h"
#include "psqt.h"
#include "consts.h"
#include "movegen/chess.hpp"

using namespace chess;

int evaluate(Board& board);
int countMaterial(Board& board, Color color);
int countAmount(Board& board, PieceType type, Color color);
int getComplexity(Board& borad, Color color);