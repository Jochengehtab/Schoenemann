#ifndef SEARCH_FWD
#define SEARCH_FWD

#include "chess.hpp"
using namespace chess;

struct alignas(2048) SearchStack
{
	int staticEval;										// (4 Byte)
	std::uint16_t pvLength;								// (2 Byte)				
	bool inCheck;										// (1 Byte)
	std::array<Move, 255> pvLine;						// (1020 Byte)
	Move killerMove = Move::NULL_MOVE;					// (4 Byte)
	PieceType previousMovedPiece = PieceType::NONE;		// (1 Byte)
	Move previousMove = Move::NULL_MOVE;				// (4 Byte)
	Move exludedMove = Move::NULL_MOVE;					// (4 Byte)
};


#endif