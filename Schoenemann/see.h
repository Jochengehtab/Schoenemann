#include "chess.hpp"

using namespace chess;

const int SEE_PIECE_VALUES[7] = {100, 300, 300, 500, 900, 0, 0};

int getPieceValue(const Board &board, Move &move);
bool see(const Board &board, Move &move, int cutoff);
PieceType getLeastValuableAttacker(const Board &board, Bitboard &occ, Bitboard attackers, Color color);