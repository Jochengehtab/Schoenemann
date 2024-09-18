#pragma once

#include <iostream>
#include <chrono>

#include "chess.hpp"
using namespace chess;
int uciLoop(int argc, char* argv[]);
int getTime();
int getIncrement();
void commandListener();
void processCommand(const std::string& cmd, Board& board) ;