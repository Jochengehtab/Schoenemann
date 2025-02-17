#include "tuneconsts.h"

DEFINE_PARAM_B(seePawn, 140, 80, 275);
DEFINE_PARAM_B(seeKnight, 287, 120, 550);
DEFINE_PARAM_B(seeBishop, 348, 250, 600);
DEFINE_PARAM_B(seeRook, 565, 400, 850);
DEFINE_PARAM_B(seeQueen, 1045, 600, 1350);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

DEFINE_PARAM_B(piecePawn, 73, 40, 175);
DEFINE_PARAM_B(pieceKnight, 258, 170, 450);
DEFINE_PARAM_B(pieceBishop, 217, 185, 500);
DEFINE_PARAM_B(pieceRook, 476, 350, 875);
DEFINE_PARAM_B(pieceQueen, 569, 400, 1000);

int PIECE_VALUES[7] = {piecePawn, pieceKnight, pieceBishop, pieceRook, pieceQueen, 15000, 0};
