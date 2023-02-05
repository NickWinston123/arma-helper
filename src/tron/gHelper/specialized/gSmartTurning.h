#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_SMARTTURNING
#define ArmageTron_GHELPER_SMARTTURNING

class gSmartTurning
{
public:
    gSmartTurning(gHelper *helper, gCycle *owner);
    void findTurnToTarget(eCoord const &target, eCoord const &velocity);
    void Activate(gHelperData &data);
    bool isClose(eCoord pos, REAL closeFactor);
    void smartTurningPlan(gHelperData &data);
    void smartTurningSurvive(gHelperData &data);
    void smartTurningOpposite(gHelperData &data);
    void followTail(gHelperData &data);
    void smartTurningFrontBot(gHelperData &data);
    void thinkSurvive(gHelperData &data);
    int thinkPath(eCoord pos, gHelperData &data);
    bool canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor = 0);
    void smartTurningAutoTrace(gHelperData &data);
    void smartTurningSurviveTrace(gHelperData &data);
    void calculateRubberFactor(REAL rubberMult, REAL rubberFactorMult);
    bool makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor = 0);
    void canSurviveTurn(gHelperData &data,
                        REAL &canSurviveLeftTurn,
                        REAL &canSurviveRightTurn,
                        bool &closedIn,
                        bool &blockedBySelf,
                        REAL freeSpaceFactor = 0);
    static gSmartTurning &Get(gHelper *helper, gCycle *owner);
    gTurnData *getEmergencyTurn(gHelperData &data);
    bool CanMakeTurn(uActionPlayer *action);

private:
    gCycle *owner_;
    gHelper *helper_;
    gHelperData *data_;
    REAL &lastTurnAttemptTime;
    REAL &lastTurnAttemptDir;
    REAL &lastTurnTime;
    REAL &turnIgnoreTime;
    REAL &lastTurnDir; // -1 left , 1 right
    REAL &blockTurn;   // 0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH
    REAL &forceTurn;   // 0 = NONE, -1 = LEFT, 1 = RIGHT
    REAL lastTailTurnTime;
    ePath path;
};
#endif