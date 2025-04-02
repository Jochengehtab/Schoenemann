#ifndef SEARCH_FWD
#define SEARCH_FWD

#include "chess.hpp"
using namespace chess;

struct SearchStack
{
	int staticEval;
	std::uint16_t pvLength;
	bool inCheck;
	std::array<Move, 256> pvLine;
	Move killerMove = Move::NULL_MOVE;
	PieceType previousMovedPiece = PieceType::NONE;
	Move previousMove = Move::NULL_MOVE;
	Move exludedMove = Move::NULL_MOVE;
};


#endif