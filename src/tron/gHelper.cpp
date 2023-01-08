// HELPER

#include "gHelper.h"

#include "nConfig.h"
#include "gSensor.h"
#include "gWall.h"
#include "gAIBase.h"
#include "eDebugLine.h"

extern REAL sg_cycleBrakeRefill;
extern REAL sg_cycleBrakeDeplete;
extern void sg_RubberValues( ePlayerNetID const * player, REAL speed, REAL & max, REAL & effectiveness );
extern REAL sg_brakeCycle;
extern REAL sg_cycleBrakeDeplete;

bool sg_helper = false;
static tConfItem<bool> sg_helperConf("HELPER", sg_helper);

REAL sg_helperBrightness = 1;
static tConfItem<REAL> sg_helperBrightnessConf("HELPER_BRIGHTESS", sg_helperBrightness);

REAL sg_helperSensorRange = 1000;
static tConfItem<REAL> sg_helperSensorRangeConf("HELPER_SENSOR_RANGE", sg_helperSensorRange);

bool sg_helperSensorLightUsagetMode = false;
static tConfItem<bool> sg_helperSensorLightUsagetModeConf("HELPER_SENSOR_LIGHT_USAGE_MODE", sg_helperSensorLightUsagetMode);

bool sg_helperDebug = false;
static tConfItem<bool> sg_helperDebugConf("HELPER_DEBUG", sg_helperDebug);

REAL sg_helperDebugDelay = 0.15;
static tConfItem<REAL> sg_helperDebugDelayConf("HELPER_DEBUG_DELAY",
                                            sg_helperDebugDelay);

bool sg_helperDebugTimeStamp = true;
static tConfItem<bool> sg_helperDebugTimeStampConf("HELPER_DEBUG_TIMESTAMP",
                                                sg_helperDebugTimeStamp);

bool sg_helperShowEnemyTail = false;
static tConfItem<bool> sg_helperShowEnemyTailConf("HELPER_SHOW_ENEMY_TAIL", sg_helperShowEnemyTail);

REAL sg_helperShowEnemyTailHeight = 1;
static tConfItem<REAL> sg_helperShowEnemyTailHeightConf("HELPER_SHOW_ENEMY_TAIL_HEIGHT", sg_helperShowEnemyTailHeight);

REAL sg_helperShowEnemyTailDistanceMult = 1;
static tConfItem<REAL> sg_helperShowEnemyTailDistanceMultConf("HELPER_SHOW_ENEMY_TAIL_DISTANCE_MULT", sg_helperShowEnemyTailDistanceMult);


REAL sg_helperShowEnemyTailTimeoutMult = 1;
static tConfItem<REAL> sg_helperShowEnemyTailTimeoutMultConf("HELPER_SHOW_ENEMY_TAIL_TIMEOUT_MULT", sg_helperShowEnemyTailTimeoutMult);

REAL sg_helperShowEnemyTailBrightness = 1;
static tConfItem<REAL> sg_helperShowEnemyTailBrightnessConf("HELPER_SHOW_ENEMY_TAIL_BRIGHTNESS", sg_helperShowEnemyTailBrightness);


REAL sg_helperEnemyTracersBrightness = 1;
static tConfItem<REAL> sg_helperEnemyTracersBrightnessConf("HELPER_SHOW_ENEMY_TRACERS_BRIGHTNESS", sg_helperEnemyTracersBrightness);

REAL sg_helperEnemyTracersHeight = 0;
static tConfItem<REAL> sg_helperEnemyTracersHeightConf("HELPER_SHOW_ENEMY_TRACERS_HEIGHT", sg_helperEnemyTracersHeight);


bool sg_helperEnemyTracers = false;
static tConfItem<bool> sg_helperEnemyTracersConf("HELPER_SHOW_ENEMY_TRACERS", sg_helperEnemyTracers);

REAL sg_helperEnemyTracersDetectionRange = 150;
static tConfItem<REAL> sg_helperEnemyTracersDetectionRangeConf("HELPER_SHOW_ENEMY_TRACERS_DETECTION_RANGE", sg_helperEnemyTracersDetectionRange);

REAL sg_helperEnemyTracersSpeedMult = 1.5;
static tConfItem<REAL> sg_helperEnemyTracersSpeedMultConf("HELPER_SHOW_ENEMY_TRACERS_SPEED_MULT", sg_helperEnemyTracersSpeedMult);

REAL sg_helperEnemyTracersPassthrough = 1;
static tConfItem<REAL> sg_helperEnemyTracersPassthroughConf("HELPER_SHOW_ENEMY_TRACERS_PASSTHROUGH", sg_helperEnemyTracersPassthrough);

REAL sg_helperEnemyTracersDelayMult = 1;
static tConfItem<REAL> sg_helperEnemyTracersDelayMultConf("HELPER_SHOW_ENEMY_TRACERS_DELAY_MULT", sg_helperEnemyTracersDelayMult);

REAL sg_helperEnemyTracersTimeout = 0.01;
static tConfItem<REAL> sg_helperEnemyTracersTimeoutConf("HELPER_SHOW_ENEMY_TRACERS_TIMEOUT", sg_helperEnemyTracersTimeout);


bool sg_helperDetectCut = false;
static tConfItem<bool> sg_helperDetectCutConf("HELPER_DETECT_CUT", sg_helperDetectCut);

REAL sg_helperDetectCutDetectionRange = 150;
static tConfItem<REAL> sg_helperDetectCutDetectionRangeConf("HELPER_DETECT_CUT_DETECTION_RANGE", sg_helperDetectCutDetectionRange);

REAL sg_helperDetectCutTimeout = .001;
static tConfItem<REAL> sg_helperDetectCutTimeoutConf("HELPER_DETECT_CUT_TIMEOUT", sg_helperDetectCutTimeout);

REAL sg_helperDetectCutHeight = 0;
static tConfItem<REAL> sg_helperDetectCutHeightConf("HELPER_DETECT_CUT_HEIGHT", sg_helperDetectCutHeight);


REAL sg_helperDetectCutReact = 1;
static tConfItem<REAL> sg_helperDetectCutReactConf("HELPER_DETECT_CUT_REACT", sg_helperDetectCutReact);


bool sg_helperShowHit = false;
static tConfItem<bool> sg_helperShowHitConf("HELPER_SHOW_HIT", sg_helperShowHit);

bool sg_helperShowHitInternalTimeout = false;
static tConfItem<bool> sg_helperShowHitInternalTimeoutConf("HELPER_SHOW_HIT_INTERNAL_TIMEOUT", sg_helperShowHitInternalTimeout);


REAL sg_showHitDataHeight = 1;
static tConfItem<REAL> sg_showHitDataHeightConf("HELPER_SHOW_HIT_HEIGHT", sg_showHitDataHeight);

REAL sg_showHitDataHeightFront = 1;
static tConfItem<REAL> sg_showHitDataHeightFrontConf("HELPER_SHOW_HIT_HEIGHT_FRONT", sg_showHitDataHeightFront);

REAL sg_showHitDataRange = 1;
static tConfItem<REAL> sg_showHitDataRangeConf("HELPER_SHOW_HIT_RANGE", sg_showHitDataRange);

REAL sg_showHitDataFreeRange = 1;
static tConfItem<REAL> sg_showHitDataFreeRangeConf("HELPER_SHOW_HIT_OPEN_RANGE", sg_showHitDataFreeRange);

int sg_showHitDataRecursion = 1;
static tConfItem<int> sg_showHitDataRecursionConf("HELPER_SHOW_HIT_RECURSION", sg_showHitDataRecursion);

REAL sg_showHitDataTimeout = 0.009;
static tConfItem<REAL> sg_showHitDataTimeoutConf("HELPER_SHOW_HIT_TIMEOUT", sg_showHitDataTimeout);


bool sg_helperShowTailTracer = false;
static tConfItem<bool> sg_helperShowTailTracerConf("HELPER_SHOW_TAIL_TRACER", sg_helperShowTailTracer);

REAL sg_helperShowTailTracerHeight = 1;
static tConfItem<REAL> sg_helperShowTailTracerHeightConf("HELPER_SHOW_TAIL_TRACER_HEIGHT", sg_helperShowTailTracerHeight);

REAL sg_helperShowTailTracerTimeoutMult = 1;
static tConfItem<REAL> sg_helperShowTailTracerTimeoutMultConf("HELPER_SHOW_TAIL_TRACER_TIMEOUT_MULT", sg_helperShowTailTracerTimeoutMult);

REAL sg_helperShowTailTracerDistanceMult = 1;
static tConfItem<REAL> sg_helperShowTailDistanceMultTracerConf("HELPER_SHOW_ENEMY_TAIL_TRACER_DISTANCE_MULT", sg_helperShowTailTracerDistanceMult);

bool sg_helperShowTailPath = false;
static tConfItem<bool> sg_helperShowTailPathConf("HELPER_SHOW_TAIL_PATH", sg_helperShowTailPath);

bool sg_helperShowTail = false;
static tConfItem<bool> sg_helperShowTailConf("HELPER_SHOW_TAIL", sg_helperShowTail);

REAL sg_helperShowTailHeight = 1;
static tConfItem<REAL> sg_helperShowTailHeightConf("HELPER_SHOW_TAIL_HEIGHT", sg_helperShowTailHeight);


REAL sg_helperShowTailPassthrough = 0.5;
static tConfItem<REAL> sg_helperShowTailPassthroughConf("HELPER_SHOW_TAIL_PASSTHROUGH", sg_helperShowTailPassthrough);

REAL sg_helperShowTailTimeout = 1;
static tConfItem<REAL> sg_helperShowTailTimeoutConf("HELPER_SHOW_TAIL_TIMEOUT", sg_helperShowTailTimeout);


bool sg_helperShowCorners = false;
static tConfItem<bool> sg_helperShowCornersConf("HELPER_SHOW_CORNERS", sg_helperShowCorners);

bool sg_helperShowCornersPassed = 0;
static tConfItem<bool> sg_helperShowCornersPassedConf("HELPER_SHOW_CORNERS_PASSED",sg_helperShowCornersPassed);

REAL sg_helperShowCornersPassedRange = 1;
static tConfItem<REAL> sg_helperShowCornersPassedRangeConf("HELPER_SHOW_CORNERS_PASSED_RANGE", sg_helperShowCornersPassedRange);

REAL sg_helperShowCornersBoundary = 10;
static tConfItem<REAL> sg_showTraceDatacornerRangeConf("HELPER_SHOW_CORNERS_BOUNDARY", sg_helperShowCornersBoundary);

REAL sg_helperShowCornersBoundaryPassed = 2.5;
static tConfItem<REAL> sg_showTraceDatacornerPassedRangeConf("HELPER_SHOW_CORNERS_BOUNDARY_PASSED", sg_helperShowCornersBoundaryPassed);

REAL sg_helperShowCornersTimeout = 1;
static tConfItem<REAL> sg_traceTimeoutConf("HELPER_SHOW_CORNERS_TIMEOUT", sg_helperShowCornersTimeout);

REAL sg_helperShowCornersHeight = 1;
static tConfItem<REAL> sg_helperShowCornersHeightConf("HELPER_SHOW_CORNERS_HEIGHT", sg_helperShowCornersHeight);

bool sg_helperSmartTurning = false;
static tConfItem<bool> sg_helperSmartTurningConf("HELPER_SMART_TURNING", sg_helperSmartTurning);


bool sg_helperAI = false;
static tConfItem<bool> sg_helperAIc("HELPER_AI", sg_helperAI);

bool sg_helperSmartTurningFrontBot = false;
static tConfItem<bool> sg_helperSmartTurningFrontBotConf("HELPER_SMART_TURNING_FRONT_BOT", sg_helperSmartTurningFrontBot);

REAL sg_helperSmartTurningFrontBotActivationRange = 1;
static tConfItem<REAL> sg_helperSmartTurningFrontBotActivationRangeConf("HELPER_SMART_TURNING_FRONT_BOT_ACTIVATION_RANGE", sg_helperSmartTurningFrontBotActivationRange);

REAL sg_helperSmartTurningFrontBotActivationRubber = 0.98;
static tConfItem<REAL> sg_helperSmartTurningFrontBotActivationRubberConf("HELPER_SMART_TURNING_FRONT_BOT_ACTIVATION_RUBBER", sg_helperSmartTurningFrontBotActivationRubber);

REAL sg_helperSmartTurningFrontBotActivationSpace = 1;
static tConfItem<REAL> sg_helperSmartTurningFrontBotActivationSpaceConf("HELPER_SMART_TURNING_FRONT_BOT_ACTIVATION_SPACE", sg_helperSmartTurningFrontBotActivationSpace);


REAL sg_helperSmartTurningFrontBotDisableTime = 0;
static tConfItem<REAL> sg_helperSmartTurningFrontBotDisableTimeConf("HELPER_SMART_TURNING_FRONT_BOT_DISABLE_TIME", sg_helperSmartTurningFrontBotDisableTime);

bool sg_helperSmartTurningClosedIn = true;
static tConfItem<bool> sg_helperSmartTurningClosedInConf("HELPER_SMART_TURNING_CLOSEDIN", sg_helperSmartTurningClosedIn);

bool sg_helperSmartTurningSurvive = false;
static tConfItem<bool> sg_helperSmartTurningSurviveConf("HELPER_SMART_TURNING_SURVIVE", sg_helperSmartTurningSurvive);

bool sg_helperSmartTurningSurviveTrace = false;
static tConfItem<bool> sg_helperSmartTurningSurviveTraceConf("HELPER_SMART_TURNING_SURVIVE_TRACE", sg_helperSmartTurningSurviveTrace);

REAL sg_helperSmartTurningSurviveTraceTurnTime = 0.02;
static tConfItem<REAL> sg_helperSmartTurningSurviveTraceTurnTimeConf("HELPER_SMART_TURNING_SURVIVE_TRACE_TURN_TIME", sg_helperSmartTurningSurviveTraceTurnTime);

REAL sg_helperSmartTurningSurviveTraceActiveTime = 1;
static tConfItem<REAL> sg_helperSmartTurningSurviveTraceActiveTimeConf("HELPER_SMART_TURNING_SURVIVE_TRACE_ACTIVE_TIME", sg_helperSmartTurningSurviveTraceActiveTime);

REAL sg_helperSmartTurningSurviveTraceCloseFactor = 1;
static tConfItem<REAL> sg_helperSmartTurningSurviveTraceCloseFactorConf("HELPER_SMART_TURNING_SURVIVE_TRACE_CLOSE_FACTOR", sg_helperSmartTurningSurviveTraceCloseFactor);

bool sg_helperSmartTurningOpposite = false;
static tConfItem<bool> sg_helperSmartTurningOppositeConf("HELPER_SMART_TURNING_OPPOSITE", sg_helperSmartTurningOpposite);

