#pragma once

#include "consts.h"

void getTimeForMove();
bool shouldStopSoft(std::chrono::time_point<std::chrono::system_clock> s);
bool shouldStopID(std::chrono::time_point<std::chrono::system_clock> s);