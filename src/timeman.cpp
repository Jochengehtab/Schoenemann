#include "timeman.h"

void getTimeForMove()
{
	searcher.timeLeft -= searcher.timeLeft / 2;
	searcher.hardLimit = searcher.softLimit = searcher.timeLeft;

	int baseTime = (int) (searcher.timeLeft * 0.054 + searcher.increment * 0.85);
	int maxTime = (int) (searcher.timeLeft * 0.76);

	searcher.hardLimit = std::min(maxTime, (int) (baseTime * 3.04));
	searcher.softLimit = std::min(maxTime, (int) (baseTime * 0.76));
}
