#include "tuneconsts.h"

DEFINE_PARAM_S(seePawn, 104, 10);
DEFINE_PARAM_S(seeKnight, 339, 30);
DEFINE_PARAM_S(seeBishop, 340, 30);
DEFINE_PARAM_S(seeRook, 463, 50);
DEFINE_PARAM_S(seeQueen, 830, 90);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_S(piecePawn, 107, 10);
DEFINE_PARAM_S(pieceKnight, 281, 20);
DEFINE_PARAM_S(pieceBishop, 293, 20);
DEFINE_PARAM_S(pieceRook, 458, 50);
DEFINE_PARAM_S(pieceQueen, 804, 90);


int PIECE_VALUES[7] = {piecePawn, pieceKnight, pieceBishop, pieceRook, pieceQueen, 15000, 0};
