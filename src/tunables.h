#ifndef TUNABLES_H
#define TUNABLES_H

#include "tune.h"

DEFINE_PARAM(rfpSub, 100, 50, 250);
DEFINE_PARAM(nmpBase, 3, 1, 5);
DEFINE_PARAM(nmpDiv, 3, 2, 4);
DEFINE_PARAM(fpAdd, 50, 25, 100);
DEFINE_PARAM(fpMul, 100, 25, 100);
DEFINE_PARAM(seeQuiet, -20, -45, -5);
DEFINE_PARAM(seeNonQuiet, -90, -120, 50);
DEFINE_PARAM(seDepthSub, 3, 2, 4);
DEFINE_PARAM(seNewDepthSub, 1, 1, 2);
DEFINE_PARAM(aspBase, 25, 15, 35);
DEFINE_PARAM(materialBase, 160, 100, 220);
DEFINE_PARAM(materialDiv, 270, 225, 315);


// Quiet History
// Bonus Base
DEFINE_PARAM(qhBB, 30, 10, 50);

// Bonus Multiplier
DEFINE_PARAM(qhBM, 200, 120, 280);

// Bonus Cap
DEFINE_PARAM(qhBC, 1750, 1400, 2100);

// Malus Base
DEFINE_PARAM(qhMB, 15, 10, 20);

// Malus Multiplier
DEFINE_PARAM(qhMM, 170, 130, 210);

// Malus Cap
DEFINE_PARAM(qhMC, 1900, 1550, 2250);

// Continuation History
// Bonus Base
DEFINE_PARAM(chBB, 25, 15, 35);

DEFINE_PARAM(razorAlpha, 250, 220, 270);
DEFINE_PARAM(razorDepthMul, 50, 35, 65);

// Bonus Multiplier
DEFINE_PARAM(chBM, 200, 120, 280);

// Bonus Cap
DEFINE_PARAM(chBC, 2000, 1700, 2300);

// Malus Base
DEFINE_PARAM(chMB, 25, 15, 35);

// Malus Multiplier
DEFINE_PARAM(chMM, 185, 135, 225);

// Malus Cap
DEFINE_PARAM(chMC, 2150, 1750, 2550);


#endif
