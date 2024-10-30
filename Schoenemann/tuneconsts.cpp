#include "tuneconsts.h"

DEFINE_PARAM_S(seePawn, 107, 10);
DEFINE_PARAM_S(seeKnight, 337, 30);
DEFINE_PARAM_S(seeBishop, 338, 30);
DEFINE_PARAM_S(seeRook, 464, 50);
DEFINE_PARAM_S(seeQueen, 864, 90);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_S(piecePawn, 105, 10);
DEFINE_PARAM_S(pieceKnigh, 287, 20);
DEFINE_PARAM_S(pieceBishp, 300, 20);
DEFINE_PARAM_S(pieceRook, 438, 50);
DEFINE_PARAM_S(pieceQuuen, 859, 90);


int PIECE_VALUES[7] = {piecePawn, pieceKnigh, pieceBishp, pieceRook, pieceQuuen, 15000, 0};
