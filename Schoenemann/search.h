#pragma once
#include "chess.hpp"

using namespace chess;

class Search {
public:

	struct SearchStack {
		Move* pv;
		int ply;
		Move currentMove;
		int staticEval;
		int moveCount;
		bool inCheck;
	};

	//Allocate memeory for the stack
	SearchStack stack[256] = {};
	SearchStack* ss = stack + 7;

	const int infinity = 32767;
	Move rootBestMove = Move::NULL_MOVE;
	bool shouldStop = false;
	bool isNormalSearch = true;
	int timeForMove = 0;
	int index = 0;

	int nodes = 0;
	long hardLimit = 0;
	long softLimit = 0;
	Move countinuationButterfly[64][64];

	int pvs(int alpha, int beta, int depth, SearchStack* ss, Board& board);
	int qs(int alpha, int beta, Board& board, SearchStack* ss);
	void iterativeDeepening(Board& board, bool isInfinite);
	int aspiration(int maxDepth, int score, Board& board);
};
