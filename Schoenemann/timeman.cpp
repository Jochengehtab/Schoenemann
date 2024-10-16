#include "timeman.h"

void getTimeForMove()
{
	if (timeLeft < 0)
	{
		timeLeft = 1000;
	}

	timeLeft -= timeLeft / 2;
	searcher->hardLimit = searcher->softLimit = timeLeft;

	int baseTime = (int) (timeLeft * 0.054 + increment * 0.85);
	int maxTime = (int) (timeLeft * 0.76);

	searcher->hardLimit = std::min(maxTime, (int) (baseTime * 3.04));
	searcher->softLimit = std::min(maxTime, (int) (baseTime * 0.76));
}

bool shouldStopSoft(auto s)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
    return elapsed.count() > searcher->hardLimit;
}

bool shouldStopID(auto s)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
    return elapsed.count() > searcher->softLimit;
}