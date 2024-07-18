#pragma once
#include "movegen/chess.hpp"

using namespace chess;

class Search {
public:
	const short infinity = 32767;
	Move bestMove = Move::NULL_MOVE;
	bool shouldStop = false;
	bool isNormalSearch = true;
	int timeForMove = 0;

	int rvpDepth = 6;
	int betaReduction = 70;
	int alphaReduction = 334;
	int razorDepth = 2;
	int depthMultiply = 66;

	//Evaluation consts
	short pawnValue = 126;
	short knightValue = 781;
	short bishopValue = 825;
	short rookValue = 1276;
	short queenValue = 2538;

	int nodes = 0;

	int pvs(int alpha, int beta, int depth, int ply, Board& board);
	int qs(int alpha, int beta, Board& board, int ply);
	void iterativeDeepening(Board& board);
private:
};
