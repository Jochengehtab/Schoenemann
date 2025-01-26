#pragma once

#include "consts.h"
#include "tune.h"

int getPieceValue(const Board &board, Move &move);
bool see(const Board &board, Move &move, int cutoff);
PieceType getLeastValuableAttacker(const Board &board, Bitboard &occ, Bitboard attackers, Color color);