#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_SMARTTURNING
#define ArmageTron_GHELPER_SMARTTURNING

class gSmartTurning
{
public:
    // constructor to create an instance of gSmartTurning with a helper and an owner cycle
    gSmartTurning(gHelper &helper, gCycle &owner);

    // function to make the cycle survive by determining which turns are possible and blocking if required
    void smartTurningSurvive(gHelperData &data);
    void smartTurningSurvive();

    // function to make the cycle turn in response to a front bot
    void smartTurningFrontBot(gHelperData &data);

    // function to make the cycle survive by tracing the wall in the direction the last turn was blocked
    void smartTurningSurviveTrace(gHelperData &data);

    // function to make the cycle turn to automatically trace corners
    void smartTurningPlan(gHelperData &data);

    // function to make the cycle turn in the opposite direction when a direction is blocked
    void smartTurningOpposite(gHelperData &data);

    // function to make the cycle follow its tail
    void followTail(gHelperData &data);

    // Activate function to activate the smart turning feature
    void Activate(gHelperData &data);

    // function to get the instance of gSmartTurning
    static gSmartTurning &Get(gHelper &helper, gCycle &owner);

private:
    // owner cycle
    gCycle &owner_;

    // helper
    gHelper &helper_;

    // time of the last turn attempt
    REAL &lastTurnAttemptTime;

    // direction of the last turn attempt
    REAL &lastTurnAttemptDir;

    // time of the last turn
    REAL &lastTurnTime;

    // time to ignore turning
    REAL &turnIgnoreTime;

    // direction of the last turn
    REAL &lastTurnDir; // -1 left , 1 right

    // block turn value (0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH)
    REAL &blockTurn;

    // force turn value (0 = NONE, -1 = LEFT, 1 = RIGHT)
    REAL &forceTurn;

private:
    // last time turn to tail was taken
    REAL lastTailTurnTime;
};

#endif
