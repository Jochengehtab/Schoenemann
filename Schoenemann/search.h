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
#include "tuneconsts.h"

using namespace chess;

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
		bool shouldStopSoft(auto s)
	{
    	std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
    	return elapsed.count() > hardLimit;
	}

	bool shouldStopID(auto s)
	{
	    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
   		return elapsed.count() > softLimit;
	}
};
