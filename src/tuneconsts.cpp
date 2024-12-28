#include "tuneconsts.h"

DEFINE_PARAM_S(seePawn, 109, 10);
DEFINE_PARAM_S(seeKnight, 345, 30);
DEFINE_PARAM_S(seeBishop, 364, 30);
DEFINE_PARAM_S(seeRook, 429, 50);
DEFINE_PARAM_S(seeQueen, 949, 90);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_S(piecePawn, 92, 10);
DEFINE_PARAM_S(pieceKnight, 265, 20);
DEFINE_PARAM_S(pieceBishop, 280, 20);
DEFINE_PARAM_S(pieceRook, 464, 50);
DEFINE_PARAM_S(pieceQueen, 824, 90);


int PIECE_VALUES[7] = {piecePawn, pieceKnight, pieceBishop, pieceRook, pieceQueen, 15000, 0};
