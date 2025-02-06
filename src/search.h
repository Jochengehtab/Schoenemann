/*
  This file is part of the Schoenemann chess engine written by Jochengehtab

  Copyright (C) 2024-2025 Jochengehtab

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <chrono>
#include <iostream>

#include "timeman.h"
#include "moveorder.h"
#include "consts.h"
#include "see.h"
#include "tune.h"

struct SearchStack
{
	int staticEval;
	int pvLength;
	bool inCheck;
	std::array<Move, 150> pvLine;
	Move killerMove = Move::NULL_MOVE;
	PieceType previousMovedPiece = PieceType::NONE;
	Move previousMove = Move::NULL_MOVE;
	Move exludedMove = Move::NULL_MOVE;
};

class Search
{
public:
	const int infinity = 32767;

	Move rootBestMove = Move::NULL_MOVE;

	bool shouldStop = false;
	bool isNormalSearch = true;
	bool hasNodeLimit = false;

	int nodeLimit = 0;
	int timeForMove = 0;
	int scoreData = 0;
	int timeLeft = 0;
	int increment = 0;
	int nodes = 0;

	long hardLimit = 0;
	long softLimit = 0;
	
	std::array<std::array<std::array<int, 64>, 6>, 2> quietHistory;
	std::array<std::array<std::array<std::array<int, 6>, 64>, 6>, 64> continuationHistory;
	std::array<std::array<std::uint8_t, 218>, 150> reductions;
	std::array<SearchStack, 150> stack;

	int pvs(int alpha, int beta, int depth, int ply, Board &board, bool isCutNode);
	int qs(int alpha, int beta, Board &board, int ply);
	int aspiration(int maxDepth, int score, Board &board);
	int scaleOutput(int rawEval, Board &board);
	int getQuietHistory(Board &board, Move move);
	int getContinuationHistory(PieceType piece, Move move, int ply);

	void iterativeDeepening(Board &board, bool isInfinite);
	void initLMR();
	void updateQuietHistory(Board &board, Move move, int bonus);
	void updateContinuationHistory(PieceType piece, Move move, int bonus, int ply);

	std::string getPVLine();

	bool shouldStopSoft(auto s)
	{
		std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - s;
		return elapsed.count() > hardLimit;
	}

	bool shouldStopID(auto s)
	{
		std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - s;
		return elapsed.count() > softLimit;
	}
};
