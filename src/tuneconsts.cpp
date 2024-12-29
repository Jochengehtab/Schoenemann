#include "tuneconsts.h"

DEFINE_PARAM_S(seePawn, 136, 10);
DEFINE_PARAM_S(seeKnight, 320, 30);
DEFINE_PARAM_S(seeBishop, 341, 30);
DEFINE_PARAM_S(seeRook, 549, 50);
DEFINE_PARAM_S(seeQueen, 1069, 90);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_S(piecePawn, 74, 10);
DEFINE_PARAM_S(pieceKnight, 255, 20);
DEFINE_PARAM_S(pieceBishop, 220, 20);
DEFINE_PARAM_S(pieceRook, 524, 50);
DEFINE_PARAM_S(pieceQueen, 603, 90);


int PIECE_VALUES[7] = {piecePawn, pieceKnight, pieceBishop, pieceRook, pieceQueen, 15000, 0};
