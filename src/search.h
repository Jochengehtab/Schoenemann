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

#include <iostream>

#include "time.h"
#include "tt.h"
#include "moveorder.h"
#include "search_fwd.h"

class Search
{
public:
	Search(Time &timeManagement,
		   tt &transpositionTabel,
		   MoveOrder &moveOrder,
		   Network &net) : timeManagement(timeManagement),
						   transpositionTabel(transpositionTabel),
						   moveOrder(moveOrder),
						   net(net) {}

	Move rootBestMove = Move::NULL_MOVE;
	Move previousBestMove = Move::NULL_MOVE;

	bool shouldStop = false;
	bool isNormalSearch = true;
	bool hasNodeLimit = false;

	std::uint64_t nodeLimit = 0;
	std::uint64_t timeForMove = 0;
	std::uint64_t nodes = 0;

	int scoreData = 0;
	int previousBestScore = 0;

	std::array<std::array<std::uint8_t, 218>, 255> reductions;
	SearchStack stack[256];

	int pvs(std::int16_t alpha, std::int16_t beta, std::int16_t depth, std::int16_t ply, Board &board, bool isCutNode);
	int qs(std::int16_t alpha, std::int16_t beta, Board &board, std::int16_t ply);

	int scaleOutput(int rawEval, Board &board);

	void iterativeDeepening(Board &board, bool isInfinite);
	void initLMR();
	void resetHistory();

private:
	int aspiration(std::int16_t maxDepth, std::int16_t score, Board &board);
	std::string getPVLine();
	Time &timeManagement;
	tt &transpositionTabel;
	History history;
	MoveOrder &moveOrder;
	Network &net;
};

#endif