REAL sg_helperSmartTurningClosedInMult = 1;
static tConfItem<REAL> sg_helperSmartTurningClosedInMultConf("HELPER_SMART_TURNING_CLOSEDIN_MULT", sg_helperSmartTurningClosedInMult);

REAL sg_helperSmartTurningRubberTimeMult = 1;
static tConfItem<REAL> sg_helperSmartTurningRubberTimeMultConf("HELPER_SMART_TURNING_RUBBERTIME_MULT", sg_helperSmartTurningRubberTimeMult);

REAL sg_helperSmartTurningRubberFactorMult = 1;
static tConfItem<REAL> sg_helperSmartTurningRubberFactorMultConf("HELPER_SMART_TURNING_RUBBERFACTOR_MULT", sg_helperSmartTurningRubberFactorMult);


REAL sg_helperSmartTurningSpace = 0;
static tConfItem<REAL> sg_helperSmartTurningSpaceConf("HELPER_SMART_TURNING_SPACE", sg_helperSmartTurningSpace);


bool sg_helperSmartTurningPlan = false;
static tConfItem<bool> sg_helperSmartTurningPlanConf("HELPER_SMART_TURNING_PLAN", sg_helperSmartTurningPlan);

bool sg_helperSmartTurningAutoBrake = false;
static tConfItem<bool> sg_helperSmartTurningAutoBrakeConf("HELPER_SMART_TURNING_BRAKE", sg_helperSmartTurningAutoBrake);

bool sg_helperSmartTurningAutoBrakeDeplete = false;
static tConfItem<bool> sg_helperSmartTurningAutoBrakeDepleteConf("HELPER_SMART_TURNING_BRAKE_DEPLETE", sg_helperSmartTurningAutoBrakeDeplete);


REAL sg_helperSmartTurningAutoBrakeMin = 0;
static tConfItem<REAL> sg_helperSmartTurningAutoBrakeMinConf("HELPER_SMART_TURNING_BRAKE_MIN", sg_helperSmartTurningAutoBrakeMin);

REAL sg_helperSmartTurningAutoBrakeMax = 2;
static tConfItem<REAL> sg_helperSmartTurningAutoBrakeMaxConf("HELPER_SMART_TURNING_BRAKE_MAX", sg_helperSmartTurningAutoBrakeMax);

bool sg_helperSmartTurningFollowTail = false;
static tConfItem<bool> sg_helperSmartTurningFollowTailConf("HELPER_SMART_TURNING_FOLLOW_TAIL", sg_helperSmartTurningFollowTail);

REAL sg_helperSmartTurningFollowTailDelayMult = 1;
static tConfItem<REAL> sg_helperSmartTurningFollowTailDelayMultConf("HELPER_SMART_TURNING_FOLLOW_TAIL_DELAY_MULT", sg_helperSmartTurningFollowTailDelayMult);

REAL sg_helperSmartTurningFollowTailFreeSpaceMult = 1;
static tConfItem<REAL> sg_helperSmartTurningFollowTailFreeSpaceMultConf("HELPER_SMART_TURNING_FOLLOW_TAIL_FREE_SPACE_MULT", sg_helperSmartTurningFollowTailFreeSpaceMult);


class gCycleTouchEvent{
public:
    REAL dist;      // the position on this cycle's wall the touching happened
    REAL otherDist; // the position on the other cylce's wall
    int  otherSide; // the side of the other cylce this wall touches
    int  winding;   // winding number to add if we cross here

    gCycleTouchEvent()
    {
        dist      = 0;
        otherDist = 0;
        otherSide = -100;
    }
};

class gCycleMemoryEntry{
public:
    gCycleMemory *memory;
    int id;
    nObserverPtr< gCycle > cycle;

    gCycleMemoryEntry(gCycleMemory* m, const gCycle* c)
            :memory(m),id(-1), cycle(c)
    {
        memory->memory.Add(this, id);

        max[0].dist = -1E+30;
        max[1].dist = -1E+30;
        min[0].dist =  1E+30;
        min[1].dist =  1E+30;
    }

    ~gCycleMemoryEntry()
    {
        memory->memory.Remove(this, id);
    }

    // usable data
    gCycleTouchEvent max[2]; // latest touch event (with the given cylce)
    gCycleTouchEvent min[2]; // earliest touch event
};

#define TOL 4


// look for a closed loop in the walls if cycle a hits cycle b's wall at
// distance bDist and on side bSide.
// Look for the loop in driving direction of b if dir is 1 or to the other side of dir is 0.
// the end of the loop is reached when the wall of cycle a is driven along in
// direction aEndDir, passing the distance aEndDist and the side's bit is set
// in aEndSides.
// Cycles that will be closed in the loop are stored in the array
// closedIn.
// return value: the number of open points this loop contains
// (if this is >0, that usually means the space is wide open)
//  or -1 if there is no loop.
static bool CheckLoop(const gCycle *a, const gCycle *b,
                      REAL bDist, int bSide, int dir,
                      tArray<const gCycle*>& closedIn, int& winding,
                      REAL aEndDist = 0, int aEndSides = 3, int aEndDir = 1 )
{
    tASSERT(0<= bSide && 1 >= bSide);
    tASSERT(0<= dir && 1 >= dir);

    int tries = 10;       // so long until we give up
    int ends  = 0;

    bool bClosedIn    = false;

    const gCycle *run = b;      // we run along this cycle's wall
    int end           = dir;    // and move towards this end its wall wall
    int side          = bSide;  // we are on this side of the cycle
    REAL dist         = bDist;  // and are at this distance.
    winding           = 0;      // the winding number we collected

    int turn     = a->Grid()->WindingNumber();
    int halfTurn = turn >> 1;


#ifdef DEBUG
    //  con << "\n";
#endif

    while(tries-- > 0 && run &&
            !(run == a &&
              end == aEndDir &&
              aEndSides & (1 << side) &&
              (end > 0 ? dist >= aEndDist : dist <= aEndDist ) ) )
    {
#ifdef DEBUG
        //      con << "end = " << end << ", side = " << side << ", dist = " << dist
        //	  << ", winding = " << winding << "\n";
#endif
        if (end > 0)
        {

            // find the last connection
            gCycleMemoryEntry* last = run->memory.Latest(side);
            if (!last || last->max[side].dist <= dist + TOL)
            {
                // no interference. We can move directly around the cylce
                // and close it in.
#ifdef DEBUG
                //	      con << "Turning around...\n";
#endif

                winding += halfTurn *
                           ( side > 0 ? -1 : 1);

                end  = 0;
                side = 1-side;
                closedIn[closedIn.Len()] = run;
                dist = run->GetDistance();

                // detect early loop
                if (run == b)
                {
                    if (bClosedIn)
                    {
                        winding = 0;
                        return false;
                    }
                    else
                        bClosedIn = true;
                }
            }
            else
            {
#ifdef DEBUG
                //	      con << "Crossing...\n";
#endif

                // find the first connection
                gCycleMemoryEntry* first = run->memory.Earliest(side);
                if (first && first->min[side].dist >= dist + TOL)
                {
                    // we cross the connection:
                    winding += first->min[side].winding;
                    run      = first->cycle;
                    end      = (side == first->min[side].otherSide) ? 1 : 0;

                    //		  if (end == 0)
                    // we need to turn around to follow
                    winding += halfTurn * (side > 0 ? 1 : -1);

                    dist     = first->min[side].otherDist;
                    side     = first->min[side].otherSide;
                }
                else
                {
                    winding = 0;
                    return false;
                }
            }
        }
        else // dir = -1, we move towards the end
        {
            // find the first connection
            gCycleMemoryEntry* first = run->memory.Earliest(side);
            if (!first || first->min[side].dist >= dist - TOL)
            {
#ifdef DEBUG
                //	      con << "Turning around...\n";
#endif


                // no interference. We can move directly around the cylce's end.
                winding += halfTurn * ( side > 0 ? 1 : -1);

                end  = 1;
                side = 1-side;
                ends++;
                dist = -2 * TOL;
            }
            else
            {
#ifdef DEBUG
                //	      con << "Crossing...\n";
#endif


                // find the latest connection
                gCycleMemoryEntry* last = run->memory.Latest(side);
                if (last && last->max[side].dist <= dist - TOL)
                {
                    // we cross the connection:
                    winding += last->max[side].winding;

                    // we need to turn around to start:
                    winding += halfTurn * (side > 0 ? -1 : 1);

                    run      = last->cycle;
                    end      = (side == last->max[side].otherSide) ? 0 : 1;

                    //		  if (end == 1)
                    // we need to turn around to follow
                    //		  winding -= halfTurn * (side > 0 ? 1 : -1);

                    dist     = last->max[side].otherDist;
                    side     = last->max[side].otherSide;
                }
                else
                    // uh oh. we are already closed in. No chance...
                {
                    winding = 0;
                    return false;
                }
            }
        }
    }

#ifdef DEBUG
    //      con << "end = " << end << ", side = " << side << ", dist = " << dist
    //	  << ", winding = " << winding << "\n\n";
#endif

    if (tries >= 0)
    {
        return true;
    }
    else
    {
        winding = 0;
        return false;
    }
}


// see if the given Cycle is trapped currently
static bool IsTrapped(const gCycle *trapped, const gCycle *other)
{
    tArray<const gCycle*> closedIn;
    int winding = 0;
    if (CheckLoop(trapped, trapped, trapped->GetDistance(), 1, 0, closedIn, winding, trapped->GetDistance() - 1))
    {
        if (winding + 2 < 0)
        {
            // see if the other cylce is trapped with him
            for (int i = closedIn.Len()-1; i>=0; i--)
                if (other == closedIn(i))
                    return false;  // we can get him!

            // no. trapped is trapped allone.
            return true;
        }
    }

    return false;
}

bool sg_pathHelper = false;
static tConfItem<bool> sg_pathHelperC("HELPER_PATH", sg_pathHelper);

bool sg_pathHelperTurn = false;
static tConfItem<bool> sg_pathHelperTurnC("HELPER_PATH_RENDER", sg_pathHelperTurn);

bool sg_pathHelperShowTurn = false;
static tConfItem<bool> sg_pathHelperShowTurnC("HELPER_PATH_TURN_SHOW", sg_pathHelperShowTurn);

REAL sg_pathHelperUpdateTime = -1000;
static tConfItem<REAL> sg_pathHelperUpdateTimeC("HELPER_PATH_UPDATE_TIME", sg_pathHelperUpdateTime);

REAL sg_pathHelperThinkAgain = 1;
static tConfItem<REAL> sg_pathHelperThinkAgainC("HELPER_PATH_THINK_AGAIN", sg_pathHelperThinkAgain);

int sg_pathHelperMode = 0;
static tConfItem<int> sg_pathHelperModeC("HELPER_PATH_MODE", sg_pathHelperMode);
//sg_pathHelperModeC->help = tOutput("help");

REAL sg_pathHelperAutoRange = 150;
static tConfItem<REAL> sg_pathHelperAutoRangeC("HELPER_PATH_AUTO_RANGE", sg_pathHelperAutoRange);


gPathHelper::gPathHelper(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner),
      lastPath(owner_->localCurrentTime - 100),
      lastTime(owner_->localCurrentTime),
      nextTime(0),
      pathInvalid(true)
{
    // Initialize any other member variables here
}

bool gPathHelper::targetExist() {
    return target != eCoord(0,0) && targetCurrentFace_;
}

gPathHelper & gPathHelper::Get(gHelper * helper, gCycle *owner)
{
    tASSERT( owner );

    // create
    if ( helper->pathHelper.get() == 0 )
        helper->pathHelper.reset( new gPathHelper( helper, owner ) );

    return *helper->pathHelper;
}

bool gPathHelper::tailMode(gHelperData data)
{
    if ( owner_->tailMoving) {

        target = owner_->tailPos;
        targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(owner_->tailPos);
        return true;
    }
    return false;
}

bool gPathHelper::enemyMode(gHelperData data)
{
    gCycle *enemy = helper_->enemies.closestEnemy;

    if (helper_->enemies.exist(enemy)) {
        target = enemy->Position();
        targetCurrentFace_ = enemy->CurrentFace();
        return true;
    }
    return false;

}

bool gPathHelper::autoMode(gHelperData data)
{
    gCycle *enemy = helper_->enemies.closestEnemy;

    bool isClose = helper_->enemies.exist(enemy) && helper_->smartTurning->isClose(enemy->Position(), sg_pathHelperAutoRange + data.turnSpeedFactor);
    if (isClose) {
        target = enemy->Position();
        targetCurrentFace_ = enemy->CurrentFace();
        return true;
    }
    else if (owner_->tailMoving) {
        target = owner_->tailPos;
        targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(target);
        return true;
    }

    return false;
}

bool gPathHelper::UpdatePath()
{
    bool updateTime = lastPath < owner_->localCurrentTime - sg_pathHelperUpdateTime;
    return updateTime;
}

#include "rRender.h"

REAL se_pathHeight = 1;
static tConfItem<REAL> se_pathHeightC("HELPER_PATH_HEIGHT", se_pathHeight);

void gPathHelper::RenderPath(gHelperData & data)
{
    if (!path_.Valid())
    {
        return;
    }

   // glDisable(GL_TEXTURE_2D);
   // glDisable(GL_LIGHTING);

    glColor4f(1,0,0,1);

    BeginLineStrip();
    for (int i = path_.positions.Len()-1; i>=0; i--)
    {
        eCoord c = path_.positions(i) + path_.offsets(i);
        Vertex(c.x, c.y, se_pathHeight);
        helper_->debugLine(.2,1,0,se_pathHeight,data.speedFactor,c,c,1);
    }
    RenderEnd();

    glColor4f(1,1,0,1);

    BeginLineStrip();
    if (path_.current >= 0 && path_.positions.Len() > 0)
    {
        eCoord c = path_.CurrentPosition();
        Vertex(c.x, c.y, 0);
        Vertex(c.x, c.y, se_pathHeight * 2);

    }
    RenderEnd();
}

void gPathHelper::RenderTurn(gHelperData &data)
{
    if (!path_.Valid())
    {
        return;
    }

    eCoord targetPos = path_.CurrentPosition() + path_.CurrentOffset();// * 0.1f;
    eCoord currentPos = owner_->Position();
    eCoord dirToTarget = targetPos - currentPos;

    if (eCoord::F(owner_->Direction(), dirToTarget) > 0)
    {
        helper_->smartTurning->makeTurnIfPossible(data,1,1);
        helper_->debugLine(.2, 1, 0, 3, data.speedFactor, owner_->Position(), data.sensors.getSensor(1)->before_hit, 1);
    }
    else
    {
        helper_->smartTurning->makeTurnIfPossible(data,-1,1);
        helper_->debugLine(.2, 1, 0, 3, data.speedFactor, owner_->Position(), data.sensors.getSensor(-1)->before_hit, 1);
    }
}

