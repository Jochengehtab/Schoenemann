#pragma once
#include "movegen/chess.hpp"

using namespace chess;

class Search {
public:
	const int infinity = 32767;
	Move bestMove = Move::NULL_MOVE;
	bool shouldStop = false;
	bool isNormalSearch = true;
	int timeForMove = 0;
	int index = 0;
	std::uint64_t repitions[256] = {0};

	bool contains(std::uint64_t key)
	{
		for (size_t i = 0; i < sizeof(repitions) / sizeof(std::uint64_t); i++)
		{
			if (key == repitions[i])
			{
				return true;
			}
		}
		return false;
	}

	void storeKey(std::uint64_t key)
	{
		repitions[index] = key;
		index++;
		if (index == 255)
		{
			index = 0;
		}
	}

	int nodes = 0;

	int pvs(int alpha, int beta, int depth, int ply, Board& board);
	int qs(int alpha, int beta, Board& board, int ply);
	void iterativeDeepening(Board& board);
private:
};
