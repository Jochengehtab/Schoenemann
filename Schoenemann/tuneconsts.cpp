#include "tuneconsts.h"

 DEFINE_PARAM_S(seePawn, 100, 10);
 DEFINE_PARAM_S(seeKnight, 300, 30);
 DEFINE_PARAM_S(seeBishop, 300, 30);
 DEFINE_PARAM_S(seeRook, 500, 50);
 DEFINE_PARAM_S(seeQueen, 900, 90);

int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

 DEFINE_PARAM_S(piecePawn, 100, 10);
 DEFINE_PARAM_S(pieceKnigh, 302, 20);
 DEFINE_PARAM_S(pieceBishp, 320, 20);
 DEFINE_PARAM_S(pieceRook, 500, 50);
 DEFINE_PARAM_S(pieceQuuen, 900, 90);


 int PIECE_VALUES[7] = {piecePawn, pieceKnigh, pieceBishp, pieceRook, pieceQuuen, 15000, 0};