void gPathHelper::ThinkPath( gHelperData & data )
{
    if (!targetExist()) {
        return;
    }
    // find a new path if the one we got is outdated:

    if (targetCurrentFace_)
    {
        //owner_->FindCurrentFace();
        eHalfEdge::FindPath(owner_->Position(), owner_->CurrentFace(),
                            target, targetCurrentFace_,
                            owner_,
                            path_);
        //con << "Found updated path & " << lastPath << "\n";
        lastPath = owner_->localCurrentTime;
    }

    if (!path_.Valid())
    {
        targetCurrentFace_ = NULL;
        lastPath = -100;
        return;
    }
    /*REAL d = (abs(target->Position()->x - Object()->Position().->) + abs(target->Position().Y - Object()->Position().Y)) * .2f;
    if (d < mindist)
        mindist = d;

    data.thinkAgain = mindist / Object()->Speed();
    if (data.thinkAgain > .4)
        data.thinkAgain *= .7;
*/
}


void gPathHelper::Activate(gHelperData &orig_data) {

    if (sg_pathHelperTurn)
    RenderPath(orig_data);
    if (!helper_->aliveCheck() || !UpdatePath())
        return;

    {
        gHelperData data = orig_data;

        bool success = false;
        switch (sg_pathHelperMode)
        {
            case 0:
                success = autoMode(data);
                break;
            case 1:
                success = tailMode(data);
                break;
            case 2:
                success = enemyMode(data);
                break;
            default:
                // do nothing
                return;
                break;
        }
        if (!success)
            return;

        ThinkPath(data);
        //RenderPath();
        //con << "Got Turn: " << data.turnDir << " Next Thought in " << nextthought << "\n";

        //if (sg_pathHelperTurn)
        //    helper_->smartTurning->makeTurnIfPossible(data,data.turnDir,1);

        if (sg_pathHelperShowTurn)
            RenderTurn(data);
            //helper_->debugLine(.2,1,0,3,data.speedFactor,owner_->Position(),data.sensors.getSensor(data.turnDir)->before_hit,1);
    }
}



// void gPathHelper::ThinkPath( gHelperData & data )
// {
//     if (!targetExist()) {
//         data.thinkAgain = 4;
//         return;
//     }

//     int lr = 0;
//     REAL mindist = 10;

//     eCoord dir = owner_->Direction();
//     REAL ls    = data.sensors.getSensor(-1)->hit;
//     REAL rs    = data.sensors.getSensor(1)->hit;

//     //con << "TARGET: " << target << "\n";
//     eCoord tDir = target - owner_->Position();

//     // find a new path if the one we got is outdated:
//     if (lastPath < owner_->localCurrentTime - sg_pathHelperUpdateTime)

//         if (targetCurrentFace_)
//         {
//             //owner_->FindCurrentFace();
//             eHalfEdge::FindPath(owner_->Position(), owner_->CurrentFace(),
//                                 target, targetCurrentFace_,
//                                 owner_,
//                                 path_);
//             //con << "Found updated path & " << lastPath << "\n";
//             lastPath = owner_->localCurrentTime;
//         }

//     if (!path_.Valid())
//     {
//         data.thinkAgain = 1;
//         return;
//     }

//     // find the most advanced path point that is in our viewing range:

//     if (!sg_pathHelperShowTurn && !sg_pathHelperTurn) {
//         return;
//     }

//     for (int z = 10; z>=0; z--)
//         path_.Proceed();

//     bool goon   = path_.Proceed();
//     bool nogood = false;

//     do
//     {
//         if (goon)
//             goon = path_.GoBack();
//         else
//             goon = true;

//         eCoord pos   = path_.CurrentPosition() + path_.CurrentOffset() * 0.1f;
//         eCoord opos  = owner_->Position();
//         eCoord odir  = pos - opos;

//         eCoord intermediate = opos + dir * eCoord::F(odir, dir);

//         gSensor p(owner_, opos, intermediate - opos);
//         p.detect(1.1f);
//         nogood = (p.hit <= .999999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);

//         if (!nogood)
//         {
//             gSensor p(owner_, intermediate, pos - intermediate);
//             p.detect(1);
//             nogood = (p.hit <= .99999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);
//         }

//     }
//     while (goon && nogood);

//     if (goon)
//     {
//         // now we have found our next goal. Try to get there.
//         eCoord pos            = owner_->Position();
//         eCoord current_target = path_.CurrentPosition();

//         // look how far ahead the current_target is:
//         REAL ahead = eCoord::F(current_target - pos, dir)
//                    + eCoord::F(path_.CurrentOffset(), dir);

//         if ( ahead > 0)
//         {	  // it is still before us. just wait a while.
//             mindist = ahead;
//         }
//         else
//         { // we have passed it. Make a turn towards it.
//             REAL side = (current_target - pos) * dir;

//             if ( !((side > 0 && ls < 3) || (side < 0 && rs < 3))
//                     && (fabs(side) > 3 || ahead < -10) )
//             {
//                 lr += (side > 0 ? 1 : -1);
//                 lr = -lr;
//             }
//         }
//     }
//     else // nogood
//     {
//         lastPath -= 1;
//     }

//     REAL d = sqrt(tDir.NormSquared()) * .2f;
//     if (d < mindist)
//         mindist = d;

//     data.thinkAgain = mindist / owner_->Speed();
//     if (data.thinkAgain > .4)
//         data.thinkAgain *= .7;
//     data.thinkAgain;
//     data.turnDir = lr;
// }




int noTurns = 0;
class gHelperEmergencyTurn
{
    gHelperEmergencyTurn();
public:
class Sensor: public gSensor
    {
    public:
        Sensor(gCycle *o,const eCoord &start,const eCoord &d)
                : gSensor(o,start,d)
                , hitOwner_( 0 )
                , hitTime_ ( 0 )
                , hitDistance_( o->MaxWallsLength() )
                , lrSuggestion_( 0 )
                , windingNumber_( 0 )
        {
            if ( hitDistance_ <= 0 )
                hitDistance_ = o->GetDistance();
        }

        /*
        // do detection and additional stuff
        void detect( REAL range )
        {
            gSensor::detect( range );
        }
        */

        virtual void PassEdge(const eWall *ww,REAL time,REAL a,int r)
        {
            try{
                gSensor::PassEdge(ww,time,a,r);
            }
            catch( eSensorFinished & e )
            {
                if ( DoExtraDetectionStuff() )
                    throw;
            }
        }

        bool DoExtraDetectionStuff()
        {
            // move towards the beginning of a wall
            lrSuggestion_ = -lr;

            switch ( type )
            {
            case gSENSOR_NONE:
            case gSENSOR_RIM:
                lrSuggestion_ = 0;
                return true;
            default:
                // unless it is an enemy, follow his wall instead (uncomment for a nasty cowardy campbot)
                // lrSuggestion *= -1;
            case gSENSOR_SELF:
                {
                    // determine whether we're hitting the front or back half of his wall
                    if ( !ehit )
                        return true;
                    eWall * wall = ehit->GetWall();
                    if ( !wall )
                        return true;
                    gPlayerWall * playerWall = dynamic_cast< gPlayerWall * >( wall );
                    if ( !playerWall )
                        return true;
                    hitOwner_ = playerWall->Cycle();
                    if ( !hitOwner_ )
                        return true;

                    // gCycleChatBot & enemyChatBot = Get( hitOwner_ );

                    REAL wallAlpha = playerWall->Edge()->Ratio( before_hit );
                    // that's an unreliable source
                    if ( wallAlpha < 0 )
                        wallAlpha = 0;
                    if ( wallAlpha > 1 )
                        wallAlpha = 1;
                    hitDistance_   = hitOwner_->GetDistance() - playerWall->Pos( wallAlpha );
                    hitTime_       = playerWall->Time( wallAlpha );
                    windingNumber_ = playerWall->WindingNumber();

                    // don't see new walls
                    if ( hitTime_ > hitOwner_->LastTime() && hitOwner_ != owned )
                    {
                        ehit = NULL;
                        hit = 1E+40;
                        return false;
                    }

                    // REAL cycleDistance = hitOwner_->GetDistance();

                    // REAL wallStart = 0;

                    /*
                    if ( gCycle::WallsLength() > 0 )
                    {
                        wallStart = cyclePos - playerWall->Cycle()->ThisWallsLength();
                        if ( wallStart < 0 )
                            wallStart = 0;
                    }
                    */
                }
            }

            return true;
        }

        // check how far the hit wall extends straight into the given direction
        REAL HitWallExtends( eCoord const & dir, eCoord const & origin )
        {
            if ( !ehit || !ehit->Other() )
            {
                return 1E+30;
            }

            REAL ret = -1E+30;
            eCoord ends[2] = { *ehit->Point(), *ehit->Other()->Point() };
            for ( int i = 1; i>=0; --i )
            {
                REAL newRet = eCoord::F( dir, ends[i]-origin );
                if ( newRet > ret )
                    ret = newRet;
            }

            return ret;
        }

        gCycle * hitOwner_;     // the owner of the hit wall
        REAL     hitTime_;      // the time the hit wall was built at
        REAL     hitDistance_;  // the distance of the wall to the cycle that built it
        short    lrSuggestion_; // sensor's oppinon on whether moving to the left or right of the hit wall is recommended (-1 for left, +1 for right)
        int      windingNumber_; // the number of turns (with sign) the cycle has taken
    };

    gHelperEmergencyTurn( gCycle * owner, gHelper *helper ) :
            owner_ ( owner ),
            helper_( helper )
    {
    }


    // determines the distance between two sensors; the size should give the likelyhood
    // to survive if you pass through a gap between the two selected walls
    REAL Distance( Sensor const & a, Sensor const & b )
    {
        // make sure a is left from b
        if ( a.Direction() * b.Direction() < 0 )
            return Distance( b, a );

        bool self = a.type == gSENSOR_SELF || b.type == gSENSOR_SELF;
        bool rim  = a.type == gSENSOR_RIM || b.type == gSENSOR_RIM;

        // avoid. own. walls.
        REAL selfHatred = 1;
        if ( a.type == gSENSOR_SELF )
        {
            selfHatred *= .5;
            if ( a.lr > 0 )
            {
                selfHatred *= .5;
                if ( b.type == gSENSOR_RIM )
                    selfHatred *= .25;
            }
        }
        if ( b.type == gSENSOR_SELF )
        {
            selfHatred *= .5;
            if ( b.lr < 0 )
            {
                selfHatred *= .5;
                if ( a.type == gSENSOR_RIM )
                    selfHatred *= .25;
            }
        }

        // some big distance to return if we don't know anything better
        REAL bigDistance = owner_->MaxWallsLength();
        if ( bigDistance <= 0 )
            bigDistance = owner_->GetDistance();

        if ( a.hitOwner_ != b.hitOwner_ )
        {
            // different owners? Great, there has to be a way through!
            REAL ret =
                a.hitDistance_ + b.hitDistance_;

            if ( rim )
            {
                ret = bigDistance * .001 + ret * .01 + ( a.before_hit - b.before_hit).Norm();

                // we love going between the rim and enemies
                if ( !self )
                    ret = bigDistance * 2;
            }

            // minimal factor should be 1, this path should never return something smaller than the
            // paths where only one cycle's walls are hit
            ret *= 16;

            // or empty space
            if ( a.type == gSENSOR_NONE || b.type == gSENSOR_NONE )
                ret *= 2;

            return ret * selfHatred;
        }
        else if ( rim )
        {
            // at least one rim wall? Take the distance between the hit positions.
            return ( a.before_hit - b.before_hit).Norm() * selfHatred;
        }
        else if ( a.type == gSENSOR_NONE && b.type == gSENSOR_NONE )
        {
            // empty space! Woo!
            return owner_->GetDistance() * 256;
        }
        else if ( a.lr != b.lr )
        {
            // different directions? Also great!
            return ( fabsf( a.hitDistance_ - b.hitDistance_ ) + .25 * bigDistance ) * selfHatred;
        }

        else if ( ( - 2 * a.lr * (a.windingNumber_ - b.windingNumber_ ) > owner_->Grid()->WindingNumber() ))
        {
            // this looks like a way out to me
            return fabsf( a.hitDistance_ - b.hitDistance_ ) * 10 * selfHatred;
        }
        else
        {
            // well, the longer the wall segment between the two points, the better.
            return fabsf( a.hitDistance_ - b.hitDistance_ ) * selfHatred;
        }

        // default: hit distance
        return ( a.before_hit - b.before_hit).Norm() * selfHatred;
    }

    int ActToTurn( uActionPlayer * action ) {
        return action == &gCycle::se_turnLeft ? -1 : 1;
    }

    template <class T>
    void BotDebug(const char *description, T value = "")  {
        HelperDebug("gHelperEmergencyTurn",description,value);
    }
    void BotDebug(const char *description)  {
        HelperDebug("gHelperEmergencyTurn",description,"");
    }

