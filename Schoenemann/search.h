#pragma once
#include "chess.hpp"

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
	Move countinuationButterfly[64][64];

	template <typename T, std::size_t N, std::size_t... Ns>
	struct MultiArrayImpl
	{
    	using Type = std::array<typename MultiArrayImpl<T, Ns...>::Type, N>;
	};

	template <typename T, std::size_t N>
	struct MultiArrayImpl<T, N>
	{
    	using Type = std::array<T, N>;
	};

	template <typename T, std::size_t... Ns>
	using MultiArray = typename MultiArrayImpl<T, Ns...>::Type;

	MultiArray<std::uint32_t, 2, 256, 256> LMRTable;

	int pvs(int alpha, int beta, int depth, int ply, Board& board);
	int qs(int alpha, int beta, Board& board, int ply);
	void initLMR();
	void iterativeDeepening(Board& board, bool isInfinite);
	int aspiration(int maxDepth, int score, Board& board);
};
