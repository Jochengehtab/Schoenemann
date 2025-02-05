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

#include "helper.h"

void transpositionTableTest(Board &board)
{
	// Set up a unique position
	board.setFen("3N4/2p5/5K2/k1PB3p/3Pr3/1b5p/6p1/5nB1 w - - 0 1");
	std::uint64_t key = board.hash();

	// Store the information
	transpositionTabel.storeEvaluation(key, 2, LOWER_BOUND, transpositionTabel.scoreToTT(200, 1), uci::uciToMove(board, "d5e4"), 1);

	// Try to get the information out of the table
	Hash *entry = transpositionTabel.getHash(key);

	if (entry == nullptr)
	{
		std::cout << "The entry is a nullptr" << std::endl;
		return;
	}
	std::uint64_t hashedKey = entry->key;
	short hashedDepth = entry->depth;
	short hashedType = entry->type;
	int hashedScore = entry->score;
	Move hashedMove = entry->move;

	if (hashedKey == key)
	{
		std::cout << "Key PASSED." << std::endl;
	}
	else
	{
		std::cout << "Key FAILED." << "Original key: \n"
				  << key << "\nHash key: \n"
				  << hashedKey << std::endl;
	}

	if (hashedDepth == 2)
	{
		std::cout << "Depth PASSED." << std::endl;
	}
	else
	{
		std::cout << "Depth FAILED." << "Original depth: 2" << "\nHash key: " << hashedDepth << std::endl;
	}

	if (hashedType == LOWER_BOUND)
	{
		std::cout << "Type PASSED." << std::endl;
	}
	else
	{
		std::cout << "Type FAILED." << "Original type: 2" << "\nHash type: " << hashedType << std::endl;
	}

	if (hashedScore == 200)
	{
		std::cout << "Score PASSED." << std::endl;
	}
	else
	{
		std::cout << "Score FAILED." << "Original score: 200" << "\nHash score: " << hashedScore << std::endl;
	}

	if (hashedMove == uci::uciToMove(board, "d5e4"))
	{
		std::cout << "Move PASSED." << std::endl;
	}
	else
	{
		std::cout << "Move FAILED." << "Original move: d5e4" << "\nHash move: " << hashedMove << std::endl;
	}
	board.setFen(STARTPOS);
}

void testCommand()
{
	Board testBoard;
	testBoard.setFen("8/4p3/8/8/8/8/8/8 w - - 0 1");
}

// Print the uci info
void uciPrint()
{
	std::cout << "id name Schoenemann" << std::endl
			  << "option name Hash type spin default 64 min 1 max 4096" << std::endl
			  << "option name Threads type spin default 1 min 1 max 1" << std::endl;
}

void runBenchmark()
{
	// Setting up the bench Board
	Board benchBoard;

	// Setting up the clock
	auto start = std::chrono::high_resolution_clock::now();

	// Reseting the nodes
	searcher.nodes = 0;

	// Looping over all bench positions
	for (const auto &test : testStrings)
	{
		benchBoard.setFen(test);
		searcher.pvs(-infinity, infinity, benchDepth, 0, benchBoard, false);
	}

	auto end = std::chrono::high_resolution_clock::now();

	// Calculates the total time used
	std::chrono::duration<double, std::milli> timeElapsed = end - start;
	int timeInMs = static_cast<int>(timeElapsed.count());

	// calculates the Nodes per Second
	int NPS = static_cast<int>(searcher.nodes / timeElapsed.count() * 1000);

	// Prints out the final bench
	std::cout << "Time  : " << timeInMs << " ms\nNodes : " << searcher.nodes << "\nNPS   : " << NPS << std::endl;
}
