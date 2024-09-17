#include "schoenemann.h"
#include "consts.h"

void getTimeForMove();

bool shouldStopSoft(auto s)
{
    return (std::chrono::high_resolution_clock::now() - s).count() > seracher.hardLimit;
}

bool shouldStopID(auto s)
{
    return (std::chrono::high_resolution_clock::now() - s).count() > seracher.softLimit;
}