    template <class T>
    void HelperDebug(const char *from, const char *description, T value, bool spamProtection = true)
    {
        if (!sg_helperDebug)
        {
            return;
        }


        tString debugMessage;
        if (sg_helperDebugTimeStamp)
        {
            debugMessage << "(" << owner_->localCurrentTime << ") ";
        }

        debugMessage << "0xff8888HELPER DEBUG - "
                     << "0xff5555" << from << "0xffff88: " << description << " ";

        debugMessage << value << "\n ";
        con << debugMessage;
    }
    // does the main thinking
    int getTurn()
    {
        int turn = 0;

        REAL lookahead = 100;  // seconds to plan ahead
        REAL minstep   = 0; // minimum timestep between thoughts in seconds

        // cylce data
        REAL speed = owner_->Speed();
        eCoord dir = owner_->Direction();
        eCoord pos = owner_->Position();

        REAL range= speed * lookahead;
        eCoord scanDir = dir * range;

        REAL frontFactor = .5;

        Sensor front(owner_,pos,scanDir);
        front.detect(frontFactor);

        if ( !front.ehit )
        {
            return turn;
        }

        //owner_->enemyInfluence.AddSensor( front, 0, owner_ );

        REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

        // get extra time we get through rubber usage
        REAL rubberGranted, rubberEffectiveness;
        sg_RubberValues( owner_->Player(), speed, rubberGranted, rubberEffectiveness );
        REAL rubberTime = ( rubberGranted - owner_->GetRubber() )*rubberEffectiveness/speed;
        REAL rubberRatio = owner_->GetRubber()/rubberGranted;

        //turnedRecently_ = false;

        // these checks can hit our last wall and fail. Temporarily set it to NULL.
        tJUST_CONTROLLED_PTR< gNetPlayerWall > lastWall = owner_->getLastWall();
        owner_->setLastWall(NULL);

        REAL narrowFront = 1;

        // cast four diagonal rays
        Sensor forwardLeft ( owner_, pos, scanDir.Turn(+1,+1 ) );
        forwardLeft.detect(1);

        Sensor backwardLeft( owner_, pos, scanDir.Turn(-1,+narrowFront) );
        backwardLeft.detect(1);

        Sensor forwardRight ( owner_, pos, scanDir.Turn(+1,-1 ) );
        forwardRight.detect(1);

        Sensor backwardRight( owner_, pos, scanDir.Turn(-1,-narrowFront) );
        backwardRight.detect(1);

        // determine survival chances in the four directions
        REAL frontOpen = Distance ( forwardLeft, forwardRight );
        REAL leftOpen  = Distance ( forwardLeft, backwardLeft );
        REAL rightOpen = Distance ( forwardRight, backwardRight );
        REAL rearOpen = Distance ( backwardLeft, backwardRight );

        Sensor self( owner_, pos, scanDir.Turn(-1, 0) );

        // fake entries
        self.before_hit = pos;
        self.windingNumber_ = owner_->getWindingNumber();
        self.type = gSENSOR_SELF;
        self.hitDistance_ = 0;
        self.hitOwner_ = owner_;
        self.hitTime_ = owner_->LastTime();//localCurrentTime;
        self.lr = -1;
        REAL rearLeftOpen = Distance( backwardLeft, self );
        self.lr = 1;
        REAL rearRightOpen = Distance( backwardRight, self );

        {
            // override: don't camp (too much)
            if ( forwardRight.type == gSENSOR_SELF &&
                    forwardLeft.type == gSENSOR_SELF &&
                    backwardRight.type == gSENSOR_SELF &&
                    backwardLeft.type == gSENSOR_SELF &&
                    front.type == gSENSOR_SELF &&
                    forwardRight.lr == front.lr &&
                    forwardLeft.lr == front.lr &&
                    backwardRight.lr == front.lr &&
                    backwardLeft.lr == front.lr &&
                    frontOpen + leftOpen + rightOpen < owner_->GetDistance() * .5 )
            {
                //turnedRecently_ = true;
                if ( front.lr > 0 )
                {
                    if ( leftOpen > minstep * speed )
                        // force a turn to the left
                        rightOpen = 0;
                    else if ( front.hit * range < 2 * minstep )
                        // force a preliminary turn to the right that will allow us to reverse
                        frontOpen = 0;
                }
                else
                {
                    if ( rightOpen > minstep * speed )
                        // force a turn to the right
                        leftOpen = 0;
                    else if ( front.hit * range < 2 * minstep )
                        // force a preliminary turn to the left that will allow us to reverse
                        frontOpen = 0;
                }
            }
        }

        // override rim hugging
        if ( forwardRight.type == gSENSOR_SELF &&
                forwardLeft.type == gSENSOR_RIM &&
                backwardRight.type == gSENSOR_SELF &&
                backwardLeft.type == gSENSOR_RIM &&
                // backwardLeft.hit < .1 &&
                forwardRight.lr == -1 &&
                backwardRight.lr == -1 )
        {
            //turnedRecently_ = true;
            if ( rightOpen > speed * ( owner_->GetTurnDelay() - rubberTime * .8 ) )
            {
                turn = 1;
                BotDebug("right by override rim hugging 1");
                return turn;

                //owner_->Act( &gCycle::se_turnRight, 1 );
                //owner_->Act( &gCycle::se_turnRight, 1 );
            }
            else if ( leftOpen > speed * ( owner_->GetTurnDelay() - rubberTime * .8 ) )
            {
                turn = -1;
                BotDebug("left by override rim hugging 1");
                return turn;
                //owner_->Act( &gCycle::se_turnLeft, 1 );
                //owner_->Act( &gCycle::se_turnLeft, 1 );
            }
        }

        if ( forwardLeft.type == gSENSOR_SELF &&
                forwardRight.type == gSENSOR_RIM &&
                backwardLeft.type == gSENSOR_SELF &&
                backwardRight.type == gSENSOR_RIM &&
                // backwardRight.hit < .1 &&
                forwardLeft.lr == 1 &&
                backwardLeft.lr == 1 )
        {
            //turnedRecently_ = true;
            if ( leftOpen > speed * ( owner_->GetTurnDelay() - rubberTime * .8 ) )
            {
                turn = -1;
                BotDebug("right by override rim hugging 2");
                return turn;
                //owner_->Act( &gCycle::se_turnLeft, 1 );
                //owner_->Act( &gCycle::se_turnLeft, 1 );
            }
            else if ( rightOpen > speed * ( owner_->GetTurnDelay() - rubberTime * .8 ) )
            {
                turn = 1;
                BotDebug("left by override rim hugging 2");
                return turn;

            }
        }

        // get the best turn direction
        uActionPlayer * bestAction = ( leftOpen > rightOpen ) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
        int             bestDir      = ( leftOpen > rightOpen ) ? 1 : -1;
        REAL            bestOpen     = ( leftOpen > rightOpen ) ? leftOpen : rightOpen;
        Sensor &        bestForward  = ( leftOpen > rightOpen ) ? forwardLeft : forwardRight;
        Sensor &        bestBackward = ( leftOpen > rightOpen ) ? backwardLeft : backwardRight;

        Sensor direct ( owner_, pos, scanDir.Turn( 0, bestDir) );
        direct.detect( 1 );

        // restore last wall
        owner_->setLastWall(lastWall);

        // only turn if the hole has a shape that allows better entry after we do a zig-zag, or if we're past the good turning point
        // see how the survival chance is distributed between forward and backward half
        REAL forwardHalf  = Distance ( direct, bestForward );
        REAL backwardHalf = Distance ( direct, bestBackward );

        REAL forwardOverhang  = bestForward.HitWallExtends( bestForward.Direction(), pos );
        REAL backwardOverhang  = bestBackward.HitWallExtends( bestForward.Direction(), pos );

        // we have to move forward this much before we can hope to turn
        minMoveOn = bestBackward.HitWallExtends( dir, pos );

        // maybe the direct to the side sensor is better?
        REAL minMoveOnOther = direct.HitWallExtends( dir, pos );

        // determine how far we can drive on
        maxMoveOn      = bestForward.HitWallExtends( dir, pos );
        REAL maxMoveOnOther = front.HitWallExtends( dir, pos );
        if ( maxMoveOn > maxMoveOnOther )
            maxMoveOn = maxMoveOnOther;

        if ( maxMoveOn > minMoveOnOther && forwardHalf > backwardHalf && direct.hitOwner_ == bestBackward.hitOwner_ )
        {
            backwardOverhang  = direct.HitWallExtends( bestForward.Direction(), pos );
            minMoveOn = minMoveOnOther;
        }

        // best place to turn
        moveOn = .5 * ( minMoveOn * ( 1 + rubberRatio ) + maxMoveOn * ( 1 - rubberRatio ) );

        // hit the brakes before you hit anything and if it's worth it
        bool brake = sg_brakeCycle > 0 &&
                        front.hit * lookahead * sg_cycleBrakeDeplete < owner_->GetBrakingReservoir() &&
                        sg_brakeCycle * front.hit * lookahead < 2 * speed * owner_->GetBrakingReservoir() &&
                        ( maxMoveOn - minMoveOn ) > 0 &&
                        owner_->GetBrakingReservoir() * ( maxMoveOn - minMoveOn ) < speed * owner_->GetTurnDelay();

        if ( frontOpen < bestOpen &&
                ( forwardOverhang <= backwardOverhang || ( minMoveOn < 0 && moveOn < minstep * speed ) ) )
        {
            // REAL expectedBackwardHalf = ( direct.before_hit - bestBackward.before_hit ).Norm();

            // if ( ( ( forwardHalf + backwardHalf > bestOpen * 2 || backwardHalf > frontOpen * 10 || backwardHalf > expectedBackwardHalf * 1.01 ) && frontOpen < bestOpen ) ||
            // rubberTime * .5 + minspace * lookahead < minstep )
            //                {
            //turnedRecently_ = true;

            minMoveOn = maxMoveOn = moveOn = 0;

          /*  if (
                (((bestBackward.type == gSENSOR_ENEMY || bestBackward.type == gSENSOR_TEAMMATE) &&
                   bestBackward.hitDistance_ < bestBackward.hit * lookahead * speed) ||
                   direct.hit * lookahead + rubberTime < owner_->GetTurnDelay()) &&
                  (bestBackward.hit * lookahead + rubberTime < owner_->GetTurnDelay() ||
                   bestForward.hit * lookahead + rubberTime < owner_->GetTurnDelay())) {
                // override: stupid turn into certain death, turn it around if that makes it less stupid
                uActionPlayer *newBestAction = (leftOpen > rightOpen) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
                Sensor newDirect(owner_, pos, scanDir.Turn(0, -bestDir));
                newDirect.detect(1);
                if (newDirect.hit > direct.hit ||
                    newDirect.hit * lookahead + rubberTime > owner_->GetTurnDelay())
                {
                    turn = ActToTurn(newBestAction);
                    BotDebug("frontOpen < bestOpen override stupid turn ", turn);
                    return turn;
                }
            }
            else */{

                turn = ActToTurn(bestAction);
                BotDebug("frontOpen < bestOpen bestAction ", turn);
                return turn;
            }

            brake = false;
        }
        else
        {
            // the best
            REAL bestSoFar = frontOpen > bestOpen ? frontOpen : bestOpen;
            bestSoFar *= ( 10 * ( 1 - rubberRatio ) + 1 );

            if ( rearOpen > bestSoFar && ( rearLeftOpen > bestSoFar || rearRightOpen > bestSoFar ) )
            {
                brake = false;
                //turnedRecently_ = true;

                bool goLeft = rearLeftOpen > rearRightOpen;

                // dead end. reverse into the opposite direction of the front wall
                uActionPlayer * bestAction = goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
                uActionPlayer * otherAction = !goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
                Sensor &        bestForward  = goLeft ? forwardLeft : forwardRight;
                Sensor &        bestBackward  = goLeft ? backwardLeft : backwardRight;
                Sensor &        otherForward  = !goLeft ? forwardLeft : forwardRight;
                Sensor &        otherBackward  = !goLeft ? backwardLeft : backwardRight;

                // space in the two directions available for turns
                REAL bestHit = bestForward.hit > bestBackward.hit ? bestBackward.hit : bestForward.hit;
                REAL otherHit = otherForward.hit > otherBackward.hit ? otherBackward.hit : otherForward.hit;

                bool wait = false;

                // well, after a short turn to the right if space is tight
                if ( bestHit * lookahead < owner_->GetTurnDelay() + rubberTime )
                {
                    if ( otherHit < bestForward.hit * 2 && front.hit * lookahead > owner_->GetTurnDelay() * 2 )
                    {
                        // wait a bit, perhaps there will be a better spot
                        wait = true;
                        BotDebug("WAITING");
                    }
                    else if (!wait)
                    {
                        {

                        turn = ActToTurn(otherAction);
                        BotDebug("rearOpen > bestSoFar ootherAction ",turn);
                        return turn;
                        }
                        // // there needs to be space ahead to finish the maneuver correctly
                        // if ( maxMoveOn < speed * owner_->GetTurnDelay() )
                        // {
                        //     // there isn't. oh well, turn into the wrong direction completely, see if I care
                        //     owner_->Act( otherAction, 1 );
                        //     wait = true;
                        // }
                    }
                }

                if ( !wait )
                {
                    turn = ActToTurn(bestAction);
                    BotDebug("not waiting bestAction", turn);
                    return turn;
                }

                minMoveOn = maxMoveOn = moveOn = 0;
            }
        }

        // execute brake command
        owner_->Act( &gCycle::s_brake, brake ? 1 : -1 );

        noTurns++;
        BotDebug("NO TURN FOUND", noTurns);
        return turn;





    }

private:
    gCycle * owner_;         //!< owner of chatbot
    gHelper* helper_;
};


 class SensorPub : public gSensor
    {
    public:
        SensorPub(gCycle *o, const eCoord &start, const eCoord &d)
            : gSensor(o, start, d), hitOwner_(0), hitTime_(0),
              hitDistance_(o->MaxWallsLength()), lrSuggestion_(0),
              windingNumber_(0), owner_(o)
        {
            if (hitDistance_ <= 0)
                hitDistance_ = o->GetDistance();
        }

        virtual void PassEdge(const eWall *ww, REAL time, REAL a, int r)
        {
            try
            {
                gSensor::PassEdge(ww, time, a, r);
            }
            catch (eSensorFinished &e)
            {
                if (DoExtraDetectionStuff())
                    throw;
            }
        }

        bool DoExtraDetectionStuff()
        {
            // move towards the beginning of a wall
            lrSuggestion_ = -lr;

            switch (type)
            {
            case gSENSOR_NONE:
            case gSENSOR_RIM:
                lrSuggestion_ = 0;
                return true;
            default:
                // unless it is an enemy, follow his wall instead (uncomment for a nasty
                // cowardy campbot) lrSuggestion *= -1;
            case gSENSOR_SELF:
            {
                // determine whether we're hitting the front or back half of his wall
                if (!ehit)
                    return true;
                eWall *wall = ehit->GetWall();
                if (!wall)
                    return true;
                gPlayerWall *playerWall = dynamic_cast<gPlayerWall *>(wall);
                if (!playerWall)
                    return true;
                hitOwner_ = playerWall->Cycle();
                if (!hitOwner_)
                    return true;

                REAL wallAlpha = playerWall->Edge()->Ratio(before_hit);
                // that's an unreliable source
                if (wallAlpha < 0)
                    wallAlpha = 0;
                if (wallAlpha > 1)
                    wallAlpha = 1;
                hitDistance_ = hitOwner_->GetDistance() - playerWall->Pos(wallAlpha);
                hitTime_ = playerWall->Time(wallAlpha);
                windingNumber_ = playerWall->WindingNumber();

                // don't see new walls
                if (hitTime_ > hitOwner_->LastTime()  &&
                    hitOwner_ != owned)
                {
                    ehit = NULL;
                    hit = 1E+40;
                    return false;
                }
            }
            }

            return true;
        }
    // determines the distance between two sensors; the size should give the
    // likelyhood to survive if you pass through a gap between the two selected
    // walls
    REAL Distance(SensorPub const &a, SensorPub const &b)
    {
        // make sure a is left from b
        if (a.Direction() * b.Direction() < 0)
            return Distance(b, a);

        bool self = a.type == gSENSOR_SELF || b.type == gSENSOR_SELF;
        bool rim = a.type == gSENSOR_RIM || b.type == gSENSOR_RIM;

        // avoid. own. walls.
        REAL selfHatred = 1;
        if (a.type == gSENSOR_SELF)
        {
            selfHatred *= .5;
            if (a.lr > 0)
            {
                selfHatred *= .5;
                if (b.type == gSENSOR_RIM)
                    selfHatred *= .25;
            }
        }
        if (b.type == gSENSOR_SELF)
        {
            selfHatred *= .5;
            if (b.lr < 0)
            {
                selfHatred *= .5;
                if (a.type == gSENSOR_RIM)
                    selfHatred *= .25;
            }
        }

        // some big distance to return if we don't know anything better
        REAL bigDistance = owner_->MaxWallsLength();
        if (bigDistance <= 0)
            bigDistance = owner_->GetDistance();

        if (a.hitOwner_ != b.hitOwner_)
        {
            // different owners? Great, there has to be a way through!
            REAL ret = a.hitDistance_ + b.hitDistance_;

            if (rim)
            {
                ret = bigDistance * .001 + ret * .01 +
                      (a.before_hit - b.before_hit).Norm();

                // we love going between the rim and enemies
                if (!self)
                    ret = bigDistance * 2;
            }

            // minimal factor should be 1, this path should never return something
            // smaller than the paths where only one cycle's walls are hit
            ret *= 16;

            // or empty space
            if (a.type == gSENSOR_NONE || b.type == gSENSOR_NONE)
                ret *= 2;

            return ret * selfHatred;
        }
        else if (rim)
        {
            // at least one rim wall? Take the distance between the hit positions.
            return (a.before_hit - b.before_hit).Norm() * selfHatred;
        }
        else if (a.type == gSENSOR_NONE && b.type == gSENSOR_NONE)
        {
            // empty space! Woo!
            return owner_->GetDistance() * 256;
        }
        else if (a.lr != b.lr)
        {
            // different directions? Also great!
            return (fabsf(a.hitDistance_ - b.hitDistance_) + .25 * bigDistance) *
                   selfHatred;
        }

    else if ( - 2 * a.lr * (a.windingNumber_ - b.windingNumber_ ) >
    owner_->Grid()->WindingNumber() )
    {
        // this looks like a way out to me
        return fabsf( a.hitDistance_ - b.hitDistance_ ) * 10 * selfHatred;
    }

        else
        {
            // well, the longer the wall segment between the two points, the better.
            return fabsf(a.hitDistance_ - b.hitDistance_) * selfHatred;
        }

        // default: hit distance
        return (a.before_hit - b.before_hit).Norm() * selfHatred;
    }

        // check how far the hit wall extends straight into the given direction
        REAL HitWallExtends(eCoord const &dir, eCoord const &origin)
        {
            if (!ehit || !ehit->Other())
            {
                return 1E+30;
            }

            REAL ret = -1E+30;
            eCoord ends[2] = {*ehit->Point(), *ehit->Other()->Point()};
            for (int i = 1; i >= 0; --i)
            {
                REAL newRet = eCoord::F(dir, ends[i] - origin);
                if (newRet > ret)
                    ret = newRet;
            }

            return ret;
        }
        gCycle *owner_;
        gCycle *hitOwner_;   // the owner of the hit wall
        REAL hitTime_;       // the time the hit wall was built at
        REAL hitDistance_;   // the distance of the wall to the cycle that built it
        short lrSuggestion_; // sensor's oppinon on whether moving to the left or
                             // right of the hit wall is recommended (-1 for left,
                             // +1 for right)
        int windingNumber_;  // the number of turns (with sign) the cycle has taken
    };

