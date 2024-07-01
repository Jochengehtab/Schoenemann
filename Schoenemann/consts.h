#pragma once
#include <iostream>

#include "tt.h"
#include "movegen/chess.hpp"

const std::string STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

//This value is used for Mate-Distance-Prunning
const int scoreMate = 33000;

extern tt transpositionTabel;