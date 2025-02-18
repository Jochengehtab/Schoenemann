#include "tuneconsts.h"

DEFINE_PARAM_B(seePawn, 68, 80, 275);
DEFINE_PARAM_B(seeKnight, 245, 120, 550);
DEFINE_PARAM_B(seeBishop, 311, 250, 600);
DEFINE_PARAM_B(seeRook, 487, 400, 850);
DEFINE_PARAM_B(seeQueen, 702, 600, 1350);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_B(piecePawn, 71, 40, 175);
DEFINE_PARAM_B(pieceKnight, 200, 170, 450);
DEFINE_PARAM_B(pieceBishop, 214, 185, 500);
DEFINE_PARAM_B(pieceRook, 626, 350, 875);
DEFINE_PARAM_B(pieceQueen, 858, 400, 1000);

int PIECE_VALUES[7] = {piecePawn, pieceKnight, pieceBishop, pieceRook, pieceQueen, 15000, 0};
