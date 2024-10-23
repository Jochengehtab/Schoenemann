#pragma once

#include <chrono>
#include <iostream>

#include "chess.hpp"
#include "timeman.h"
#include "moveorder.h"
#include "consts.h"
#include "see.h"
#include "nnue.h"
#include "tune.h"

using namespace chess;

inline DEFINE_PARAM_S(seePawn, 100, 10);
inline DEFINE_PARAM_S(seeKnight, 300, 30);
inline DEFINE_PARAM_S(seeBishop, 300, 30);
inline DEFINE_PARAM_S(seeRook, 500, 50);
inline DEFINE_PARAM_S(seeQueen, 900, 90);

inline int SEE_PIECE_VALUES[7] = {seePawn, seeKnight, seeBishop, seeRook, seeQueen, 0, 0};

inline DEFINE_PARAM_S(piecePawn, 100, 10);
inline DEFINE_PARAM_S(pieceKnigh, 302, 20);
inline DEFINE_PARAM_S(pieceBishp, 320, 20);
inline DEFINE_PARAM_S(pieceRook, 500, 50);
inline DEFINE_PARAM_S(pieceQuuen, 900, 90);


inline int PIECE_VALUES[7] = {piecePawn, pieceKnigh, pieceBishp, pieceRook, pieceQuuen, 15000, 0};

class Search {
public:
	const int infinity = 32767;
	Move rootBestMove = Move::NULL_MOVE;
	bool shouldStop = false;
	bool isNormalSearch = true;
	int timeForMove = 0;
	int index = 0;

	int nodes = 0;
	long hardLimit = 0;
	long softLimit = 0;
	int timeLeft = 0;
	int increment = 0;
	Move countinuationButterfly[64][64];

	int pvs(int alpha, int beta, int depth, int ply, Board& board);
	int qs(int alpha, int beta, Board& board, int ply);
	void iterativeDeepening(Board& board, bool isInfinite);
	int aspiration(int maxDepth, int score, Board& board);
};
