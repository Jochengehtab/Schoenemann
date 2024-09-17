#pragma once

#include "consts.h"
#include "chess.hpp"

#include <chrono>

void transpositionTableTest(Board& board);
void runBenchmark();
void uciPrint();