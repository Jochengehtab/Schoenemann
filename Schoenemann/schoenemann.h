#pragma once

#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>
#include <cstring>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "chess.hpp"
using namespace chess;
int uciLoop(int argc, char* argv[]);
int getTime();
int getIncrement();
void commandListener();
void processCommand(const std::string& cmd, Board& board) ;