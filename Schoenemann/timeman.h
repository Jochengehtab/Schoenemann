#include "schoenemann.h"
#include "consts.h"

void getTimeForMove();

bool shouldStopSoft(auto s)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
    return elapsed.count() > seracher.hardLimit;
}

bool shouldStopID(auto s)
{
    std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - s;
    return elapsed.count() > seracher.softLimit;
}