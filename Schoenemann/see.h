#pragma once

#include "chess.hpp"
#include "consts.h"

using namespace chess;

int getPieceValue(const Board &board, Move &move);
bool see(const Board &board, Move &move, int cutoff);
PieceType getLeastValuableAttacker(const Board &board, Bitboard &occ, Bitboard attackers, Color color);