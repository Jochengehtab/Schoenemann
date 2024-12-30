#include "tuneconsts.h"

DEFINE_PARAM_S(seePawn, 140, 10);
DEFINE_PARAM_S(seeKnight, 287, 30);
DEFINE_PARAM_S(seeBishop, 348, 30);
DEFINE_PARAM_S(seeRook, 565, 50);
DEFINE_PARAM_S(seeQueen, 1045, 90);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_S(piecePawn, 73, 10);
DEFINE_PARAM_S(pieceKnight, 258, 20);
DEFINE_PARAM_S(pieceBishop, 217, 20);
DEFINE_PARAM_S(pieceRook, 476, 50);
DEFINE_PARAM_S(pieceQueen, 569, 90);

int PIECE_VALUES[7] = {piecePawn, pieceKnight, pieceBishop, pieceRook, pieceQueen, 15000, 0};
