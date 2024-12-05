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

struct SearchStack {
	int staticEval;
	std::array<Move, 150> pvLine;
	int pvLength;
	Move killerMove;
};

class Search {
public:
	const int infinity = 32767;

	Move rootBestMove = Move::NULL_MOVE;

	bool shouldStop = false;
	bool isNormalSearch = true;
	bool hasNodeLimit = false;
	int nodeLimit = 0;
	int timeForMove = 0;
	int scoreData = 0;

	int nodes = 0;
	long hardLimit = 0;
	long softLimit = 0;
	int timeLeft = 0;
	int increment = 0;

	Move countinuationButterfly[64][64];
	std::array<std::array<uint8_t, 218>, 150> reductions;
	std::array<SearchStack, 150> stack;

	int pvs(int alpha, int beta, int depth, int ply, Board& board);
	int qs(int alpha, int beta, Board& board, int ply);
	void iterativeDeepening(Board& board, bool isInfinite);
	void initLMR();
	void reset();
	int aspiration(int maxDepth, int score, Board& board);

	std::string getPVLine();

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