bool gHelperEnemiesData::exist(gCycle* enemy) {
    return bool(enemy);
}

gCycle* gHelperEnemiesData::detectEnemies() {
    allEnemies.Clear();
    const tList<eGameObject>& gameObjects = owner_->Grid()->GameObjects();

    closestEnemy = nullptr;
    REAL currentMax = 1212121212;
    for (int i = gameObjects.Len() - 1; i >= 0; i--) {
        gCycle* other = dynamic_cast<gCycle*>(gameObjects(i));
        if (bool(other) && other->Alive() && other->Team() != owner_->Team()) {
            REAL positionDifference = st_GetDifference(other->Position(), owner_->Position());

            if (positionDifference <= currentMax) {
                currentMax = positionDifference;
                closestEnemy = other;
            }
            allEnemies.Insert(other);
        }
    }
    return closestEnemy;
}

REAL gSmartTurningCornerData::getTurnTime(REAL speed) {
    return distanceFromPlayer /= speed * .98f;
}

bool gSmartTurningCornerData::isInfront(eCoord pos, eCoord dir) {
    return eCoord::F(dir, currentPos - pos) > 0;
}

gHelperSensors::gHelperSensors(gSensor* front_, gSensor* left_, gSensor* right_) :
front(front_), left(left_), right(right_) {}


gHelperSensorsData::gHelperSensorsData(gCycle *owner) :
    owner_(owner),
    lock(false),
    front_stored(new gSensor(owner_, owner_->Position(), owner_->Direction())),
    left_stored(new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(0, 1)))),
    right_stored(new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(0, -1))))
{
    front_stored->detect(sg_helperSensorRange);
    left_stored->detect(sg_helperSensorRange);
    right_stored->detect(sg_helperSensorRange);
    toggleLock();
}
void gHelperSensorsData::toggleLock() { lock = !lock; }

gSensor * gHelperSensorsData::getSensor(int dir)
{
    return getSensor(owner_->Position(),dir);
}

gSensor * gHelperSensorsData::getSensor(eCoord start, int dir)
{
    if (sg_helperSensorLightUsagetMode && lock)
    {
        switch (dir)
        {
        case -1:
        {
            return left_stored;
        }
        case 0:
        {
            return front_stored;
        }
        case 1:
        {
            return right_stored;
        }
        }
    }
    else
    {
switch (dir)
        {
        case -1:
        {
            gSensor* left = new gSensor(owner_, start, owner_->Direction().Turn(eCoord(0, 1)));
            left->detect(sg_helperSensorRange);
            return left;
        }
        case 0:
        {
            gSensor* front = new gSensor(owner_, start, owner_->Direction());
            front->detect(sg_helperSensorRange);
            return front;
        }
        case 1:
        {
            gSensor* right = new gSensor(owner_, start, owner_->Direction().Turn(eCoord(0, -1)));
            right->detect(sg_helperSensorRange);
            return right;
        }
        }
    }
}

gHelperSensors *gHelperSensorsData::getSensors()
{

    if (sg_helperSensorLightUsagetMode && lock)
    {
        return new gHelperSensors(front_stored, left_stored, right_stored);
    }

    gSensor *front = new gSensor(owner_, owner_->Position(), owner_->Direction());
    front->detect(sg_helperSensorRange);
    gSensor *left = new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(0, 1)));
    left->detect(sg_helperSensorRange);
    gSensor *right = new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(0, -1)));
    right->detect(sg_helperSensorRange);
    return new gHelperSensors(front, left, right);
}



gHelperData::gHelperData(gHelperSensorsData &sensors_, REAL &a_speedFactor,
                         REAL &a_turnSpeedFactor, REAL &a_turnSpeedFactorPercent,
                         REAL &a_turnDistance, REAL a_thinkAgain, REAL a_turnDir,
                         REAL a_turnTime)
    : sensors(sensors_),
      speedFactor(a_speedFactor),
      turnSpeedFactor(a_turnSpeedFactor),
      turnSpeedFactorPercent(a_turnSpeedFactorPercent),
      turnDistance(a_turnDistance),
      thinkAgain(a_thinkAgain),
      turnDir(a_turnDir),
      turnTime(a_turnTime)
{ }


class gSmarterBot
{
    gSmarterBot(gHelper *helper, gCycle *owner,  ePlayerNetID *player)
            :  helper_             ( helper ),
               owner_              ( owner ),
               player_             ( player )
                {

                }
void createAI(){
    if (!ai) {
                     ai 	= tNEW( gAIPlayer ) ();
                     sg_AIReferences.Add( ai );
                    ai 	= tNEW( gAIPlayer ) ();
                     ai->character = BestIQ( 1000 );
    } else {
    //ai->ControlObject(owner_);
    //ai->Think();
            //ai->SetName( ai->character->name );
            //ai->UpdateTeam();
    }
}

void Exist() {
    con << "Created AI? " << bool(ai) << "\n";
}
public:
    static gSmarterBot & Get( gHelper * helper, gCycle *owner,  ePlayerNetID *player )
    {
        tASSERT( helper );

        // // create
        // if ( helper->smarterBot.get() == 0 )
        //     helper->smarterBot.reset( new gSmarterBot( helper, owner, player ) );

        // return *helper->smarterBot;
    }
private:
    friend class gHelper;
    gHelper *helper_;
    gCycle * owner_;         //!< owner of chatbot
    ePlayerNetID *player_;
    gAIPlayer *ai;

};


//SmartTurning
    gSmartTurning::gSmartTurning(gHelper *helper, gCycle *owner)
            : helper_             ( helper ),
              owner_              ( owner ),
              localCurrentTime    ( owner_->localCurrentTime ),
              lastTurnAttemptTime ( owner_->lastTurnAttemptTime ),
              lastTurnAttemptDir  ( owner_->lastTurnAttemptDir ),
              lastTurnTime        ( owner_->lastTurnTime ),
              turnIgnoreTime      ( owner_->turnIgnoreTime ),
              lastTurnDir         ( owner_->lastTurnDir ), // 0 none, -1 left , 1 right
              blockTurn           ( owner_->blockTurn ), // 0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH
              forceTurn           ( owner_->forceTurn ) // 0 = NONE, -1 = LEFT, 1 = RIGHT

    {
    }

    void gSmartTurning::Activate( gHelperData &data ) {

        if (sg_helperSmartTurningSurvive) {
            sg_helperSmartTurningOpposite = 0; sg_helperSmartTurningPlan = 0;
            smartTurningSurvive(data);
        }

        if (sg_helperSmartTurningOpposite) {
            sg_helperSmartTurningSurvive = 0; sg_helperSmartTurningPlan = 0;
            smartTurningOpposite(data);
        }

        if (sg_helperSmartTurningPlan) {
            sg_helperSmartTurningSurvive = 0; sg_helperSmartTurningOpposite = 0;
            smartTurningPlan(data);
        }

        if (sg_helperSmartTurningAutoBrake) {
            autoUnBrake();
        }

        if (sg_helperSmartTurningFollowTail) {
            followTail(data);
        }

        if (sg_helperSmartTurningFrontBot) {
            smartTurningFrontBot(data);
        }
    }

    bool gSmartTurning::isClose(eCoord pos, REAL closeFactor){
    eCoord distanceToPos = owner_->pos - pos;
    return (distanceToPos.x < closeFactor &&
            distanceToPos.y < closeFactor &&
            distanceToPos.x > -closeFactor &&
            distanceToPos.y > -closeFactor);
    }

    void gSmartTurning::smartTurningPlan(gHelperData &data) {
        if (!helper_->aliveCheck()) { return; }
        if (!sg_helperShowCorners) {
            helper_->findCorners(data);
        }
        if (helper_->leftCorner.exist) {
            //con << helper_->leftCorner.getTurnTime(owner_->verletSpeed_) << "\n";
            if (  isClose(owner_->pos, sg_helperShowCornersBoundary) && helper_->leftCorner.infront  && data.sensors.getSensor(-1)->hit > 3 && helper_->leftCorner.getTurnTime(owner_->verletSpeed_) <= 0.0005) {
                owner_->Act(&gCycle::se_turnLeft, 1);
            }
        }

        // bool isClose = helper_->leftCorner.isClose(owner_->pos, sg_helperShowCornersBoundary) || helper_->rightCorner.isClose(owner_->pos, sg_helperShowCornersBoundary);
        // if (!isClose){
        //     return;
        // }
        // bool turnLeft = helper_->leftCorner.distanceFromPlayer < helper_->rightCorner.distanceFromPlayer;
        // if (turnLeft) {
        // if (helper_->leftCorner.infront && helper_->leftCorner.distanceFromPlayer <= 0.01 && data.sensors.getSensor(-1)->hit > 5) {
        //     owner_->Act(&gCycle::se_turnLeft, 1);
        // }
        // } else {
        // if (helper_->leftCorner.infront && helper_->rightCorner.distanceFromPlayer <= 0.01 && data.sensors.getSensor(1)->hit > 5) {
        //     owner_->Act(&gCycle::se_turnRight, 1);
        // }
        // }
    }

    void gSmartTurning::thinkSurvive(gHelperData &data) {

    }

    REAL lastTailTurnTime = -999;
    void gSmartTurning::followTail(gHelperData &data) {

        if (!helper_->aliveCheck()) { return; }

        if (owner_->justCreated == true){
            lastTailTurnTime = -999;
        }
        // if (!isClose(owner_->tailPos, data.turnSpeedFactor * 3)) {
        //     return;
        // }

        if (owner_->tailMoving != true) {
            return;
        }
        REAL delay = data.turnSpeedFactor * sg_helperSmartTurningFollowTailDelayMult;
        bool drivingStraight = helper_->drivingStraight();
        REAL canSurviveLeftTurn, canSurviveRightTurn;
        bool closedIn, blockedBySelf;

        canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningFollowTailFreeSpaceMult);
        if (closedIn || blockedBySelf) {
            return;
        }

