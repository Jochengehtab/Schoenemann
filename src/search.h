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

#ifndef SEARCH_H
#define SEARCH_H

#include <chrono>
#include <iostream>

#include "consts.h"

struct SearchStack
{
	int staticEval;
	int pvLength;
	bool inCheck;
	std::array<Move, 256> pvLine;
	Move killerMove = Move::NULL_MOVE;
	PieceType previousMovedPiece = PieceType::NONE;
	Move previousMove = Move::NULL_MOVE;
	Move exludedMove = Move::NULL_MOVE;
};

class Search
{
public:
	const int infinity = 32767;
	const int CORRHIST_LIMIT = 1024;
	const int MAX_PLY = 256;

	Move rootBestMove = Move::NULL_MOVE;
	Move previousBestMove = Move::NULL_MOVE;

	bool shouldStop = false;
	bool isNormalSearch = true;
	bool hasNodeLimit = false;

	int nodeLimit = 0;
	int timeForMove = 0;
	int scoreData = 0;
	int nodes = 0;
	int previousBestScore = 0;

	std::array<std::array<std::uint8_t, 218>, 256> reductions;
	SearchStack stack[256];

	int pvs(int alpha, int beta, int depth, int ply, Board &board, bool isCutNode);
	int qs(int alpha, int beta, Board &board, int ply);

	int scaleOutput(int rawEval, Board &board);

	void iterativeDeepening(Board &board, bool isInfinite);
	void initLMR();

private:
	int aspiration(int maxDepth, int score, Board &board);
	std::string getPVLine();
};

#endif