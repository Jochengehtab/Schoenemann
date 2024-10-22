#include "timeman.h"

void getTimeForMove()
{
	if (seracher.timeLeft < 0)
	{
		seracher.timeLeft = 1000;
	}

	seracher.timeLeft -= seracher.timeLeft / 2;
	seracher.hardLimit = seracher.softLimit = seracher.timeLeft;

	int baseTime = (int) (seracher.timeLeft * 0.054 + seracher.increment * 0.85);
	int maxTime = (int) (seracher.timeLeft * 0.76);

	seracher.hardLimit = std::min(maxTime, (int) (baseTime * 3.04));
	seracher.softLimit = std::min(maxTime, (int) (baseTime * 0.76));
}

bool shouldStopSoft(std::chrono::time_point<std::chrono::system_clock> s)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
    return elapsed.count() > seracher.hardLimit;
}

bool shouldStopID(std::chrono::time_point<std::chrono::system_clock> s)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
    return elapsed.count() > seracher.softLimit;
}