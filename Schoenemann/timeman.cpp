#include "timeman.h"

void getTimeForMove()
{
	int timeLeft = getTime();
	int increment = getIncrement();
	if (timeLeft < 0)
	{
		timeLeft = 1000;
	}

	timeLeft -= timeLeft / 2;
	seracher.hardLimit = seracher.softLimit = timeLeft;

	int baseTime = (int) (timeLeft * 0.054 + increment * 0.85);
	int maxTime = (int) (timeLeft * 0.76);

	seracher.hardLimit = std::min(maxTime, (int) (baseTime * 3.04));
	seracher.softLimit = std::min(maxTime, (int) (baseTime * 0.76));
}