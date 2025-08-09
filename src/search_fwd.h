#ifndef SEARCH_FWD
#define SEARCH_FWD

#include "consts.h"
#include "chess.hpp"
using namespace chess;

struct alignas(1024) SearchStack {
    int staticEval = EVAL_NONE; // (4 Byte)
    std::uint16_t pvLength = 0; // (2 Byte)
    bool inCheck = false; // (1 Byte)
    Move pvLine[MAX_PLY]; // (980 Byte)
    Move killerMove = Move::NULL_MOVE; // (4 Byte)
    PieceType previousMovedPiece = PieceType::NONE; // (1 Byte)
    Move previousMove = Move::NULL_MOVE; // (4 Byte)
    Move excludedMove = Move::NULL_MOVE; // (4 Byte)
};

struct alignas(8) RootMove {
    Move move = Move::NULL_MOVE;
    int score = EVAL_NONE;
};

#endif