if (owner_->tailMoving) {
    // Calculate the direction from the cycle to the tail
    eCoord directionToTail = owner_->tailPos - owner_->pos;
    directionToTail.Normalize();

    // Determine if the tail is in front of or behind the cycle
    if (directionToTail * owner_->dir < 0) {
        // The tail is behind the cycle, turn towards it

        // Check if there are walls to the left or right of the cycle
        gSensor leftSensor(owner_, owner_->pos, owner_->dir.Turn(0, 1));
        leftSensor.detect(sg_helperSensorRange);
        gSensor rightSensor(owner_, owner_->pos, owner_->dir.Turn(0, -1));
        rightSensor.detect(sg_helperSensorRange);

        if (directionToTail * owner_->dir > 0 && canSurviveRightTurn && rightSensor.hit > .999999) {
            owner_->Act(&gCycle::se_turnRight, 1);
        }
        else if (canSurviveLeftTurn && leftSensor.hit > .999999) {
            owner_->Act(&gCycle::se_turnLeft, 1);
        }
    }
    else {
        // The tail is in front of the cycle, stay on current path

        // The tail is directly in front of or behind the cycle
        // Stay on current path
    }
}
    return;
        //bool turnedRecently = !(lastTailTurnTime < 0) && owner_->lastTurnTime + delay > owner_->localCurrentTime;
        //con << owner_->lastTurnTime + delay << " > " << owner_->localCurrentTime << "\n" << turnedRecently << "\n";
        bool readyToTurn = owner_->localCurrentTime > lastTailTurnTime + delay;

        if (!readyToTurn) {
            return;
        }

        int turnDirection = thinkPath(owner_->tailPos,data);

        if (canSurviveRightTurn && turnDirection == -1) {
            owner_->Act(&gCycle::se_turnRight, 1);
            lastTailTurnTime = owner_->localCurrentTime;
        }

        if (canSurviveLeftTurn && turnDirection == 1) {
            owner_->Act(&gCycle::se_turnLeft, 1);
            lastTailTurnTime = owner_->localCurrentTime;
        }
    }

    int gSmartTurning::thinkPath( eCoord pos, gHelperData &data ) {

        eFace * posFace = owner_->grid->FindSurroundingFace(pos);


        eHalfEdge::FindPath(owner_->Position(), owner_->CurrentFace(),
                            pos, posFace,
                            owner_,
                            path);


        REAL mindist = 10;
        int lr = 0;
        eCoord dir = owner_->Direction();

        REAL ls=data.sensors.getSensor(-1)->hit;
        REAL rs=data.sensors.getSensor(1)->hit;

         for (int z = 5; z>=0; z--){
             if (!path.Proceed()){
                 break;
             }
         }

        bool goon   = path.Proceed();
        bool nogood = false;

        do
        {
            if (goon)
                goon = path.GoBack();
            else
                goon = true;

            eCoord pos   = path.CurrentPosition() + path.CurrentOffset() * 0.1f;
            eCoord opos  = owner_->Position();
            eCoord odir  = pos - opos;

            eCoord intermediate = opos + dir * eCoord::F(odir, dir);

            gSensor p(owner_, opos, intermediate - opos);
            p.detect(1.1f);
            nogood = (p.hit <= .999999999 || eCoord::F(path.CurrentOffset(), odir) < 0);

            if (!nogood)
            {
                gSensor p(owner_, intermediate, pos - intermediate);
                p.detect(1);
                nogood = (p.hit <= .99999999 || eCoord::F(path.CurrentOffset(), odir) < 0);
            }
            //helper_->debugLine(0,1,1,5,4*data.speedFactor,pos,pos);

        }
        while (goon && nogood);

        if (goon)
        {

            // now we have found our next goal. Try to get there.
            eCoord pos    = owner_->Position();
            eCoord target = path.CurrentPosition();

            helper_->debugLine(.2,1,0,5,data.speedFactor,target,target);
            helper_->debugLine(.2,1,0,5,data.speedFactor,pos,target);
            // look how far ahead the target is:
            REAL ahead = eCoord::F(target - pos, dir) + eCoord::F(path.CurrentOffset(), dir);

            if ( ahead > 0)
            {	  // it is still before us. just wait a while.
                mindist = ahead;
            }
            else
            { // we have passed it. Make a turn towards it.
                REAL side = (target - pos) * dir;

                if ( !((side > 0 && ls < 3) || (side < 0 && rs < 3))
                        && (fabs(side) > 3 || ahead < -10) )
                {
                    lr += (side > 0 ? 1 : -1);
                    return lr;
                }
            }
        }

    }

    void gSmartTurning::autoUnBrake() {
        if (!helper_->aliveCheck()) { return; }
        REAL brakingReservoir = owner_->GetBrakingReservoir();
        bool cycleBrakeDeplete = true;

        if (sg_helperSmartTurningAutoBrakeDeplete && sg_helperSmartTurningAutoBrakeMin == 0 && sg_cycleBrakeDeplete != 1) {
            cycleBrakeDeplete = false;
        }

        if (brakingReservoir <= sg_helperSmartTurningAutoBrakeMin && cycleBrakeDeplete) {
            owner_->braking = false;
        }

         if (brakingReservoir >= sg_helperSmartTurningAutoBrakeMax) {
             owner_->braking = true;
         }
    }

    void gSmartTurning::smartTurningOpposite(gHelperData &data) {
        if (!helper_->aliveCheck()) { return; }
        REAL canSurviveLeftTurn, canSurviveRightTurn;
        bool closedIn, blockedBySelf;


        canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);

        if ((closedIn) || (closedIn) && blockedBySelf) {
            goto SKIP_FORCETURN;
        }

        if (!canSurviveLeftTurn && canSurviveRightTurn)
        {
            owner_->forceTurn = 1;
            owner_->blockTurn = 0;
            return;
        }
        else if (canSurviveLeftTurn && !canSurviveRightTurn)
        {
            owner_->forceTurn = -1;
            owner_->blockTurn = 0;
            return;
        }

        SKIP_FORCETURN:
        {
            this->forceTurn = 0;
            return;
        }
    }

    void gSmartTurning::smartTurningSurvive(gHelperData &data) {
        if (!helper_->aliveCheck()) { return; }
        if (!owner_->pendingTurns.empty()) { return; }
        REAL canSurviveLeftTurn, canSurviveRightTurn;
        bool closedIn, blockedBySelf;

        canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);
        //con << canSurviveLeftTurn << " | " << canSurviveRightTurn << " | " << closedIn << " | " << blockedBySelf << " | " << sg_helperSmartTurningSpace << "\n";

        if ((closedIn && sg_helperSmartTurningClosedIn ) || (closedIn) && blockedBySelf) {
            goto SKIP_BLOCKTURN;
        }

        if (!canSurviveLeftTurn && !canSurviveRightTurn) {
            this->blockTurn = 2;
            //con << "BLOCKING BOTH TURNS\n";
            return;
        }
        if (!canSurviveLeftTurn) {
            //con << "BLOCKING left TURN\n";
            this->blockTurn = -1;

            return;
        }
        if (!canSurviveRightTurn) {
            this->blockTurn = 1;
            ///con << "BLOCKING RIGHT TURN\n";

            return;
        }

        SKIP_BLOCKTURN:
        {
            //this->turnIgnoreTime = -999;
            this->blockTurn = 0;
            owner_->lastBlockedTurn = this->blockTurn;
            if (sg_helperSmartTurningSurviveTrace) {
                smartTurningSurviveTrace(data);
            }
            return;
        }
    }

    void gSmartTurning::smartTurningSurviveTrace(gHelperData &data) {
        if (!helper_->aliveCheck()) { return; }
        if (!helper_->drivingStraight()) { return; }
        int lastTurn = owner_->lastTurnAttemptDir;
        //con << lastTurn << " \n";
        if (owner_->lastTurnAttemptTime < owner_->lastTurnTime) {
            return;
        }

        switch(lastTurn) {
            case -1: {
                REAL turnTimeFactor = helper_->leftCorner.getTurnTime(owner_->Speed());

                if ( helper_->leftCorner.exist && isClose(helper_->leftCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactor) &&
                     owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime/(10 * data.turnSpeedFactor)) > helper_->leftCorner.noticedTime &&
                     helper_->leftCorner.getTurnTime(owner_->Speed()) > 0 &&
                     (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime)  ) {
                    makeTurnIfPossible(data,-1,1);
                }
                return;
            }
            case 1: {
                REAL turnTimeFactor = helper_->rightCorner.getTurnTime(owner_->Speed());

                if ( helper_->rightCorner.exist && isClose(helper_->rightCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactor) &&
                     owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime/(10 * data.turnSpeedFactor)) > helper_->rightCorner.noticedTime &&
                     helper_->rightCorner.getTurnTime(owner_->Speed()) > 0 &&
                     (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime)  ) {
                    makeTurnIfPossible(data,1,1);
                }
                return;
            }
        }
    }

    bool gSmartTurning::makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor){
        if (!helper_->drivingStraight())
            return false;

        switch (dir) {
            case -1: {
                if (canSurviveTurnSpecific(data,dir,spaceFactor)) {
                    owner_->ActBot(&gCycle::se_turnLeft, 1);
                    return true;
                }
                return false;
            }
            case 1: {
                if (canSurviveTurnSpecific(data,dir,spaceFactor)) {
                    owner_->ActBot(&gCycle::se_turnRight, 1);
                    return true;
                }
                return false;
            }
            default: {
            return false;
            }
        }
    }

    bool gSmartTurning::canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor)
    {
        REAL compareFactor;

        if (spaceFactor > 0)
        {
            compareFactor = spaceFactor * data.turnSpeedFactor;
        }
        else
        {
            calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);
            compareFactor = rubberFactor;
        }

        if (dir == -1)
        {
            return data.sensors.getSensor(-1)->hit > compareFactor;
        }
        else if (dir == 1)
        {
            return data.sensors.getSensor(1)->hit > compareFactor;
        }
    }

    void gSmartTurning::canSurviveTurn(gHelperData &data, REAL &canSurviveLeftTurn, REAL &canSurviveRightTurn, bool &closedIn, bool &blockedBySelf, REAL freeSpaceFactor) {
        if (!helper_->aliveCheck()) { return; }

        calculateRubberFactor(sg_helperSmartTurningRubberTimeMult,sg_helperSmartTurningRubberFactorMult);

        canSurviveLeftTurn = true;
        canSurviveRightTurn = true;

        gSensor *front = data.sensors.getSensor(0);
        gSensor *left = data.sensors.getSensor(-1);
        gSensor *right = data.sensors.getSensor(1);

        bool canTurnLeftRubber = true,canTurnRightRubber = true,canTurnLeftSpace = true,canTurnRightSpace = true;
        REAL closedInFactor = data.turnSpeedFactor * sg_helperSmartTurningClosedInMult;
        closedIn = (front->hit <= closedInFactor && left->hit <= closedInFactor && right->hit <= closedInFactor);
        blockedBySelf = (left->type == gSENSOR_SELF && right->type == gSENSOR_SELF && front->type == gSENSOR_SELF);

        if (freeSpaceFactor > 0) {
//            con << "right " << data.sensors.getSensor(-1)->hit << " < " << data.turnSpeedFactor << " * " << freeSpaceFactor << "\n";
            if (left->hit < data.turnSpeedFactor * freeSpaceFactor) {
                canTurnLeftSpace = false;
            }
//            con << "left " << data.sensors.getSensor(1)->hit << " < " << data.turnSpeedFactor << " * " << freeSpaceFactor << " " << (data.sensors.getSensor(1)->hit < data.turnSpeedFactor * freeSpaceFactor) << "\n";
            if (right->hit < data.turnSpeedFactor * freeSpaceFactor) {
                canTurnRightSpace = false;
            }
        }

   //     con << "rubber factor " << rubberFactor <<"\n";
        if (left->hit < rubberFactor) {
            canTurnLeftRubber = false;
        }

        if (right->hit < rubberFactor) {
            canTurnRightRubber = false;
        }

        if (freeSpaceFactor > 0){
            canSurviveLeftTurn = !closedIn ? canTurnLeftRubber && canTurnLeftSpace : canTurnLeftRubber ;
            canSurviveRightTurn = !closedIn ? canTurnRightRubber && canTurnRightSpace : canTurnRightRubber ;
        } else {
            canSurviveLeftTurn = canTurnLeftRubber;
            canSurviveRightTurn = canTurnRightRubber;
        }


        // if (sg_helperDebug) {

        // gSensor left(owner_, owner_->pos, owner_->dir.Turn(eCoord(0, 1)));
        // left->detect(data.turnSpeedFactor * sg_helperSmartTurningClosedInMult);

        // gSensor right(owner_, owner_->pos, owner_->dir.Turn(eCoord(0, -1)));
        // right->detect(data.turnSpeedFactor * sg_helperSmartTurningClosedInMult);
        // helper_->debugLine(1,0,0,0,data.speedFactor,owner_->pos,left->before_hit);
        // helper_->debugLine(1,0,0,0,data.speedFactor,owner_->pos,right->before_hit);
        // }
    }


    void gSmartTurning::calculateRubberFactor(REAL rubberMult, REAL rubberFactorMult) {
        if (!helper_->aliveCheck()) { return; }
        REAL rubberGranted, rubberEffectiveness;
        sg_RubberValues( owner_->Player(), owner_->verletSpeed_, rubberGranted, rubberEffectiveness );
        rubberTime = ( rubberGranted - owner_->GetRubber() )*rubberEffectiveness/owner_->verletSpeed_;
        rubberRatio = owner_->GetRubber()/rubberGranted;
        rubberFactor = owner_->verletSpeed_ * (  owner_->GetTurnDelay() - rubberTime * sg_helperSmartTurningRubberTimeMult );
        rubberFactor *= rubberFactorMult;
    }

    gSmartTurning & gSmartTurning::Get( gHelper * helper, gCycle *owner)
    {
        tASSERT( helper );

        // create
        if ( helper->smartTurning.get() == 0 )
            helper->smartTurning.reset( new gSmartTurning( helper, owner ) );

        return *helper->smartTurning;
    }

    void gSmartTurning::smartTurningFrontBot(gHelperData &data)
    {
        if (!owner_->pendingTurns.empty())
        {
            return;
        }

        REAL hitRange = data.sensors.getSensor(0)->hit;
        if (hitRange <= sg_helperSmartTurningFrontBotActivationRange * data.turnSpeedFactor)
        {
            calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);
            gHelperEmergencyTurn emergencyTurn(owner_, helper_);

            if (rubberRatio >= sg_helperSmartTurningFrontBotActivationRubber || hitRange <= sg_helperSmartTurningFrontBotActivationSpace)
            {
               REAL turn = emergencyTurn.getTurn();
               // REAL blockTurn = owner_->blockTurn;
               // owner_->blockTurn = 0;
               // owner_->turnIgnoreTime = -999;
               if (turn == 0) {
                    return;
               }
               noTurns = 0;
               bool turnPossible = makeTurnIfPossible(data, turn , 0);
               // owner_->blockTurn = blockTurn;

                if (sg_helperSmartTurningFrontBotDisableTime > 0 && turnPossible)
                {
                    owner_->turnIgnoreTime = owner_->localCurrentTime + (sg_helperSmartTurningFrontBotDisableTime * data.turnDistance);
                }
            }
        }
        // con << rubberFactor << " | " << rubberTime << " | " << rubberRatio << "\n";
    }

    int gSmartTurning::getEmergencyTurn(gHelperData &data)
    {
        // seconds to plan ahead
        REAL minstep = 0;
        // cylce data
        REAL speed = owner_->Speed();
        eCoord dir = owner_->Direction();
        eCoord pos = owner_->Position();

        REAL lookahead = speed;
        REAL ownerTurnDelay = owner_->GetTurnDelay();
        REAL range = 10 * data.turnSpeedFactor;
        eCoord scanDir = dir * range;

        REAL frontFactor = .5;

        SensorPub front(owner_, pos, scanDir);

        front.detect(frontFactor);

        owner_->enemyInfluence.AddSensor(front, 0, owner_);

        REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

        // get extra time we get through rubber usage
        calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);

        if (front.ehit)
        {
            // these checks can hit our last wall and fail. Temporarily set it to
            // NULL.
            tJUST_CONTROLLED_PTR<gNetPlayerWall> lastWall = owner_->lastWall;
            owner_->lastWall = NULL;

            REAL narrowFront = 1;

            // cast four diagonal rays
            SensorPub forwardLeft(owner_, pos, scanDir.Turn(+1, +1));
            SensorPub backwardLeft(owner_, pos, scanDir.Turn(-1, +narrowFront));
            SensorPub forwardRight(owner_, pos, scanDir.Turn(+1, -1));
            SensorPub backwardRight(owner_, pos, scanDir.Turn(-1, -narrowFront));

            REAL detectRange = (owner_->Speed());
            forwardRight.detect(detectRange);
            backwardRight.detect(detectRange);
            forwardLeft.detect(detectRange);
            backwardLeft.detect(detectRange);

            // determine survival chances in the four directions
            REAL frontOpen = forwardRight.Distance(forwardLeft, forwardRight);
            REAL leftOpen = forwardRight.Distance(forwardLeft, backwardLeft);
            REAL rightOpen = forwardRight.Distance(forwardRight, backwardRight);
            REAL rearOpen = forwardRight.Distance(backwardLeft, backwardRight);

            SensorPub self(owner_, pos, scanDir.Turn(-1, 0));
            // fake entries
            self.before_hit = pos;
            self.windingNumber_ = owner_->windingNumber_;
            self.type = gSENSOR_SELF;
            self.hitDistance_ = 0;
            self.hitOwner_ = owner_;
            self.hitTime_ = owner_->localCurrentTime;
            self.lr = -1;
            REAL rearLeftOpen = forwardLeft.Distance(backwardLeft, self);
            self.lr = 1;
            REAL rearRightOpen = forwardLeft.Distance(backwardRight, self);

            // override rim hugging
            if (forwardRight.type == gSENSOR_SELF &&
                forwardLeft.type == gSENSOR_RIM &&
                backwardRight.type == gSENSOR_SELF &&
                backwardLeft.type == gSENSOR_RIM && forwardRight.lr == -1 &&
                backwardRight.lr == -1)
            {

                if (rightOpen > speed * (ownerTurnDelay - rubberTime * .8))
                {
                    helper_->HelperDebug("FRONT BOT THINK","Right by rim","");
                    //nextTurn_ = 1;
                    //noTurnFound = 0;
                    return 1;
                }
            }

            if (forwardLeft.type == gSENSOR_SELF &&
                forwardRight.type == gSENSOR_RIM &&
                backwardLeft.type == gSENSOR_SELF &&
                backwardRight.type == gSENSOR_RIM && forwardLeft.lr == 1 &&
                backwardLeft.lr == 1)
            {

                if (leftOpen > speed * (ownerTurnDelay - rubberTime * .8))
                {
                    helper_->HelperDebug("FRONT BOT THINK","Left by rim","");
                    //nextTurn_ = -1;
                    //noTurnFound = 0;
                    return -1;
                }
            }

            // get the best turn direction
            int bestDir =
                (leftOpen + rearLeftOpen > rightOpen + rearRightOpen) ? 1 : -1;

            SensorPub direct(owner_, pos, scanDir.Turn(0, bestDir));
            direct.detect(1);

            // restore last wall
            owner_->lastWall = lastWall;

            uActionPlayer *bestAction =
                (bestDir > 0) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;

            REAL bestOpen = (bestDir > 0) ? leftOpen : rightOpen;
            SensorPub &bestForward = (bestDir > 0) ? forwardLeft : forwardRight;
            SensorPub &bestBackward = (bestDir > 0) ? backwardLeft : backwardRight;

            // only turn if the hole has a shape that allows better entry after we do
            // a zig-zag, or if we're past the good turning point see how the survival
            // chance is distributed between forward and backward half
            REAL forwardHalf = bestForward.Distance(direct, bestForward);
            REAL backwardHalf = bestForward.Distance(direct, bestBackward);

            REAL forwardOverhang =
                bestForward.HitWallExtends(bestForward.Direction(), pos);
            REAL backwardOverhang =
                bestBackward.HitWallExtends(bestForward.Direction(), pos);

            // we have to move forward this much before we can hope to turn
            minMoveOn = bestBackward.HitWallExtends(dir, pos);

            // maybe the direct to the side sensor is better?
            REAL minMoveOnOther = direct.HitWallExtends(dir, pos);

            // determine how far we can drive on
            maxMoveOn = bestForward.HitWallExtends(dir, pos);
            REAL maxMoveOnOther = front.HitWallExtends(dir, pos);
            if (maxMoveOn > maxMoveOnOther)
                maxMoveOn = maxMoveOnOther;

            if (maxMoveOn > minMoveOnOther && forwardHalf > backwardHalf &&
                direct.hitOwner_ == bestBackward.hitOwner_)
            {
                backwardOverhang = direct.HitWallExtends(bestForward.Direction(), pos);
                minMoveOn = minMoveOnOther;
            }

            // best place to turn
            moveOn =
                .5 * (minMoveOn * (1 + rubberRatio) + maxMoveOn * (1 - rubberRatio));

            if (frontOpen < bestOpen &&
                (forwardOverhang <= backwardOverhang ||
                 (minMoveOn < 0 && moveOn < minstep * speed)))
            {

                minMoveOn = maxMoveOn = moveOn = 0;

                {
                    if (!CanMakeTurn(bestAction))
                    {
                        //botTurnMethod = "bestActionV1";
                        //noTurnFound = 0;
                        return 0;
                    }
                    //              bool boxedInLeft = (backwardLeft.type == gSENSOR_SELF
                    //              && forwardLeft.type == gSENSOR_SELF);
                    //                bool boxedInRight = (backwardRight.type ==
                    //                gSENSOR_SELF && forwardRight.type == gSENSOR_SELF);
                    bool leftRearMoreOpen = rearLeftOpen > rearRightOpen;

                    if (data.sensors.getSensor(-1)->hit > data.turnSpeedFactor &&
                        (bestAction == &gCycle::se_turnLeft) && leftRearMoreOpen)
                    //&&                (!(forwardLeft.type == gSENSOR_SELF &&
                    // backwardLeft.type == gSENSOR_SELF) ) )) //|| (backwardLeft.type ==
                    // gSENSOR_ENEMY) )
                    {
                        helper_->HelperDebug("FRONT BOT THINK","Left by bestActionv1","");
                        //nextTurn_ = -1;
                        //noTurnFound = 0;
                        return -1;
                    }
                    else if (data.sensors.getSensor(1)->hit > data.turnSpeedFactor &&

                             (bestAction == &gCycle::se_turnRight) &&
                             !leftRearMoreOpen) // &&
                                                //      (!(forwardRight.type ==
                                                //      gSENSOR_SELF &&
                                                //      backwardRight.type ==
                                                //      gSENSOR_SELF)) )) //||
                                                //      (backwardRight.type ==
                                                //      gSENSOR_ENEMY) )
                    {
                        helper_->HelperDebug("FRONT BOT THINK","Right by bestActionv1","");
                        //nextTurn_ = 1;
                        //noTurnFound = 0;
                        return 1;
                    }
                }
            }
            else
            {
                // the best
                REAL bestSoFar = frontOpen > bestOpen ? frontOpen : bestOpen;
                bestSoFar *= (10 * (1 - rubberRatio) + 1);

                if (rearOpen > bestSoFar &&
                    (rearLeftOpen > bestSoFar || rearRightOpen > bestSoFar))
                {
                    bool goLeft = rearLeftOpen > rearRightOpen;

                    // dead end. reverse into the opposite direction of the front wall
                    uActionPlayer *bestAction =
                        goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
                    uActionPlayer *otherAction =
                        !goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
                    SensorPub &bestForward = goLeft ? forwardLeft : forwardRight;
                    SensorPub &bestBackward = goLeft ? backwardLeft : backwardRight;
                    SensorPub &otherForward = !goLeft ? forwardLeft : forwardRight;
                    SensorPub &otherBackward = !goLeft ? backwardLeft : backwardRight;

                    // space in the two directions available for turns
                    REAL bestHit = bestForward.hit > bestBackward.hit ? bestBackward.hit
                                                                      : bestForward.hit;
                    REAL otherHit = otherForward.hit > otherBackward.hit
                                        ? otherBackward.hit
                                        : otherForward.hit;

                    bool wait = false;

                    if (!CanMakeTurn(bestAction))
                    {
                        //botTurnMethod = "bestActionV2";
                        //noTurnFound = 0;
                        helper_->HelperDebug("FRONT BOT THINK","cant make turn bestAction","");
                        return 0;
                    }

                    // well, after a short turn to the right if space is tight
                    if (bestHit * lookahead <
                        ownerTurnDelay +
                            rubberTime)
                    { /*
                              if (otherHit < bestForward.hit * 2 && front.hit *
                              lookahead > ownerTurnDelay * 2)
                              {
                                  // wait a bit, perhaps there will be a better
                              spot
                                 // wait = true;
                              }
                              else */
                        {
                            // owner_->Act( otherAction, 1 );

                            // there needs to be space ahead to finish the maneuver correctly
                            if (maxMoveOn < speed * ownerTurnDelay)
                            {
                                if (!CanMakeTurn(otherAction))
                                {
                                    //botTurnMethod = "otherAction";
                                    //noTurnFound = 0;
                                    helper_->HelperDebug("FRONT BOT THINK","cant make turn otherAction","");
                                    return 0;
                                }
                                // there isn't. oh well, turn into the wrong direction
                                // completely, see if I care
                                if (otherAction == &gCycle::se_turnLeft)
                                {
                                    helper_->HelperDebug("FRONT BOT THINK","Left by otherAction","");
                                    //nextTurn_ = -1;
                                    //noTurnFound = 0;
                                    return -1;
                                }
                                else if (otherAction == &gCycle::se_turnRight)
                                {
                                    helper_->HelperDebug("FRONT BOT THINK","Right by otherAction","");
                                    //nextTurn_ = 1;
                                    //noTurnFound = 0;
                                    return 1;
                                }
                            }
                        }
                    }

                    if (wait)
                    {
                        if (bestAction == &gCycle::se_turnLeft)
                        {
                            helper_->HelperDebug("FRONT BOT THINK","Left by bestActionv2","");
                            //nextTurn_ = -1;
                            //noTurnFound = 0;
                            return -1;
                        }
                        else if (bestAction == &gCycle::se_turnRight)
                        {
                            helper_->HelperDebug("FRONT BOT THINK","Right by bestActionv2","");
                            //nextTurn_ = 1;
                            //noTurnFound = 0;
                            return 1;
                        }
                    }
                }
            }
        }
        //noTurnFound++;
        helper_->HelperDebug("FRONT BOT THINK","No turns found","");
        return 2;
    }

    bool gSmartTurning::CanMakeTurn(uActionPlayer *action)
    {
        return owner_->CanMakeTurn((action == &gCycle::se_turnRight) ? 1 : -1);
    }

    gHelper::gHelper(gCycle *owner)
        : owner_(owner),
          player_(owner->Player()),
          ownerWallLength(owner->ThisWallsLength()),
          ownerTurnDelay(owner->GetTurnDelay()),
          lastHelperDebugMessage(NULL),
          lastHelperDebugMessageTimeStamp(-999),
          data_stored(NULL)
    {
        aiCreated = false;
        ownerPos = &owner_->pos;
        ownerDir = &owner_->dir;
        tailPos = &owner_->tailPos;
        ownerSpeed = &owner_->verletSpeed_;
        enemies.owner_ = owner;
        gSmartTurning::Get(this, owner);
        gPathHelper::Get(this, owner);
    }

    gCycle* gHelper::getOwner() { return owner_; }

    eCoord gHelper::closestCorner(eCoord center, REAL radius)
    {

        eCoord corner[3];
        eCoord closestCorner;

        corner[0] = eCoord(center.x - (radius), center.y + (radius));
        corner[1] = eCoord(center.x + (radius), center.y + (radius));
        corner[2] = eCoord(center.x + (radius), center.y - (radius));
        corner[3] = eCoord(center.x - (radius), center.y - (radius));

        REAL positionDifference[3];

        positionDifference[0] = st_GetDifference(corner[0], (*ownerPos));
        positionDifference[1] = st_GetDifference(corner[1], (*ownerPos));
        positionDifference[2] = st_GetDifference(corner[2], (*ownerPos));
        positionDifference[3] = st_GetDifference(corner[3], (*ownerPos));

        REAL currentMax = 99999;
        for (int i = 0; i < 4; i++)
        {
            if (positionDifference[i] <= currentMax)
            {
                currentMax = positionDifference[i];
                closestCorner = corner[i];
            }
        }
        return closestCorner;
    }

    void gHelper::debugLine(REAL R, REAL G, REAL B, REAL height, REAL timeout,
                   eCoord start,eCoord end, REAL brightness) {
        if (timeout >= 6) {
            timeout = 1;
        }
        REAL startHeight = height;
        if (start == end) {
            startHeight = 0;
        }
        eDebugLine::SetTimeout(timeout);
        eDebugLine::SetColor(R * sg_helperBrightness * brightness, G * sg_helperBrightness * brightness, B * sg_helperBrightness* brightness);
        eDebugLine::Draw(start, startHeight, end, height);
    }

    void gHelper::detectCut(gHelperData &data, int detectionRange)
    {
        if (!aliveCheck()) { return; }
        REAL timeout = data.speedFactor + sg_helperDetectCutTimeout;
        gCycle *target = enemies.closestEnemy;

        if (enemies.exist(target))
        {
            REAL range = ((*ownerSpeed) * (ownerTurnDelay * sg_helperEnemyTracersDelayMult));
            REAL closeReact = (range + detectionRange);

            if (!smartTurning->isClose(target->Position(), closeReact)) {
                return;
            }
            // HelperDebug("detectCut","\n", 1);
            // HelperDebug("detectCut","start", 1);
                eCoord relativeEnemyPos = target->Position() - (*ownerPos);
                eCoord actualEnemyPos = target->Position();
                eCoord enemydir = target->Direction();
                REAL enemyspeed = target->Speed();

                // HelperDebug("detectCut","ownerPos", (*ownerPos));
                // HelperDebug("detectCut","relativeEnemyPos", relativeEnemyPos);
                // HelperDebug("detectCut","actualEnemyPos", actualEnemyPos);
                // HelperDebug("detectCut","enemydir", enemydir);

                relativeEnemyPos = relativeEnemyPos.Turn(ownerDir->Conj()).Turn(0, 1);
                enemydir = enemydir.Turn(ownerDir->Conj()).Turn(0, 1);

                //  relativeEnemyPos makes the enemy 0,0. Owner position relative to enemy as center point
                // HelperDebug("detectCut","relativeEnemyPos after turn ", relativeEnemyPos);
                // HelperDebug("detectCut","enemydir after turn ", enemydir);

                // bool enemyDirection = false;
                // if (enemydir.x <= 0 && enemydir.y == 1)
                // {
                //     enemyDirection = true;
                // }

                int side = 1;
                if (relativeEnemyPos.x < 0)
                {
                    side *= -1;
                    relativeEnemyPos.x *= -1;
                    enemydir.x *= -1;
                }
                // HelperDebug("detectCut","side ", side);
                // HelperDebug("detectCut","relativeEnemyPos.x", relativeEnemyPos.x);
                // HelperDebug("detectCut","enemydir.x", enemydir.x);
                REAL enemydist = target->Lag() * enemyspeed;

                /*
                if (!sr_predictObjects) {
                    relativeEnemyPos = relativeEnemyPos - enemydir * enemydist;
                }
                */
                enemydist += sg_helperDetectCutReact * enemyspeed;

                REAL ourdist = sg_helperDetectCutReact * (*ownerSpeed);

                // HelperDebug("detectCut","enemydist",enemydist);
                // HelperDebug("detectCut","ourdist",ourdist);
                relativeEnemyPos.y -= ourdist;


                REAL forward = -relativeEnemyPos.y + .01;
                if (forward < 0)
                    forward = 0;
                if (forward > enemydist)
                    forward = enemydist;

                relativeEnemyPos.y += forward;
                enemydist -= forward;
                relativeEnemyPos.x -= enemydist;

                // HelperDebug("detectCut","relativeEnemyPos.y after forward",relativeEnemyPos.y);
                // HelperDebug("detectCut","relativeEnemyPos.x after forward ",relativeEnemyPos.x);
    bool canCutUs, canCutEnemy;

    canCutUs = relativeEnemyPos.y * enemyspeed > relativeEnemyPos.x * (*ownerSpeed);
    canCutEnemy = relativeEnemyPos.y * (*ownerSpeed) < -relativeEnemyPos.x * enemyspeed;
    if (canCutUs && !canCutEnemy)
    {
        debugLine(1, 0, 0, sg_helperDetectCutHeight, timeout, (*ownerPos), actualEnemyPos);
    }
    else if (canCutEnemy)
    {
        if (side == -1 && smartTurning->canSurviveTurnSpecific(data,side,range))
        {
            debugLine(0, 1, 0, sg_helperDetectCutHeight, timeout, (*ownerPos), actualEnemyPos);
        }

        if (side == 1 && smartTurning->canSurviveTurnSpecific(data,side,range))
        {
            debugLine(0, 1, 0, sg_helperDetectCutHeight, timeout, (*ownerPos), actualEnemyPos);
        }
    }

    /*
    eCoord myPos = *ownerPos;
eCoord enemyPos = target->Position();
eCoord enemyDir = target->Direction();
REAL enemySpeed = target->Speed();

// Check if the enemy is facing towards or away from you
bool enemyFacingTowards = enemyDir.x * (myPos.x - enemyPos.x) + enemyDir.y * (myPos.y - enemyPos.y) < 0;

// Calculate the distance the enemy will travel before you can react to them cutting you off
REAL enemyDist = target->Lag() * enemySpeed + sg_helperDetectCutReact * enemySpeed;

// Calculate the distance you will travel before you can react to cutting off the enemy
REAL myDist = sg_helperDetectCutReact * (*ownerSpeed);

// Check if you can cut off the enemy
bool canCutEnemy;
if (enemyFacingTowards) {
    // If the enemy is facing towards you, check if their distance traveled is less than the distance you need to travel to cut them off
    canCutEnemy = enemyDist < myDist;
} else {
    // If the enemy is facing away from you, check if your distance traveled is less than the distance they need to travel to cut you off
    canCutEnemy = myDist < enemyDist;
}

// Check if the enemy can cut you off
bool canCutUs;
if (enemyFacingTowards) {
    // If the enemy is facing towards you, check if your distance traveled is less than the distance they need to travel to cut you off
    canCutUs = myDist < enemyDist;
} else {
    // If the enemy is facing away from you, check if their distance traveled is less than the distance you need to travel to cut them off
    canCutUs = enemyDist < myDist;
}

if (canCutUs && !canCutEnemy) {
    // If the enemy can cut you off but you can't cut them off, draw a red line
    debugLine(1, 0, 0, sg_helperDetectCutHeight, timeout, myPos, enemyPos);
} else if (canCutEnemy) {
    // If you can cut the enemy off, draw a green line
    debugLine(0, 1, 0, sg_helperDetectCutHeight, timeout, myPos, enemyPos);
}
*/
        }
    }

    void gHelper::enemyTracers(gHelperData &data, int detectionRange, REAL timeout)
    {
        if (!aliveCheck()) { return; }
        for(int i=0;i < enemies.allEnemies.Len();i++)
        {
            gCycle *enemy = enemies.allEnemies[i];
            if (!enemies.exist(enemy)) {
                continue;
            }

            eCoord enemyPos = enemy->Position();
            REAL R = .1, G = .1, B = 0;
            bool isClose = smartTurning->isClose(enemyPos, detectionRange + data.turnSpeedFactor);
            bool enemyFaster = ((enemy->Speed() > ((*ownerSpeed) * sg_helperEnemyTracersSpeedMult)));
            bool isTeammate = (owner_->Team() == enemy->Team());

            if ((isClose || enemyFaster))
            {
                if (!isTeammate)
                {
                    R = 1, G = 1;
                }

                if (isTeammate)
                {
                R = 1, G = 0, B = 1;
                }

                if (enemyFaster && !isTeammate)
                {
                    R = 1, G = .5, B = 0;
                }

                debugLine(R,G,B,sg_helperEnemyTracersHeight,timeout,(*ownerPos),enemyPos,sg_helperEnemyTracersBrightness);
            }
        }

    }

    bool gHelper::canSeeTarget(eCoord target,REAL passthrough) {
        gSensor sensor(owner_, (*ownerPos), target - (*ownerPos));
        sensor.detect(REAL(.98));
        return (sensor.hit >=passthrough);
    }

    void gHelper::showTail(gHelperData &data)
    {
        if (!aliveCheck()) { return; }
        if (owner_->tailMoving != true) {
            return;
        }

        REAL timeout = sg_helperShowTailTimeout * data.speedFactor;

        if (canSeeTarget((*tailPos),sg_helperShowTailPassthrough)) {
            debugLine(owner_->color_.r,owner_->color_.g,owner_->color_.b,sg_helperShowTailHeight,timeout,(*ownerPos),(*tailPos));
        }
    }

    void gHelper::showTailPath(gHelperData &data)
    {
        if (!aliveCheck()) { return; }
        smartTurning->thinkPath(owner_->tailPos,data);
    }

    void gHelper::showEnemyTail(gHelperData &data)
    {
        if (!aliveCheck()) { return; }
        REAL distanceToTail, timeout;
        for (int i = 0; i < enemies.allEnemies.Len(); i++)
        {
            gCycle *other = enemies.allEnemies[i];
            bool exist = enemies.exist(other);
            if (!exist || exist && !other->tailMoving) {
                continue;
            }
            distanceToTail = sg_helperShowEnemyTailDistanceMult * (eCoord::F(*ownerDir, (other->tailPos) - (*ownerPos)));
            timeout = fabs(distanceToTail) / 10 * data.speedFactor;
            debugLine(other->color_.r, other->color_.g, other->color_.b, sg_helperShowEnemyTailHeight, timeout * sg_helperShowEnemyTailTimeoutMult, other->tailPos, other->tailPos, sg_helperShowEnemyTailBrightness);
        }
    }

    void gHelper::showTailTracer(gHelperData &data)
    {
        if (!aliveCheck()) { return; }
        if (!owner_->tailMoving) {
            return;
        }

        REAL distanceToTail = sg_helperShowTailTracerTimeoutMult * eCoord::F(*ownerDir, (*tailPos) - (*ownerPos));
        REAL timeout = fabs(distanceToTail)/10 * data.speedFactor;

        debugLine(1,1,1,sg_helperShowTailTracerHeight,timeout*sg_helperShowTailTracerTimeoutMult,*tailPos,*tailPos);
    }

    void gHelper::findCorner(gHelperData &data, gSmartTurningCornerData &corner, const gSensor *sensor)
    {

        corner.noticedTime = owner_->localCurrentTime;
        corner.exist = true;

        corner.type = sensor->type;
        corner.currentPos = *sensor->ehit->Point();
        corner.distanceFromPlayer = eCoord::F(*ownerDir, corner.currentPos - (*ownerPos));
        corner.turnTime = leftCorner.getTurnTime(owner_->Speed()) + owner_->localCurrentTime;
        REAL secondEdgeDistance = eCoord::F(*ownerDir, *sensor->ehit->Other()->Point() - (*ownerPos));

        if (corner.distanceFromPlayer < secondEdgeDistance)
        {
            corner.distanceFromPlayer = secondEdgeDistance;
            corner.currentPos = *sensor->ehit->Other()->Point();
        }

        corner.infront = corner.isInfront(*ownerPos, *ownerDir);

        if (corner.lastPos != corner.currentPos)
        {
            corner.lastPos = corner.currentPos;
            corner.ignoredTime = owner_->localCurrentTime;
        }
    }

    void gHelper::findCorners(gHelperData &data)
    {
        findCorner(data,leftCorner,data.sensors.getSensor(-1));
        findCorner(data,rightCorner,data.sensors.getSensor(1));
    }

    void gHelper::showCorner(gHelperData &data, gSmartTurningCornerData &corner, REAL timeout) {
        if (corner.exist) {
            REAL timeout = data.speedFactor * sg_helperShowCornersTimeout;
            bool isClose = smartTurning->isClose(corner.currentPos, sg_helperShowCornersBoundary);


            if (isClose)// && data.sensors.getSensor(-1)->type != gSENSOR_RIM)
            {
                debugLine(1,.5,0,sg_helperShowCornersHeight,timeout,corner.currentPos,corner.currentPos);
            }
        }
    }

    void gHelper::showCorners(gHelperData &data) {
        if (!aliveCheck()) { return; }
        REAL timeout = data.speedFactor * sg_helperShowCornersTimeout;
        findCorners(data);
        showCorner(data,leftCorner,timeout);
        showCorner(data,rightCorner,timeout);
    }

    bool gHelper::drivingStraight() {
        return ((fabs(ownerDir->x) == 1 || fabs(ownerDir->y) == 1));
    }

    void gHelper::showHit(gHelperData &data)
    {
        if (!aliveCheck()) { return; }
        if (!drivingStraight()) {
            return;
        }

        bool wallClose = data.sensors.getSensor(0)->hit < data.turnSpeedFactor * sg_showHitDataRange;

        REAL timeout = data.speedFactor * sg_showHitDataTimeout;

        if (wallClose)
        {
            eCoord frontBeforeHit = data.sensors.getSensor(0)->before_hit;
            debugLine(1,.5,0,sg_showHitDataHeightFront,data.speedFactor,(*ownerPos),frontBeforeHit);
            showHitDebugLines(frontBeforeHit, owner_->Direction(), timeout, data, sg_showHitDataRecursion, -1);
            showHitDebugLines(frontBeforeHit, owner_->Direction(), timeout, data, sg_showHitDataRecursion, 1);
        }
    }

    void gHelper::showHitDebugLines(eCoord pos, eCoord dir, REAL timeout, gHelperData &data, int recursion, int sensorDir)
    {

        if (recursion <= 0)
        {
            return;
        }
        recursion--;
        eCoord newDir = dir.Turn(eCoord(0, sensorDir * -1));
        gSensor *sensor = data.sensors.getSensor(pos, sensorDir);
        eCoord hitPos = sensor->before_hit;
        REAL hitDistance = sensor->hit;

        bool open = hitDistance > data.turnSpeedFactor * sg_showHitDataFreeRange;

        if (open)
        {
            debugLine(0, 1, 0, sg_showHitDataHeightFront, timeout, pos, hitPos);
        }
        else
        {
            debugLine(1, 0, 0, sg_showHitDataHeightFront, timeout, pos, hitPos);
        }
        showHitDebugLines(hitPos, newDir, timeout, data, recursion, sensorDir);
    }

    gHelper& gHelper::Get( gCycle * cycle )
    {
        tASSERT( cycle );

        // create
        if ( cycle->helper_.get() == 0 )
            cycle->helper_.reset( new gHelper( cycle ) );

        return *cycle->helper_;

    }

    bool gHelper::aliveCheck() {
        return owner_->Alive();
    }

    void gHelper::Activate()
    {
        //con << "AI Created? " << bool(aiPlayer == nullptr) << "\n";
        if (sg_helperAI)
        {
            if (!aiCreated)
            {
                con << "CREATING AI!! \n";
                gAIPlayer *aiPlayer = new gAIPlayer(owner_);
                // aiPlayer->character = BestIQ(1000);
                sg_AIReferences.Add(aiPlayer);
                aiCreated = true;
            }
            else
            {
                aiPlayer->Timestep(se_GameTime());
            }
        }
        else
        {
            aiPlayer = nullptr;
        }

        if (!aliveCheck()) { return; }
        owner_->localCurrentTime = se_GameTime();
        REAL speedFactor = (1/(*ownerSpeed));
        REAL turnSpeedFactor = ((*ownerSpeed) * ownerTurnDelay);
        REAL turnSpeedFactorPercent = (1/turnSpeedFactor);
        REAL turnDistance = (turnSpeedFactor/100);

        gHelperSensorsData sensors(owner_);
        gHelperData data(sensors, speedFactor, turnSpeedFactor,turnSpeedFactorPercent,turnDistance,0,0,0);
        data_stored = &data;

        enemies.detectEnemies();

        if (sg_helperSmartTurning) {
            smartTurning->Activate(data);
        }

        if (sg_pathHelper) {
            pathHelper->Activate(data);
        }

        if (sg_helperEnemyTracers)
        {
            enemyTracers(data, sg_helperEnemyTracersDetectionRange, sg_helperEnemyTracersTimeout);
        }

        if (sg_helperDetectCut)
        {
            detectCut(data, sg_helperDetectCutDetectionRange);
        }

        if (sg_helperShowHit) {
            showHit(data);
        }

        if (sg_helperShowTail)
        {
            showTail(data);
        }

        if (sg_helperShowTailTracer){
            showTailTracer(data);
        }

        if (sg_helperShowTailPath) {
            showTailPath(data);
        }

        if (sg_helperShowEnemyTail) {
            showEnemyTail(data);
        }
        if (sg_helperShowCorners)
        {
            showCorners(data);
        }

        owner_->justCreated = false;
    }


gHelper::~gHelper()
{
}
