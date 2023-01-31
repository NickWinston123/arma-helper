// HELPER
#include "gHelper.h"
#include "nConfig.h"
#include "../gSensor.h"
#include "../gWall.h"
#include "rRender.h"
#include "../gAIBase.h"
#include "eDebugLine.h"
#include "tDirectories.h"

#include "gHelperHud.h"

#define LEFT -1
#define RIGHT 1
#define FRONT 0
#define BOTH 2

extern REAL sg_cycleBrakeRefill;
extern REAL sg_cycleBrakeDeplete;
extern void sg_RubberValues( ePlayerNetID const * player, REAL speed, REAL & max, REAL & effectiveness );
extern REAL sg_brakeCycle;
extern REAL sg_cycleBrakeDeplete;

namespace helperConfig {
bool sg_helper = false;
static tConfItem<bool> sg_helperConf("HELPER", sg_helper);

REAL sg_helperBrightness = 1;
static tConfItem<REAL> sg_helperBrightnessConf("HELPER_BRIGHTESS", sg_helperBrightness);

REAL sg_helperSensorRange = 1000;
static tConfItem<REAL> sg_helperSensorRangeConf("HELPER_SENSOR_RANGE", sg_helperSensorRange);
bool sg_helperSensorLightUsageMode = false;
static tConfItem<bool> sg_helperSensorLightUsageModeConf("HELPER_SENSOR_LIGHT_USAGE_MODE", sg_helperSensorLightUsageMode);
bool sg_helperSensorDiagonalMode = false;
static tConfItem<bool> sg_helperSensorDiagonalModeConf("HELPER_SENSOR_DIAGONAL_MODE", sg_helperSensorDiagonalMode);

bool sg_helperDebug = false;
static tConfItem<bool> sg_helperDebugConf("HELPER_DEBUG", sg_helperDebug);
tString sg_helperDebugIgnoreList = tString("");
static tConfItem<tString> sg_helperDebugIgnoreListConf("HELPER_DEBUG_IGNORE_LIST", sg_helperDebugIgnoreList);
REAL sg_helperDebugDelay = 0.15;
static tConfItem<REAL> sg_helperDebugDelayConf("HELPER_DEBUG_DELAY", sg_helperDebugDelay);
bool sg_helperDebugTimeStamp = true;
static tConfItem<bool> sg_helperDebugTimeStampConf("HELPER_DEBUG_TIMESTAMP", sg_helperDebugTimeStamp);

bool sg_helperAI = false;
static tConfItem<bool> sg_helperAIc("HELPER_AI", sg_helperAI);
REAL sg_helperAITime = 0;
static tConfItem<REAL> sg_helperAITimec("HELPER_AI_TIME", sg_helperAITime);

bool sg_helperSmartTurning = false;
static tConfItem<bool> sg_helperSmartTurningConf("HELPER_SMART_TURNING", sg_helperSmartTurning);

bool sg_helperSmartTurningFrontBot = false;
static tConfItem<bool> sg_helperSmartTurningFrontBotConf("HELPER_SMART_TURNING_FRONT_BOT", sg_helperSmartTurningFrontBot);
REAL sg_helperSmartTurningFrontBotThinkRange = 1;
static tConfItem<REAL> sg_helperSmartTurningFrontBotThinkRangeConf("HELPER_SMART_TURNING_FRONT_BOT_THINK_RANGE", sg_helperSmartTurningFrontBotThinkRange);
bool sg_helperSmartTurningFrontBotTurnOnce = true;
static tConfItem<bool> sg_helperSmartTurningFrontBotTurnOnceConf("HELPER_SMART_TURNING_FRONT_BOT_TURN_ONCE", sg_helperSmartTurningFrontBotTurnOnce);

REAL sg_helperSmartTurningFrontBotActivationRubber = 0.98;
static tConfItem<REAL> sg_helperSmartTurningFrontBotActivationRubberConf("HELPER_SMART_TURNING_FRONT_BOT_ACTIVATION_RUBBER", sg_helperSmartTurningFrontBotActivationRubber);
REAL sg_helperSmartTurningFrontBotActivationSpace = 1;
static tConfItem<REAL> sg_helperSmartTurningFrontBotActivationSpaceConf("HELPER_SMART_TURNING_FRONT_BOT_ACTIVATION_SPACE", sg_helperSmartTurningFrontBotActivationSpace);
REAL sg_helperSmartTurningFrontBotDisableTime = 0;
static tConfItem<REAL> sg_helperSmartTurningFrontBotDisableTimeConf("HELPER_SMART_TURNING_FRONT_BOT_DISABLE_TIME", sg_helperSmartTurningFrontBotDisableTime);

bool sg_helperSmartTurningClosedIn = true;
static tConfItem<bool> sg_helperSmartTurningClosedInConf("HELPER_SMART_TURNING_CLOSEDIN", sg_helperSmartTurningClosedIn);


bool sg_helperSmartTurningAutoTrace = false;
static tConfItem<bool> sg_helperSmartTurningAutoTraceConf("HELPER_SMART_TURNING_AUTO_TRACE", sg_helperSmartTurningAutoTrace);

REAL sg_helperSmartTurningAutoTraceDistance = 1;
static tConfItem<REAL> sg_helperSmartTurningAutoTraceDistanceConf("HELPER_SMART_TURNING_AUTO_TRACE_DISTANCE", sg_helperSmartTurningAutoTraceDistance);

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

bool sg_helperAutoBrake = false;
static tConfItem<bool> sg_helperAutoBrakeConf("HELPER_AUTO_BRAKE", sg_helperAutoBrake);
bool sg_helperAutoBrakeDeplete = false;
static tConfItem<bool> sg_helperAutoBrakeDepleteConf("HELPER_AUTO_BRAKE_DEPLETE", sg_helperAutoBrakeDeplete);
REAL sg_helperAutoBrakeMin = 0;
static tConfItem<REAL> sg_helperAutoBrakeMinConf("HELPER_AUTO_BRAKE_MIN", sg_helperAutoBrakeMin);
REAL sg_helperAutoBrakeMax = 2;
static tConfItem<REAL> sg_helperAutoBrakeMaxConf("HELPER_AUTO_BRAKE_MAX", sg_helperAutoBrakeMax);

bool sg_helperSmartTurningFollowTail = false;
static tConfItem<bool> sg_helperSmartTurningFollowTailConf("HELPER_SMART_TURNING_FOLLOW_TAIL", sg_helperSmartTurningFollowTail);
REAL sg_helperSmartTurningFollowTailDelayMult = 1;
static tConfItem<REAL> sg_helperSmartTurningFollowTailDelayMultConf("HELPER_SMART_TURNING_FOLLOW_TAIL_DELAY_MULT", sg_helperSmartTurningFollowTailDelayMult);
REAL sg_helperSmartTurningFollowTailFreeSpaceMult = 1;
static tConfItem<REAL> sg_helperSmartTurningFollowTailFreeSpaceMultConf("HELPER_SMART_TURNING_FOLLOW_TAIL_FREE_SPACE_MULT", sg_helperSmartTurningFollowTailFreeSpaceMult);

bool sg_tailHelper = false;
static tConfItem<bool> sg_tailHelperC("HELPER_TAIL", sg_tailHelper);
REAL sg_tailHelperBrightness = 1;
static tConfItem<REAL> sg_tailHelperBrightnessC("HELPER_TAIL_BRIGHTNESS", sg_tailHelperBrightness);
REAL sg_tailHelperDelay = 0;
static tConfItem<REAL> sg_tailHelperDelayC("HELPER_TAIL_DELAY", sg_tailHelperDelay);
REAL sg_tailHelperUpdateTime = 1;
static tConfItem<REAL> sg_tailHelperUpdateTimeC("HELPER_TAIL_UPDATE_TIME", sg_tailHelperUpdateTime);

bool sg_pathHelper = false;
static tConfItem<bool> sg_pathHelperC("HELPER_PATH", sg_pathHelper);
bool sg_pathHelperRenderPath = false;
static tConfItem<bool> sg_pathHelperRenderPathC("HELPER_PATH_RENDER", sg_pathHelperRenderPath);
bool sg_pathHelperShowTurn = false;
static tConfItem<bool> sg_pathHelperShowTurnC("HELPER_PATH_RENDER_TURN", sg_pathHelperShowTurn);
bool sg_pathHelperShowTurnAct = false;
static tConfItem<bool> sg_pathHelperShowTurnActC("HELPER_PATH_RENDER_TURN_ACT", sg_pathHelperShowTurnAct);
REAL sg_pathHelperShowTurnAhead = 0;
static tConfItem<REAL> sg_pathHelperShowTurnAheadC("HELPER_PATH_RENDER_TURN_AHEAD", sg_pathHelperShowTurnAhead);
int sg_pathHelperMode = 0;
static tConfItem<int> sg_pathHelperModeC("HELPER_PATH_MODE", sg_pathHelperMode);
REAL sg_pathHelperAutoCloseDistance = 150;
static tConfItem<REAL> sg_pathHelperAutoCloseDistanceC("HELPER_PATH_AUTO_CLOSE_DISTANCE", sg_pathHelperAutoCloseDistance);
REAL sg_pathHelperUpdateTime = -1000;
static tConfItem<REAL> sg_pathHelperUpdateTimeC("HELPER_PATH_UPDATE_TIME", sg_pathHelperUpdateTime);
REAL se_pathHeight = 1;
static tConfItem<REAL> se_pathHeightC("HELPER_PATH_HEIGHT", se_pathHeight);
REAL se_pathBrightness = 1;
static tConfItem<REAL> se_pathBrightnessC("HELPER_PATH_BRIGHTNESS", se_pathBrightness);
REAL sg_pathHelperUpdateDistance = 1;
static tConfItem<REAL> sg_pathHelperUpdateDistanceC("HELPER_PATH_UPDATE_DISTANCE", sg_pathHelperUpdateDistance);

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

bool sg_helperDetectCut = false;
static tConfItem<bool> sg_helperDetectCutConf("HELPER_DETECT_CUT", sg_helperDetectCut);
REAL sg_helperDetectCutDetectionRange = 150;
static tConfItem<REAL> sg_helperDetectCutDetectionRangeConf("HELPER_DETECT_CUT_DETECTION_RANGE", sg_helperDetectCutDetectionRange);
REAL sg_helperDetectCutTimeout = .001;
static tConfItem<REAL> sg_helperDetectCutTimeoutConf("HELPER_DETECT_CUT_TIMEOUT", sg_helperDetectCutTimeout);
REAL sg_helperDetectCutHeight = 0;
static tConfItem<REAL> sg_helperDetectCutHeightConf("HELPER_DETECT_CUT_HEIGHT", sg_helperDetectCutHeight);
REAL sg_helperDetectCutReact = .2; // .005 ?
static tConfItem<REAL> sg_helperDetectCutReactConf("HELPER_DETECT_CUT_REACT", sg_helperDetectCutReact);
bool sg_helperEnemyTracers = false;

static tConfItem<bool> sg_helperEnemyTracersConf("HELPER_SHOW_ENEMY_TRACERS", sg_helperEnemyTracers);
REAL sg_helperEnemyTracersHeight = 0;
static tConfItem<REAL> sg_helperEnemyTracersHeightConf("HELPER_SHOW_ENEMY_TRACERS_HEIGHT", sg_helperEnemyTracersHeight);
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
REAL sg_helperEnemyTracersBrightness = 1;
static tConfItem<REAL> sg_helperEnemyTracersBrightnessConf("HELPER_SHOW_ENEMY_TRACERS_BRIGHTNESS", sg_helperEnemyTracersBrightness);

bool sg_helperShowTail = false;
static tConfItem<bool> sg_helperShowTailConf("HELPER_SHOW_TAIL", sg_helperShowTail);
bool sg_helperShowTailPath = false;
static tConfItem<bool> sg_helperShowTailPathConf("HELPER_SHOW_TAIL_PATH", sg_helperShowTailPath);
REAL sg_helperShowTailHeight = 1;
static tConfItem<REAL> sg_helperShowTailHeightConf("HELPER_SHOW_TAIL_HEIGHT", sg_helperShowTailHeight);
REAL sg_helperShowTailPassthrough = 0.5;
static tConfItem<REAL> sg_helperShowTailPassthroughConf("HELPER_SHOW_TAIL_PASSTHROUGH", sg_helperShowTailPassthrough);
REAL sg_helperShowTailTimeout = 1;
static tConfItem<REAL> sg_helperShowTailTimeoutConf("HELPER_SHOW_TAIL_TIMEOUT", sg_helperShowTailTimeout);

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

bool sg_helperShowTailTracer = false;
static tConfItem<bool> sg_helperShowTailTracerConf("HELPER_SHOW_TAIL_TRACER", sg_helperShowTailTracer);
REAL sg_helperShowTailTracerHeight = 1;
static tConfItem<REAL> sg_helperShowTailTracerHeightConf("HELPER_SHOW_TAIL_TRACER_HEIGHT", sg_helperShowTailTracerHeight);
REAL sg_helperShowTailTracerTimeoutMult = 1;
static tConfItem<REAL> sg_helperShowTailTracerTimeoutMultConf("HELPER_SHOW_TAIL_TRACER_TIMEOUT_MULT", sg_helperShowTailTracerTimeoutMult);
REAL sg_helperShowTailTracerDistanceMult = 10;
static tConfItem<REAL> sg_helperShowTailTracerDistanceMultConf("HELPER_SHOW_TAIL_TRACER_DISTANCE_MULT", sg_helperShowTailTracerDistanceMult);

bool sg_helperShowHit = false;
static tConfItem<bool> sg_helperShowHitConf("HELPER_SHOW_HIT", sg_helperShowHit);
REAL sg_showHitDataHeight = 1;
static tConfItem<REAL> sg_showHitDataHeightConf("HELPER_SHOW_HIT_HEIGHT", sg_showHitDataHeight);
REAL sg_showHitDataHeightFront = 1;
static tConfItem<REAL> sg_showHitDataHeightFrontConf("HELPER_SHOW_HIT_HEIGHT_FRONT", sg_showHitDataHeightFront);
REAL sg_showHitDataHeightSides = 2;
static tConfItem<REAL> sg_showHitDataHeightSidesConf("HELPER_SHOW_HIT_HEIGHT_SIDES", sg_showHitDataHeightSides);
REAL sg_showHitDataRange = 1;
static tConfItem<REAL> sg_showHitDataRangeConf("HELPER_SHOW_HIT_RANGE", sg_showHitDataRange);
REAL sg_showHitDataFreeRange = 1;
static tConfItem<REAL> sg_showHitDataFreeRangeConf("HELPER_SHOW_HIT_OPEN_RANGE", sg_showHitDataFreeRange);
int sg_showHitDataRecursion = 1;
static tConfItem<int> sg_showHitDataRecursionConf("HELPER_SHOW_HIT_RECURSION", sg_showHitDataRecursion);
REAL sg_showHitDataTimeout = 1;
static tConfItem<REAL> sg_showHitDataTimeoutConf("HELPER_SHOW_HIT_TIMEOUT", sg_showHitDataTimeout);
bool sg_helperShowHitStartAtHitPos = true;
static tConfItem<bool> sg_helperShowHitStartAtHitPosConf("HELPER_SHOW_HIT_START_AT_HIT_POS", sg_helperShowHitStartAtHitPos);

bool sg_helperCurrentTimeLocal = true; // Determines if the helper uses its own internal clock or the games to sync actions
static tConfItem<bool> sg_helperCurrentTimeLocalConf("HELPER_CURRENT_TIME_LOCAL", sg_helperCurrentTimeLocal);

}
using namespace helperConfig;
//HUD ITEMS
gHelperHudItemRef<bool> sg_helperSmartTurningH("Smart Turning",sg_helperSmartTurning);
gHelperHudItemRef<bool> sg_pathHelperH("Path Helper",sg_pathHelper);

gHelperHudItem<eCoord> ownerPosH("Owner Pos",eCoord(0,0));
gHelperHudItem<eCoord> ownerDirH("Owner Dir",eCoord(0,0));

gHelperHudItem<eCoord> tailPosH("Tail Pos",eCoord(0,0));
gHelperHudItem<eCoord> tailDirH("Tail Dir",eCoord(0,0));

gHelperHudItemRef<bool> sg_helperDetectCutH("Detect Cut",sg_helperDetectCut);
gHelperHudItem<tColoredString> detectCutdebugH("Detect Cut Debug",tColoredString("None"),"Detect Cut");


gHelperHudItem<tColoredString> closestEnemyH("Closest Enemy",tColoredString("None"), "Detect Cut");
gHelperHudItem<tColoredString> cutTurnDirectionH("Cut Turn Dir",tColoredString("0xdd0000None"), "Detect Cut");

gHelperHudItemRef<bool> sg_helperShowHitH("Show Hit",sg_helperShowHit);
gHelperHudItem<REAL> sg_helperShowHitFrontDistH("Show Hit Front Dist",1000,"Show Hit");

gHelperHudItem<REAL> sg_helperActivateTimeH("Acivate Time",0);


void debugLine(REAL R, REAL G, REAL B, REAL height, REAL timeout,
               eCoord start, eCoord end, REAL brightness = 1)
{
    if (timeout >= 6)
    {
        timeout = 1;
    }
    REAL startHeight = height;
    if (start == end)
    {
        startHeight = 0;
    }
    eDebugLine::SetTimeout(timeout);
    eDebugLine::SetColor(R * sg_helperBrightness * brightness, G * sg_helperBrightness * brightness, B * sg_helperBrightness * brightness);
    eDebugLine::Draw(start, startHeight, end, height);
}

// REAL sg_cycleTurnSpeedFactor = 0.95
// void gHelper::Path_To_Tail(){
//     eCoord tailPos = owner_->tailPos; // x,y position, example: tailPos.x
//     eCoord tailDir = owner_->tailDir; // Direction moving relative to grid, possible variation: (depends on where you spawn) (0,-1) = FORWARD, (0,1) = BACK WARD, (1,-1) = LEFT (-1,-1) = RIGHT
//     eCoord ownerPos = owner_->pos;
//     eCoord ownerDir = owner_->dir; // Same rules apply as tail dir ( and both tail dir and tail pos start as the same)

//     REAL ownerSpeed = owner_->verletSpeed;
//     // SPEED AFTER TURN = ownerSpeed * sg_cycleTurnSpeedFactor, Example if speed is 30, and a turn is made and sg_cycleTurnSpeedFactor is equal to .95 the new speed is 28.5
//     /*
//     EXAMPLE OF HOW TO TURN LEFT ON A eCoord
//     owner_->Direction().Turn(eCoord(0, 1)
//     TURN RIGHT:
//     owner_->Direction().Turn(eCoord(0, -1)
//     */
//     REAL turnDelay = owner_->GetTurnDelay(); // the delay between turns in seconds
// }

void DebugLog(std::string message)
{
    std::ofstream o;
    if (tDirectories::Var().Open(o, "helperdebug.txt", std::ios::app))
    {
        o << message << std::endl;
    }
    else
    {
        con << tOutput("Log Error");
    }
    o.close();
}

gTailHelper::gTailHelper(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner)
{
        ownerPos = &owner_->pos;
        ownerDir = &owner_->dir;
        tailPos = &owner_->tailPos;
        tailDir = &owner_->tailDir;
        ownerSpeed = &owner_->verletSpeed_;
        turnDelay = owner_->GetTurnDelay();
        updateTime = -100;
}

// std::vector<eCoord> gTailHelper::getPathToTail() {
//     std::vector<eCoord> path;

//     REAL delay = turnDelay + sg_tailHelperDelay;
//     eCoord direction = (*tailPos - *ownerPos);  // Initial direction towards tail
//     double dist = direction.Norm();
//     int numSegments = (int)(dist / delay);

//     // Get the angle between the tail direction and the initial direction
//     double tailAngle = atan2(tailDir->y, tailDir->x);
//     double initAngle = atan2(direction.y, direction.x);
//     double deltaAngle = tailAngle - initAngle;

//     // Divide the total angle by the number of segments to get the angle for each segment
//     double segmentAngle = deltaAngle / numSegments;

//     // Add the first point to the path
//     path.push_back(*ownerPos);
//     eCoord currentPos = *ownerPos;

//     // Create a path of turns to the tail
//     for (int i = 1; i < numSegments; i++) {
//         double currentAngle = atan2(direction.y, direction.x);
//         currentPos += direction*delay;
//         path.push_back(currentPos);
//         direction = eCoord(cos(currentAngle + segmentAngle), sin(currentAngle + segmentAngle));  // rotate direction by the segment angle
//     }

//     // Check if last segment is shorter than delay
//     if ((*tailPos - currentPos).Norm() > 1e-6) {
//         // Add the last point to the path
//         path.push_back(*tailPos);
//     }
//     return path;
// }

std::vector<eCoord> gTailHelper::getPathToTail() {
    std::vector<eCoord> path;

    eCoord direction = (*tailPos - *ownerPos);  // Initial direction towards tail
    double dist = direction.Norm();
    int numSegments = (int)(dist / sg_tailHelperDelay);

    // align initial direction with the grid
    double angle = atan2(direction.y, direction.x);
    angle = round(angle / (M_PI / 2)) * (M_PI / 2);
    direction = eCoord(cos(angle), sin(angle));

    direction.Normalize();

    // Add the first point to the path
    path.push_back(*ownerPos);
    eCoord currentPos = *ownerPos;

    // Create a path of 90-degree turns to the tail
    for (int i = 1; i < numSegments; i++) {
        double segmentAngle = atan2(direction.y, direction.x);
        currentPos += direction*sg_tailHelperDelay;
        path.push_back(currentPos);
        direction = eCoord(cos(segmentAngle + (i % 2 == 1 ? M_PI / 2 : -M_PI / 2)), sin(segmentAngle + (i % 2 == 1 ? M_PI / 2 : -M_PI / 2)));  // rotate direction by 90 degrees
    }

    // Check if last segment is shorter than delay
    if ((*tailPos - currentPos).Norm() > 1e-6) {
        // Add the last point to the path
        path.push_back(*tailPos);
    }
    return path;
}


void gTailHelper::Activate(gHelperData &data) {
    if (!owner_->tailMoving)
        return;

    if (updateTime <= helper_->CurrentTime() || path.size() < 1) {
        path = getPathToTail();
        updateTime = helper_->CurrentTime() + sg_tailHelperUpdateTime;
    }

    if (path.size() < 1)
        return;

    eCoord lastPos = *ownerPos;
    for (int i = 0; i < path.size(); i++) {
        debugLine(1, 0, 0, 1, data.speedFactorF(), lastPos, path[i], sg_tailHelperBrightness);
        lastPos = path[i];
    }
}


gTailHelper &gTailHelper::Get(gHelper * helper, gCycle * owner)
{
    tASSERT(owner);
    // create
    if (helper->tailHelper.get() == 0)
        helper->tailHelper.reset(new gTailHelper(helper, owner));
    return *helper->tailHelper;
}

gPathHelper::gPathHelper(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner),
      lastPath(helper_->CurrentTime() - 100),
      lastTime(helper_->CurrentTime()),
      nextTime(0),
      pathInvalid(true)
{
// Initialize any other member variables here
}

bool gPathHelper::targetExist()
{
    return target != eCoord(0, 0) && targetCurrentFace_;
}

gPathHelper &gPathHelper::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(owner);

    // create
    if (helper->pathHelper.get() == 0)
        helper->pathHelper.reset(new gPathHelper(helper, owner));

    return *helper->pathHelper;
}

REAL eCoordDistance(const eCoord &p1, const eCoord &p2)
{
    eCoord difference = p1 - p2;
    return difference.Norm();
}

bool gPathHelper::cornerMode(gHelperData data)
{
    helper_->findCorners(data);

    bool left = helper_->leftCorner.exist;
    bool right = helper_->rightCorner.exist;
    bool both = left && right;

    eCoord position;
    if (left && !right)
    {
        position = helper_->leftCorner.currentPos;
    }

    if (!left && right)
    {
        position = helper_->rightCorner.currentPos;
    }

    if (both)
    {
        // Calculate the distance from the owner's position to the left corner
        REAL leftDistance = eCoordDistance(owner_->Position(), helper_->leftCorner.currentPos);
        // Calculate the distance from the owner's position to the right corner
        REAL rightDistance = eCoordDistance(owner_->Position(), helper_->rightCorner.currentPos);
        position = (leftDistance < rightDistance) ? helper_->leftCorner.currentPos : helper_->rightCorner.currentPos;
    }

    target = position;

    if (!DistanceCheck(data))
    {
        return false;
    }
    owner_->FindCurrentFace();
    targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(target);

    return true;
    }

    bool gPathHelper::tailMode(gHelperData data)
    {
    if (owner_->tailMoving)
    {

        target = owner_->tailPos;
        if (!DistanceCheck(data))
        {
        return false;
        }
        owner_->FindCurrentFace();
        targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(owner_->tailPos);
        return true;
    }
    return false;
}

bool gPathHelper::enemyMode(gHelperData data)
{
gCycle *enemy = helper_->enemies.closestEnemy;

if (helper_->enemies.exist(enemy))
{
    target = enemy->Position();
    if (!DistanceCheck(data))
    {
    return false;
    }
    owner_->FindCurrentFace();
    targetCurrentFace_ = enemy->CurrentFace();
    return true;
}
return false;
}

bool gPathHelper::autoMode(gHelperData data)
{
gCycle *enemy = helper_->enemies.closestEnemy;

bool isClose = helper_->enemies.exist(enemy) && helper_->smartTurning->isClose(enemy->Position(), sg_pathHelperAutoCloseDistance + data.turnSpeedFactorF());
if (isClose)
{
    target = enemy->Position();
    if (!DistanceCheck(data))
    {
    return false;
    }
    owner_->FindCurrentFace();
    targetCurrentFace_ = enemy->CurrentFace();
    return true;
}
else if (owner_->tailMoving)
{
    target = owner_->tailPos;
    if (!DistanceCheck(data))
    {
    return false;
    }
    owner_->FindCurrentFace();
    targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(target);
    return true;
}

return false;
}


bool gPathHelper::UpdateTimeCheck(gHelperData &data)
{
return lastPath < helper_->CurrentTime() - sg_pathHelperUpdateTime;
}
void gPathHelper::RenderPath(gHelperData &data)
{
    if (!path_.Valid() || path_.positions.Len() == 0)
        return;

    eCoord last_c;
    eCoord owner_pos = owner_->pos; // get the current position of the owner

    debugLine(1, 1, 0, 0, data.speedFactorF(), owner_pos, path_.positions(0) + path_.offsets(0), se_pathBrightness);

    for (int i = path_.positions.Len() - 1; i >= 0; i--)
    {
        eCoord c = path_.positions(i) + path_.offsets(i);
        if (i != path_.positions.Len() - 1)
            debugLine(1, 0, 0, se_pathHeight, data.speedFactorF(), last_c, c, se_pathBrightness);
        last_c = c;
    }

    if (path_.current >= 0 && path_.positions.Len() > 0)
    {
        eCoord c = path_.CurrentPosition();
        debugLine(1, 1, 0, se_pathHeight, data.speedFactorF(), c, c, 1);
        debugLine(1, 1, 0, (se_pathHeight * 2), data.speedFactorF(), c, c, se_pathBrightness);
    }
}


void gPathHelper::RenderTurn(gHelperData &data)
{
if (!path_.Valid())
{
    return;
}

for (int z = 10; z >= 0; z--)
    path_.Proceed();

bool goon = path_.Proceed();
bool nogood = false;

do
{
    if (goon)
    goon = path_.GoBack();
    else
    goon = true;

    eCoord pos = path_.CurrentPosition() + path_.CurrentOffset() * 0.1f;
    eCoord opos = owner_->Position();
    eCoord odir = pos - opos;

    eCoord intermediate = opos + owner_->Direction() * eCoord::F(odir, owner_->Direction());

    gSensor p(owner_, opos, intermediate - opos);
    p.detect(1.1f);
    nogood = (p.hit <= .999999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);

    if (!nogood)
    {
    gSensor p(owner_, intermediate, pos - intermediate);
    p.detect(1);
    nogood = (p.hit <= .99999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);
    }

} while (goon && nogood);

if (goon)
{
    // now we have found our next goal. Try to get there.
    eCoord pos = owner_->Position();
    eCoord target = path_.CurrentPosition();

    // look how far ahead the target is:
    REAL ahead = eCoord::F(target - pos, owner_->Direction()) + eCoord::F(path_.CurrentOffset(), owner_->Direction());

    HelperDebug::Debug("RenderTurn", "AHEAD = ", &ahead);
    if (ahead > sg_pathHelperShowTurnAhead)
    { // it is still before us. just wait a while.
        // mindist = ahead;
    }
    else
    { // we have passed it. Make a turn towards it.
    int lr;
    REAL side = (target - pos) * owner_->Direction();

    if (!((side > 0 && data.sensors.getSensor(LEFT)->hit < 3) || (side < 0 && data.sensors.getSensor(RIGHT)->hit < 3)) && (fabs(side) > 3 || ahead < -10))
    {
        lr += (side > 0 ? 1 : -1);
    }

    lr *= -1;
    if (lr == RIGHT)
    {
        debugLine(.2, 1, 0, 3, data.speedFactorF() * 3, owner_->Position(), data.sensors.getSensor(RIGHT)->before_hit, 1);
        if (sg_pathHelperShowTurnAct)
            helper_->smartTurning->makeTurnIfPossible(data, RIGHT, 1);
    }
    else if (lr == LEFT)
    {
        debugLine(.2, 1, 0, 3, data.speedFactorF() * 3, owner_->Position(), data.sensors.getSensor(LEFT)->before_hit, 1);
        if (sg_pathHelperShowTurnAct)
            helper_->smartTurning->makeTurnIfPossible(data, LEFT, 1);
    }
    }
}
}

bool gPathHelper::DistanceCheck(gHelperData &data)
{
eCoord difference = target - lastPos;
REAL distance = difference.Norm();
// con << distance << " > " << sg_pathHelperUpdateDistance << " \n";
return distance >= sg_pathHelperUpdateDistance;
}

void gPathHelper::FindPath(gHelperData &data)
{
if (!targetExist())
{
    return;
}

if (targetCurrentFace_)
{
    // owner_->FindCurrentFace();
    eHalfEdge::FindPath(owner_->Position(), owner_->CurrentFace(),
                        target, targetCurrentFace_,
                        owner_,
                        path_);
    // con << "Found updated path & " << lastPath << "\n";
    lastPath = helper_->CurrentTime();
    lastPos = target;
    // con << "Updated path\n";
}

if (!path_.Valid())
{
    targetCurrentFace_ = NULL;
    lastPath = -100;
    return;
}
}

void gPathHelper::Activate(gHelperData &orig_data)
{

if (!helper_->aliveCheck())
    return;

if (sg_pathHelperRenderPath)
    RenderPath(orig_data);

if (sg_pathHelperShowTurn)
    RenderTurn(orig_data);

if (!UpdateTimeCheck(orig_data))
    return;

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
case 3:
    success = cornerMode(data);
    break;
default:
    // do nothing
    return;
}

if (!success)
    return;

    FindPath(data);
}

int noTurns = 0;
class gHelperEmergencyTurn
{
gHelperEmergencyTurn();

public:
class Sensor : public gSensor
{
public:
    Sensor(gCycle *o, const eCoord &start, const eCoord &d)
        : gSensor(o, start, d), hitOwner_(0), hitTime_(0), hitDistance_(o->MaxWallsLength()), lrSuggestion_(0), windingNumber_(0)
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
        // unless it is an enemy, follow his wall instead (uncomment for a nasty cowardy campbot)
        // lrSuggestion *= -1;
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

        // gCycleChatBot & enemyChatBot = Get( hitOwner_ );

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
        if (hitTime_ > hitOwner_->LastTime() && hitOwner_ != owned)
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

    gCycle *hitOwner_;   // the owner of the hit wall
    REAL hitTime_;       // the time the hit wall was built at
    REAL hitDistance_;   // the distance of the wall to the cycle that built it
    short lrSuggestion_; // sensor's oppinon on whether moving to the left or right of the hit wall is recommended (-1 for left, +1 for right)
    int windingNumber_;  // the number of turns (with sign) the cycle has taken
};

gHelperEmergencyTurn(gCycle *owner, gHelper *helper) : owner_(owner),
                                                        helper_(helper)
{
}

// determines the distance between two sensors; the size should give the likelyhood
// to survive if you pass through a gap between the two selected walls
REAL Distance(Sensor const &a, Sensor const &b)
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
    REAL ret =
        a.hitDistance_ + b.hitDistance_;

    if (rim)
    {
        ret = bigDistance * .001 + ret * .01 + (a.before_hit - b.before_hit).Norm();

        // we love going between the rim and enemies
        if (!self)
            ret = bigDistance * 2;
    }

    // minimal factor should be 1, this path should never return something smaller than the
    // paths where only one cycle's walls are hit
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
    return (fabsf(a.hitDistance_ - b.hitDistance_) + .25 * bigDistance) * selfHatred;
    }

    else if ((-2 * a.lr * (a.windingNumber_ - b.windingNumber_) > owner_->Grid()->WindingNumber()))
    {
    // this looks like a way out to me
    return fabsf(a.hitDistance_ - b.hitDistance_) * 10 * selfHatred;
    }
    else
    {
    // well, the longer the wall segment between the two points, the better.
    return fabsf(a.hitDistance_ - b.hitDistance_) * selfHatred;
    }

    // default: hit distance
    return (a.before_hit - b.before_hit).Norm() * selfHatred;
}

int ActToTurn(uActionPlayer *action)
{
    return action == &gCycle::se_turnLeft ? -1 : 1;
}

template <class T>
void BotDebug(std::string description, T value = "")
{
    HelperDebug::Debug("gHelperEmergencyTurn", description, &value);
}

void BotDebug(std::string description)
{
    HelperDebug::Debug("gHelperEmergencyTurn", description, "");
}

// does the main thinking
gTurnData * getTurn()
{
    gTurnData * turn = (new gTurnData(false));

    REAL lookahead = 100; // seconds to plan ahead
    REAL minstep = 0;     // minimum timestep between thoughts in seconds

    // cylce data
    REAL speed = owner_->Speed();
    eCoord dir = owner_->Direction();
    eCoord pos = owner_->Position();

    REAL range = speed * lookahead;
    eCoord scanDir = dir * range;

    REAL frontFactor = 1;

    Sensor front(owner_, pos, scanDir);
    front.detect(frontFactor);

    if (!front.ehit)
    {
        return turn;
    }

    // owner_->enemyInfluence.AddSensor( front, 0, owner_ );

    REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

    // get extra time we get through rubber usage
    /*
    REAL rubberGranted, rubberEffectiveness;
    sg_RubberValues(owner_->Player(), speed, rubberGranted, rubberEffectiveness);
    REAL rubberTime = (rubberGranted - owner_->GetRubber()) * rubberEffectiveness / speed;
    REAL rubberRatio = owner_->GetRubber() / rubberGranted;
    */
    helper_->rubberData->calculate();
    REAL rubberTime = helper_->rubberData->rubberTimeLeft;
    REAL rubberRatio = helper_->rubberData->rubberUsedRatio;
    // turnedRecently_ = false;

    // these checks can hit our last wall and fail. Temporarily set it to NULL.
    tJUST_CONTROLLED_PTR<gNetPlayerWall> lastWall = owner_->getLastWall();
    owner_->setLastWall(NULL);

    REAL narrowFront = 1;

    // cast four diagonal rays
    Sensor forwardLeft(owner_, pos, scanDir.Turn(+1, +1));
    forwardLeft.detect(5);

    Sensor backwardLeft(owner_, pos, scanDir.Turn(-1, +narrowFront));
    backwardLeft.detect(5);

    Sensor forwardRight(owner_, pos, scanDir.Turn(+1, -1));
    forwardRight.detect(5);

    Sensor backwardRight(owner_, pos, scanDir.Turn(-1, -narrowFront));
    backwardRight.detect(5);

    // determine survival chances in the four directions
    REAL frontOpen = Distance(forwardLeft, forwardRight);
    REAL leftOpen = Distance(forwardLeft, backwardLeft);
    REAL rightOpen = Distance(forwardRight, backwardRight);
    REAL rearOpen = Distance(backwardLeft, backwardRight);

    Sensor self(owner_, pos, scanDir.Turn(-1, 0));

    // fake entries
    self.before_hit = pos;
    self.windingNumber_ = owner_->getWindingNumber();
    self.type = gSENSOR_SELF;
    self.hitDistance_ = 0;
    self.hitOwner_ = owner_;
    self.hitTime_ = owner_->LastTime(); // localCurrentTime;
    self.lr = -1;
    REAL rearLeftOpen = Distance(backwardLeft, self);
    self.lr = 1;
    REAL rearRightOpen = Distance(backwardRight, self);

    {
    // override: don't camp (too much)
    if (forwardRight.type == gSENSOR_SELF &&
        forwardLeft.type == gSENSOR_SELF &&
        backwardRight.type == gSENSOR_SELF &&
        backwardLeft.type == gSENSOR_SELF &&
        front.type == gSENSOR_SELF &&
        forwardRight.lr == front.lr &&
        forwardLeft.lr == front.lr &&
        backwardRight.lr == front.lr &&
        backwardLeft.lr == front.lr &&
        frontOpen + leftOpen + rightOpen < owner_->GetDistance() * .5)
    {
        // turnedRecently_ = true;
        if (front.lr > 0)
        {
            if (leftOpen > minstep * speed)
                // force a turn to the left
                rightOpen = 0;
            else if (front.hit * range < 2 * minstep)
                // force a preliminary turn to the right that will allow us to reverse
                frontOpen = 0;
        }
        else
        {
            if (rightOpen > minstep * speed)
                // force a turn to the right
                leftOpen = 0;
            else if (front.hit * range < 2 * minstep)
                // force a preliminary turn to the left that will allow us to reverse
                frontOpen = 0;
        }
    }
    }

    // override rim hugging
    if (forwardRight.type == gSENSOR_SELF &&
        forwardLeft.type == gSENSOR_RIM &&
        backwardRight.type == gSENSOR_SELF &&
        backwardLeft.type == gSENSOR_RIM &&
        // backwardLeft.hit < .1 &&
        forwardRight.lr == -1 &&
        backwardRight.lr == -1)
    {
    // turnedRecently_ = true;
    if (rightOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
    {
        turn = new gTurnData(RIGHT,2,"right by override rim hugging 1");
        return turn;

        // owner_->Act( &gCycle::se_turnRight, 1 );
        // owner_->Act( &gCycle::se_turnRight, 1 );
    }
    else if (leftOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
    {
        turn = new gTurnData(LEFT,2,"left 2 by override rim hugging 1");
        return turn;
        // owner_->Act( &gCycle::se_turnLeft, 1 );
        // owner_->Act( &gCycle::se_turnLeft, 1 );
    }
    }

    if (forwardLeft.type == gSENSOR_SELF &&
        forwardRight.type == gSENSOR_RIM &&
        backwardLeft.type == gSENSOR_SELF &&
        backwardRight.type == gSENSOR_RIM &&
        // backwardRight.hit < .1 &&
        forwardLeft.lr == 1 &&
        backwardLeft.lr == 1)
    {
    // turnedRecently_ = true;
    if (leftOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
    {
        turn = new gTurnData(LEFT,2,"left 2 by override rim hugging 2");
        return turn;
        // owner_->Act( &gCycle::se_turnLeft, 1 );
        // owner_->Act( &gCycle::se_turnLeft, 1 );
    }
    else if (rightOpen > speed * (owner_->GetTurnDelay() - rubberTime * .8))
    {
        turn = new gTurnData(RIGHT,2,"right 2 by override rim hugging 2");
        return turn;
    }
    }

    // get the best turn direction
    uActionPlayer *bestAction = (leftOpen > rightOpen) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
    int bestDir = (leftOpen > rightOpen) ? 1 : -1;
    REAL bestOpen = (leftOpen > rightOpen) ? leftOpen : rightOpen;
    Sensor &bestForward = (leftOpen > rightOpen) ? forwardLeft : forwardRight;
    Sensor &bestBackward = (leftOpen > rightOpen) ? backwardLeft : backwardRight;

    Sensor direct(owner_, pos, scanDir.Turn(0, bestDir));
    direct.detect(5);

    // restore last wall
    owner_->setLastWall(lastWall);

    // only turn if the hole has a shape that allows better entry after we do a zig-zag, or if we're past the good turning point
    // see how the survival chance is distributed between forward and backward half
    REAL forwardHalf = Distance(direct, bestForward);
    REAL backwardHalf = Distance(direct, bestBackward);

    REAL forwardOverhang = bestForward.HitWallExtends(bestForward.Direction(), pos);
    REAL backwardOverhang = bestBackward.HitWallExtends(bestForward.Direction(), pos);

    // we have to move forward this much before we can hope to turn
    minMoveOn = bestBackward.HitWallExtends(dir, pos);

    // maybe the direct to the side sensor is better?
    REAL minMoveOnOther = direct.HitWallExtends(dir, pos);

    // determine how far we can drive on
    maxMoveOn = bestForward.HitWallExtends(dir, pos);
    REAL maxMoveOnOther = front.HitWallExtends(dir, pos);
    if (maxMoveOn > maxMoveOnOther)
    maxMoveOn = maxMoveOnOther;

    if (maxMoveOn > minMoveOnOther && forwardHalf > backwardHalf && direct.hitOwner_ == bestBackward.hitOwner_)
    {
    backwardOverhang = direct.HitWallExtends(bestForward.Direction(), pos);
    minMoveOn = minMoveOnOther;
    }

    // best place to turn
    moveOn = .5 * (minMoveOn * (1 + rubberRatio) + maxMoveOn * (1 - rubberRatio));

    // hit the brakes before you hit anything and if it's worth it
    bool brake = sg_brakeCycle > 0 &&
                    front.hit * lookahead * sg_cycleBrakeDeplete < owner_->GetBrakingReservoir() &&
                    sg_brakeCycle * front.hit * lookahead < 2 * speed * owner_->GetBrakingReservoir() &&
                    (maxMoveOn - minMoveOn) > 0 &&
                    owner_->GetBrakingReservoir() * (maxMoveOn - minMoveOn) < speed * owner_->GetTurnDelay();

    if (frontOpen < bestOpen &&
        (forwardOverhang <= backwardOverhang || (minMoveOn < 0 && moveOn < minstep * speed)))
    {
    minMoveOn = maxMoveOn = moveOn = 0;
    {

        turn = new gTurnData(ActToTurn(bestAction) ,1,"frontOpen < bestOpen bestAction ");;
        return turn;
    }

    brake = false;
    }
    else
    {
    // the best
    REAL bestSoFar = frontOpen > bestOpen ? frontOpen : bestOpen;
    bestSoFar *= (10 * (1 - rubberRatio) + 1);

    if (rearOpen > bestSoFar && (rearLeftOpen > bestSoFar || rearRightOpen > bestSoFar))
    {
        brake = false;
        // turnedRecently_ = true;

        bool goLeft = rearLeftOpen > rearRightOpen;

        // dead end. reverse into the opposite direction of the front wall
        uActionPlayer *bestAction = goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
        uActionPlayer *otherAction = !goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
        Sensor &bestForward = goLeft ? forwardLeft : forwardRight;
        Sensor &bestBackward = goLeft ? backwardLeft : backwardRight;
        Sensor &otherForward = !goLeft ? forwardLeft : forwardRight;
        Sensor &otherBackward = !goLeft ? backwardLeft : backwardRight;

        // space in the two directions available for turns
        REAL bestHit = bestForward.hit > bestBackward.hit ? bestBackward.hit : bestForward.hit;
        REAL otherHit = otherForward.hit > otherBackward.hit ? otherBackward.hit : otherForward.hit;

        bool wait = false;

        // well, after a short turn to the right if space is tight
        if (bestHit * lookahead < owner_->GetTurnDelay() + rubberTime)
        {
            if (otherHit < bestForward.hit * 2 && front.hit * lookahead > owner_->GetTurnDelay() * 2)
            {
                // wait a bit, perhaps there will be a better spot
                wait = true;
                BotDebug("WAITING");
            }
            else if (!wait)
            {
                {

                    turn = new gTurnData(ActToTurn(otherAction),1,"rearOpen > bestSoFar ootherAction");;
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

        if (!wait)
        {
            turn = new gTurnData(ActToTurn(bestAction),1,"not waiting bestAction");;
            return turn;
        }

        minMoveOn = maxMoveOn = moveOn = 0;
    }
    }

    // execute brake command
    owner_->Act(&gCycle::s_brake, brake ? 1 : -1);

    noTurns++;
    // BotDebug("NO TURN FOUND", noTurns);
    return turn;
}


private:
gCycle *owner_; //!< owner of chatbot
gHelper *helper_;
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
    if (hitTime_ > hitOwner_->LastTime() &&
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

    else if (-2 * a.lr * (a.windingNumber_ - b.windingNumber_) >
                owner_->Grid()->WindingNumber())
    {
    // this looks like a way out to me
    return fabsf(a.hitDistance_ - b.hitDistance_) * 10 * selfHatred;
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

gCycle* gHelperEnemiesData::detectEnemies() {
    allEnemies.clear();
    closestEnemy = nullptr;
    REAL closestEnemyDistanceSquared = 1212121212;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        auto other = dynamic_cast<gCycle*>(se_PlayerNetIDs[i]->Object());
        if (other != nullptr && other->Alive() && other->Team() != owner_->Team()) {
            REAL positionDifference = st_GetDifference(other->Position(), owner_->Position());
            if (positionDifference < closestEnemyDistanceSquared) {
                closestEnemyDistanceSquared = positionDifference;
                closestEnemy = other;
            }
            allEnemies.emplace(other);
        }
    }

    return closestEnemy;
}



void gHelperRubberData::calculate() {
    if (!helper_->aliveCheck()) { return; }

    sg_RubberValues( owner_->Player(), owner_->verletSpeed_, rubberAvailable, rubberEffectiveness );
    rubberTimeLeft = ( rubberAvailable - owner_->GetRubber() )*rubberEffectiveness/owner_->verletSpeed_;
    rubberUsedRatio = owner_->GetRubber()/rubberAvailable;
    rubberFactor = owner_->verletSpeed_ * (  owner_->GetTurnDelay() - rubberTimeLeft );//* sg_helperSmartTurningRubberTimeMult );
    //rubberFactor *= rubberFactorMult; For Editing value with a mult
}


bool gHelperEnemiesData::exist(gCycle* enemy) {
    return bool(enemy) && enemy->Alive();
}

REAL gSmartTurningCornerData::getTimeUntilTurn(REAL speed) {
    return distanceFromPlayer /= speed * .98f;
}

bool gSmartTurningCornerData::isInfront(eCoord pos, eCoord dir) {
    return eCoord::F(dir, currentPos - pos) > 0;
}

gHelperSensors::gHelperSensors(gSensor* front_, gSensor* left_, gSensor* right_) :
front(front_), left(left_), right(right_) {}


gHelperSensorsData::gHelperSensorsData(gCycle *owner) :
    owner_(owner),
    front_stored(new gSensor(owner_, owner_->Position(), owner_->Direction())),
    left_stored(new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(LEFT)))),
    right_stored(new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(RIGHT))))
{
}

gSensor * gHelperSensorsData::getSensor(int dir, bool newSensor)
{
    return getSensor(owner_->Position(),dir, newSensor);
}

gSensor *gHelperSensorsData::getSensor(eCoord start, eCoord dir)
{
    gSensor * sensor = new gSensor(owner_,start, dir);
    sensor->detect(sg_helperSensorRange);
    return sensor;
}

gSensor *gHelperSensorsData::getSensor(eCoord start, int dir, bool newSensor)
{
    if (sg_helperSensorLightUsageMode && !newSensor)
    {
        switch (dir)
        {

        case LEFT:
        {

            if (sg_helperSensorDiagonalMode)
                left_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(-cos(M_PI/4), sin(M_PI/4))), true);
            else
                left_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(0, 1)), true);

            return left_stored;
        }
        case FRONT:
        {
            front_stored->detect(sg_helperSensorRange, start, owner_->Direction(), true);
            return front_stored;
        }
        case RIGHT:
        {
            if (sg_helperSensorDiagonalMode)
                right_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(-cos(M_PI/4), -sin(M_PI/4))), true);
            else
                right_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(0, -1)), true);
            return right_stored;
        }
        }
    }
    else
    {
        switch (dir)
        {
        case LEFT:
        {
            gSensor *left = new gSensor(owner_, start, owner_->Direction().Turn(eCoord(0, 1)));
            left->detect(sg_helperSensorRange, true);
            return left;
        }
        case FRONT:
        {
            gSensor *front = new gSensor(owner_, start, owner_->Direction());
            front->detect(sg_helperSensorRange, true);
            return front;
        }
        case RIGHT:
        {
            gSensor *right = new gSensor(owner_, start, owner_->Direction().Turn(eCoord(0, -1)));
            right->detect(sg_helperSensorRange, true);
            return right;
        }
        }
    }
}

gHelperData::gHelperData(gHelperSensorsData *sensors_, gCycle *owner)
    : sensors(*sensors_),
      owner_(owner)
{
    ownerSpeed = &owner_->verletSpeed_;
}

REAL gHelperData::turnSpeedFactorF() {
    return ((*ownerSpeed) * owner_->GetTurnDelay());
}

void gHelperData::Load(REAL a_speedFactor,
                         REAL a_turnSpeedFactor, REAL a_turnSpeedFactorPercent,
                         REAL a_turnDistance, REAL a_thinkAgain, REAL a_turnDir,
                         REAL a_turnTime)
    {
      speedFactor = (a_speedFactor);
      turnSpeedFactor = (a_turnSpeedFactor);
      turnSpeedFactorPercent = (a_turnSpeedFactorPercent);
      turnDistance = (a_turnDistance);
      thinkAgain = (a_thinkAgain);
      turnDir = (a_turnDir);
      turnTime = (a_turnTime);
      }



gHelperData::gHelperData(gHelperSensorsData &sensors_, REAL a_speedFactor,
                         REAL a_turnSpeedFactor, REAL a_turnSpeedFactorPercent,
                         REAL a_turnDistance, REAL a_thinkAgain, REAL a_turnDir,
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



//SmartTurning
gSmartTurning::gSmartTurning(gHelper *helper, gCycle *owner)
        : helper_             ( helper ),
            owner_              ( owner ),
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

    if (sg_helperSmartTurningAutoTrace) {
        smartTurningAutoTrace(data);
    }

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

    if (sg_helperSmartTurningFollowTail) {
        followTail(data);
    }

    if (sg_helperSmartTurningFrontBot) {
        smartTurningFrontBot(data);
    }
}

bool gSmartTurning::isClose(eCoord pos, REAL closeFactor) {
    eCoord distanceToPos = owner_->pos - pos;
    return distanceToPos.NormSquared() < closeFactor * closeFactor;
}



void gSmartTurning::smartTurningPlan(gHelperData &data) {
    if (!helper_->aliveCheck()) { return; }
    if (!helper_->leftCorner.exist && !helper_->rightCorner.exist) {
        helper_->findCorners(data);
        return;
    }
    gSmartTurningCornerData *cornerToUse = &helper_->leftCorner;
    int dir = LEFT;
    if (cornerToUse->exist && helper_->rightCorner.exist) {
        cornerToUse = cornerToUse->distanceFromPlayer < helper_->rightCorner.distanceFromPlayer ? cornerToUse : &helper_->rightCorner;
        dir = cornerToUse == cornerToUse ? LEFT : RIGHT;
    } else if (!cornerToUse->exist && helper_->rightCorner.exist) {
        cornerToUse =  &helper_->rightCorner;
        dir = RIGHT;
    }

    bool close = isClose(cornerToUse->currentPos, sg_helperShowCornersBoundary);
    if (!close)
    {
        return;
    }

    if (cornerToUse->infront)
    {
        if (data.sensors.getSensor(dir)->hit > 5)
        {
            dir == LEFT ? owner_->Act(&gCycle::se_turnLeft, 1) : owner_->Act(&gCycle::se_turnRight, 1);
        }
    }
}

void gSmartTurning::thinkSurvive(gHelperData &data) {

}

REAL lastTailTurnTime = -999;
void gSmartTurning::followTail(gHelperData &data) {

    if (!helper_->aliveCheck()) { return; }

    if (owner_->justCreated == true){
        lastTailTurnTime = -999;
    }
    // if (!isClose(owner_->tailPos, data.turnSpeedFactorF() * 3)) {
    //     return;
    // }

    if (owner_->tailMoving != true) {
        return;
    }
    REAL delay = data.turnSpeedFactorF() * sg_helperSmartTurningFollowTailDelayMult;
    bool drivingStraight = helper_->drivingStraight();
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;

    canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningFollowTailFreeSpaceMult);
    if (closedIn || blockedBySelf) {
        return;
    }

    // if (owner_->tailMoving) {
    // // Calculate the direction from the cycle to the tail
    // eCoord directionToTail = owner_->tailPos - owner_->pos;
    // directionToTail.Normalize();

    // // Determine if the tail is in front of or behind the cycle
    // if (directionToTail * owner_->dir < 0) {
    //     // The tail is behind the cycle, turn towards it

    //     // Check if there are walls to the left or right of the cycle
    //     gSensor leftSensor(owner_, owner_->pos, owner_->dir.Turn(0, 1));
    //     leftSensor.detect(sg_helperSensorRange);
    //     gSensor rightSensor(owner_, owner_->pos, owner_->dir.Turn(0, -1));
    //     rightSensor.detect(sg_helperSensorRange);

    //     if (directionToTail * owner_->dir > 0 && canSurviveRightTurn && rightSensor.hit > .999999) {
    //         owner_->Act(&gCycle::se_turnRight, 1);
    //     }
    //     else if (canSurviveLeftTurn && leftSensor.hit > .999999) {
    //         owner_->Act(&gCycle::se_turnLeft, 1);
    //     }
    // }
    // else {
    //     // The tail is in front of the cycle, stay on current path

    //     // The tail is directly in front of or behind the cycle
    //     // Stay on current path
    // }
    // }
    // return;
    //bool turnedRecently = !(lastTailTurnTime < 0) && owner_->lastTurnTime + delay > CurrentTime();
    //con << owner_->lastTurnTime + delay << " > " << CurrentTime() << "\n" << turnedRecently << "\n";
    bool readyToTurn = helper_->CurrentTime() > lastTailTurnTime + delay;

    if (!readyToTurn) {
        return;
    }

    int turnDirection = thinkPath(owner_->tailPos,data);

    if (canSurviveRightTurn && turnDirection == -1) {
        owner_->Act(&gCycle::se_turnRight, 1);
        lastTailTurnTime = helper_->CurrentTime();
    }

    if (canSurviveLeftTurn && turnDirection == 1) {
        owner_->Act(&gCycle::se_turnLeft, 1);
        lastTailTurnTime = helper_->CurrentTime();
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
    REAL ls=data.sensors.getSensor(LEFT)->hit;
    REAL rs=data.sensors.getSensor(RIGHT)->hit;

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
        //debugLine(0,1,1,5,4*data.speedFactorF(),pos,pos);

    }
    while (goon && nogood);

    if (goon)
    {
        // now we have found our next goal. Try to get there.
        eCoord pos    = owner_->Position();
        eCoord target = path.CurrentPosition();

        debugLine(.2,1,0,5,data.speedFactorF(),target,target);
        debugLine(.2,1,0,5,data.speedFactorF(),pos,target);
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

    if (owner_->turnIgnoreTime > owner_->localCurrentTime){
        this->blockTurn = BOTH;
        return;
    }
    if ((closedIn && sg_helperSmartTurningClosedIn ) || (closedIn) && blockedBySelf) {
        goto SKIP_BLOCKTURN;
    }

    if (!canSurviveLeftTurn && !canSurviveRightTurn) {
        if (this->blockTurn != BOTH)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING BOTH TURNS", "");
        this->blockTurn = BOTH;
        return;
    }

    if (!canSurviveLeftTurn) {

        if (this->blockTurn != LEFT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING LEFT TURNS", "");
        this->blockTurn = LEFT;
        return;
    }

    if (!canSurviveRightTurn) {
        if (this->blockTurn != RIGHT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING RIGHT TURN", "");
        this->blockTurn = RIGHT;
        return;
    }

    SKIP_BLOCKTURN:
    {
        //this->turnIgnoreTime = -999;
        owner_->lastBlockedTurn = this->blockTurn;
        if (this->blockTurn !=0)
            HelperDebug::Debug("SMART TURNING SURVIVE", "UNBLOCKING TURNS", "");
        this->blockTurn = 0;
        if (sg_helperSmartTurningSurviveTrace) {
            smartTurningSurviveTrace(data);
        }
        return;
    }
}

void gSmartTurning::smartTurningAutoTrace(gHelperData &data) {
    if (!helper_->aliveCheck()) { return; }
    if (!helper_->drivingStraight()) { return; }
    int lastTurn = owner_->lastTurnAttemptDir;

    if (owner_->lastTurnAttemptTime < owner_->lastTurnTime) {
        return;
    }

    if (!(helper_->rightCorner.exist || helper_->rightCorner.exist))
        return;

    switch(lastTurn) {
        case LEFT: {
            REAL turnTimeFactor = helper_->leftCorner.getTimeUntilTurn(owner_->Speed());

            if ( helper_->leftCorner.exist && isClose(helper_->leftCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
                    helper_->leftCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
                    helper_->leftCorner.distanceFromPlayer < sg_helperSmartTurningAutoTraceDistance &&
                    (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime)  ) {
                    if ( makeTurnIfPossible(data,LEFT,1) )
                        HelperDebug::Debug("SMART TURNING AUTO TRACE", "TURNED LEFT", "");
            }
            return;
        }
        case RIGHT: {
            REAL turnTimeFactor = helper_->rightCorner.getTimeUntilTurn(owner_->Speed());

            if ( helper_->rightCorner.exist && isClose(helper_->rightCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
                    helper_->rightCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
                    helper_->rightCorner.distanceFromPlayer < sg_helperSmartTurningAutoTraceDistance &&
                    (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime)  ) {
                 if (makeTurnIfPossible(data,RIGHT,1));
                    HelperDebug::Debug("SMART TURNING TRACE", "TURNED RIGHT", "");
            }
            return;
        }
    }
}


void gSmartTurning::smartTurningSurviveTrace(gHelperData &data) {
    if (!helper_->aliveCheck()) { return; }
    if (!helper_->drivingStraight()) { return; }
    int lastTurn = owner_->lastTurnAttemptDir;

    if (owner_->lastTurnAttemptTime < owner_->lastTurnTime) {
        return;
    }

    switch(lastTurn) {
        case LEFT: {
            REAL turnTimeFactor = helper_->leftCorner.getTimeUntilTurn(owner_->Speed());

            if ( helper_->leftCorner.exist && isClose(helper_->leftCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
                    owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime/(10 * data.turnSpeedFactorF())) > helper_->leftCorner.noticedTime &&
                    helper_->leftCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
                    (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime)  ) {
                    if ( makeTurnIfPossible(data,LEFT,1) )
                        HelperDebug::Debug("SMART TURNING TRACE", "TURNED LEFT", "");
            }
            return;
        }
        case RIGHT: {
            REAL turnTimeFactor = helper_->rightCorner.getTimeUntilTurn(owner_->Speed());

            if ( helper_->rightCorner.exist && isClose(helper_->rightCorner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
                    owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime/(10 * data.turnSpeedFactorF())) > helper_->rightCorner.noticedTime &&
                    helper_->rightCorner.getTimeUntilTurn(owner_->Speed()) > 0 &&
                    (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime)  ) {
                    if ( makeTurnIfPossible(data,RIGHT,1) )
                        HelperDebug::Debug("SMART TURNING TRACE", "TURNED LEFT", "");
            }
            return;
        }
    }
}

bool gSmartTurning::makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor){
    if (!helper_->drivingStraight())
        return false;

    switch (dir) {
        case LEFT: {
            if (canSurviveTurnSpecific(data,dir,spaceFactor)) {
                return owner_->ActBot(&gCycle::se_turnLeft, 1);
            }
            return false;
        }
        case RIGHT: {
            if (canSurviveTurnSpecific(data,dir,spaceFactor)) {
                return owner_->ActBot(&gCycle::se_turnRight, 1);
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
        compareFactor = spaceFactor * data.turnSpeedFactorF();
    }
    else
    {
        //calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);
        helper_->rubberData->calculate();
        compareFactor = helper_->rubberData->rubberFactor;
    }

    if (dir == LEFT)
    {
        return data.sensors.getSensor(LEFT)->hit > compareFactor;
    }
    else if (dir == RIGHT)
    {
        return data.sensors.getSensor(RIGHT)->hit > compareFactor;
    }
}

void gSmartTurning::canSurviveTurn(gHelperData &data, REAL &canSurviveLeftTurn, REAL &canSurviveRightTurn, bool &closedIn, bool &blockedBySelf, REAL freeSpaceFactor) {
    if (!helper_->aliveCheck()) { return; }

    //calculateRubberFactor(sg_helperSmartTurningRubberTimeMult,sg_helperSmartTurningRubberFactorMult);
    helper_->rubberData->calculate();
    canSurviveLeftTurn = true;
    canSurviveRightTurn = true;

    gSensor *front = data.sensors.getSensor(FRONT);
    gSensor *left = data.sensors.getSensor(LEFT);
    gSensor *right = data.sensors.getSensor(RIGHT);

    bool canTurnLeftRubber = true, canTurnRightRubber = true, canTurnLeftSpace = true, canTurnRightSpace = true;
    REAL closedInFactor = data.turnSpeedFactorF() * sg_helperSmartTurningClosedInMult;
    closedIn = (front->hit <= closedInFactor && left->hit <= closedInFactor && right->hit <= closedInFactor);
    blockedBySelf =  (left->type == gSENSOR_SELF && right->type == gSENSOR_SELF && front->type == gSENSOR_SELF);

    if (freeSpaceFactor > 0) {
        if (left->hit < data.turnSpeedFactorF() * freeSpaceFactor
         && front->hit > data.turnSpeedFactorF() * freeSpaceFactor
         && right->hit > data.turnSpeedFactorF() * freeSpaceFactor  ) {
            canTurnLeftSpace = false;
        }
        if (right->hit < data.turnSpeedFactorF() * freeSpaceFactor
         && front->hit > data.turnSpeedFactorF() * freeSpaceFactor
         && left->hit > data.turnSpeedFactorF() * freeSpaceFactor ) {
            canTurnRightSpace = false;
        }
    }

    if (left->hit < helper_->rubberData->rubberFactor) {
        canTurnLeftRubber = false;
    }

    if (right->hit < helper_->rubberData->rubberFactor) {
        canTurnRightRubber = false;
    }

    if (freeSpaceFactor > 0){
        canSurviveLeftTurn = !closedIn ? canTurnLeftRubber && canTurnLeftSpace : canTurnLeftRubber ;
        canSurviveRightTurn = !closedIn ? canTurnRightRubber && canTurnRightSpace : canTurnRightRubber ;
    } else {
        canSurviveLeftTurn = canTurnLeftRubber;
        canSurviveRightTurn = canTurnRightRubber;
    }
}


// void gSmartTurning::calculateRubberFactor(REAL rubberMult, REAL rubberFactorMult) {
//     if (!helper_->aliveCheck()) { return; }
//     REAL rubberGranted, rubberEffectiveness;
//     sg_RubberValues( owner_->Player(), owner_->verletSpeed_, rubberGranted, rubberEffectiveness );
//     rubberTime = ( rubberGranted - owner_->GetRubber() )*rubberEffectiveness/owner_->verletSpeed_;
//     rubberRatio = owner_->GetRubber()/rubberGranted;
//     rubberFactor = owner_->verletSpeed_ * (  owner_->GetTurnDelay() - rubberTime * sg_helperSmartTurningRubberTimeMult );
//     rubberFactor *= rubberFactorMult;
// }

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
    if(sg_helperSmartTurningFrontBotTurnOnce && owner_->lastBotTurnTime > owner_->lastTurnTime){
        return;
    }
    REAL hitRange = data.sensors.getSensor(FRONT)->hit;
    if (hitRange <= sg_helperSmartTurningFrontBotThinkRange * data.turnSpeedFactorF())
    {
        //calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);
        helper_->rubberData->calculate();
        gHelperEmergencyTurn emergencyTurn(owner_, helper_);

        if (helper_->rubberData->rubberUsedRatio >= sg_helperSmartTurningFrontBotActivationRubber || hitRange <= sg_helperSmartTurningFrontBotActivationSpace)
        {
            bool turnMade = false;
            gTurnData *turnData = emergencyTurn.getTurn();
            if (turnData->exist && turnData->numberOfTurns > 0)
            {
                for (int i = 0; i < turnData->numberOfTurns; i++)
                {
                    if (makeTurnIfPossible(data,turnData->direction, 0) == true){

                        HelperDebug::Debug("smartTurningFrontBot",
                            "Turn made: "+std::string(turnData->direction == -1 ? "LEFT" : "RIGHT") + " Reason: " + (turnData->reason),"");
                        turnMade = true;
                    }
                }
            }
            if (sg_helperSmartTurningFrontBotDisableTime > 0 && turnMade)
            {
                owner_->turnIgnoreTime = helper_->CurrentTime() + (sg_helperSmartTurningFrontBotDisableTime);
            }
        }
    }
}

REAL Distance( SensorPub const & a, SensorPub const & b , gCycle *owner_)
{
    // make sure a is left from b
    if ( a.Direction() * b.Direction() < 0 )
        return Distance( b, a, owner_ );

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

    else if (( - 2 * a.lr * (a.windingNumber_ - b.windingNumber_ ) > owner_->Grid()->WindingNumber() ))
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

// gTurnData * gSmartTurning::getEmergencyTurn(gHelperData &data)
//     {
//         REAL chatBotNewWallBlindness = 0;
//         REAL chatBotMinTimestep      = 0;
//         REAL chatBotDelay            = 0;
//         REAL chatBotRange            = 15;
//         REAL chatBotDecay            = 0;
//         REAL chatBotEnemyPenalty     = 0;

//         REAL lookahead = chatBotRange;  // seconds to plan ahead
//         REAL minstep   = chatBotMinTimestep; // minimum timestep between thoughts in seconds

//         // cylce data
//         REAL speed = owner_->Speed();
//         eCoord dir = owner_->Direction();
//         eCoord pos = owner_->Position();


//         REAL range = speed * lookahead;
//         eCoord scanDir = dir * range;

//         REAL frontFactor = .5;

//         SensorPub front(owner_,pos,scanDir);
//         front.detect(frontFactor);
//         owner_->enemyInfluence.AddSensor( front, chatBotEnemyPenalty, owner_ );

//         REAL minMoveOn = 0, maxMoveOn = 0, moveOn = 0;

//         // get extra time we get through rubber usage
//         calculateRubberFactor(sg_helperSmartTurningRubberTimeMult, sg_helperSmartTurningRubberFactorMult);

//         if ( !front.ehit )
//             return (new gTurnData(false));

//         // these checks can hit our last wall and fail. Temporarily set it to NULL.
//         tJUST_CONTROLLED_PTR< gNetPlayerWall > lastWall = owner_->lastWall;
//         owner_->lastWall = NULL;

//         REAL narrowFront = 1;

//         // cast four diagonal rays
//         SensorPub forwardLeft ( owner_, pos, scanDir.Turn(+1,+1 ) );
//         SensorPub backwardLeft( owner_, pos, scanDir.Turn(-1,+narrowFront) );
//         forwardLeft.detect(1);
//         backwardLeft.detect(1);
//         SensorPub forwardRight ( owner_, pos, scanDir.Turn(+1,-1 ) );
//         SensorPub backwardRight( owner_, pos, scanDir.Turn(-1,-narrowFront) );
//         forwardRight.detect(1);
//         backwardRight.detect(1);

//         // determine survival chances in the four directions
//         REAL frontOpen = Distance ( forwardLeft, forwardRight,owner_ );
//         REAL leftOpen  = Distance ( forwardLeft, backwardLeft,owner_ );
//         REAL rightOpen = Distance ( forwardRight, backwardRight,owner_ );
//         REAL rearOpen = Distance ( backwardLeft, backwardRight ,owner_);

//         SensorPub self( owner_, pos, scanDir.Turn(-1, 0) );
//         // fake entries
//         self.before_hit = pos;
//         self.windingNumber_ = owner_->windingNumber_;
//         self.type = gSENSOR_SELF;
//         self.hitDistance_ = 0;
//         self.hitOwner_ = owner_;
//         self.hitTime_ = helper_->CurrentTime();
//         self.lr = -1;
//         REAL rearLeftOpen = Distance( backwardLeft, self ,owner_);
//         self.lr = 1;
//         REAL rearRightOpen = Distance( backwardRight, self ,owner_);
//         bool sg_localBot = true;

//         // get the best turn direction
//         uActionPlayer * bestAction = ( leftOpen > rightOpen ) ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
//         int             bestDir      = ( leftOpen > rightOpen ) ? LEFT : RIGHT;
//         REAL            bestOpen     = ( leftOpen > rightOpen ) ? leftOpen : rightOpen;
//         SensorPub &        bestForward  = ( leftOpen > rightOpen ) ? forwardLeft : forwardRight;
//         SensorPub &        bestBackward = ( leftOpen > rightOpen ) ? backwardLeft : backwardRight;

//         SensorPub direct ( owner_, pos, scanDir.Turn( 0, bestDir) );
//         direct.detect( 1 );

//         // restore last wall
//         owner_->lastWall = lastWall;

//         // only turn if the hole has a shape that allows better entry after we do a zig-zag, or if we're past the good turning point
//         // see how the survival chance is distributed between forward and backward half
//         REAL forwardHalf  = Distance ( direct, bestForward,owner_ );
//         REAL backwardHalf = Distance ( direct, bestBackward,owner_ );

//         REAL forwardOverhang  = bestForward.HitWallExtends( bestForward.Direction(), pos );
//         REAL backwardOverhang  = bestBackward.HitWallExtends( bestForward.Direction(), pos );

//         // we have to move forward this much before we can hope to turn
//         minMoveOn = bestBackward.HitWallExtends( dir, pos );

//         // maybe the direct to the side sensor is better?
//         REAL minMoveOnOther = direct.HitWallExtends( dir, pos );

//         // determine how far we can drive on
//         maxMoveOn      = bestForward.HitWallExtends( dir, pos );
//         REAL maxMoveOnOther = front.HitWallExtends( dir, pos );
//         if ( maxMoveOn > maxMoveOnOther )
//             maxMoveOn = maxMoveOnOther;

//         if ( maxMoveOn > minMoveOnOther && forwardHalf > backwardHalf && direct.hitOwner_ == bestBackward.hitOwner_ )
//         {
//             backwardOverhang  = direct.HitWallExtends( bestForward.Direction(), pos );
//             minMoveOn = minMoveOnOther;
//         }

//         // best place to turn
//         moveOn = .5 * ( minMoveOn * ( 1 + rubberRatio ) + maxMoveOn * ( 1 - rubberRatio ) );


//         if ( frontOpen < bestOpen &&
//                 ( forwardOverhang <= backwardOverhang || ( minMoveOn < 0 && moveOn < minstep * speed ) ) )
//         {
//             minMoveOn = maxMoveOn = moveOn = 0;
//             {
//                 return (new gTurnData(bestDir,2,"frontOpen < bestOpen"));
//             }

//         }
//         else
//         {
//             // the best
//             REAL bestSoFar = frontOpen > bestOpen ? frontOpen : bestOpen;
//             bestSoFar *= ( 10 * ( 1 - rubberRatio ) + 1 );

//             if ( rearOpen > bestSoFar && ( rearLeftOpen > bestSoFar || rearRightOpen > bestSoFar ) )
//             {
//                 bool goLeft = rearLeftOpen > rearRightOpen;

//                 // dead end. reverse into the opposite direction of the front wall
//                 uActionPlayer * bestAction = goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
//                 int bestDir = goLeft ? LEFT : RIGHT;
//                 uActionPlayer * otherAction = !goLeft ? &gCycle::se_turnLeft : &gCycle::se_turnRight;
//                 int otherDir = bestDir * -1;
//                 SensorPub &        bestForward  = goLeft ? forwardLeft : forwardRight;
//                 SensorPub &        bestBackward  = goLeft ? backwardLeft : backwardRight;
//                 SensorPub &        otherForward  = !goLeft ? forwardLeft : forwardRight;
//                 SensorPub &        otherBackward  = !goLeft ? backwardLeft : backwardRight;

//                 // space in the two directions available for turns
//                 REAL bestHit = bestForward.hit > bestBackward.hit ? bestBackward.hit : bestForward.hit;
//                 REAL otherHit = otherForward.hit > otherBackward.hit ? otherBackward.hit : otherForward.hit;

//                 bool wait = false;

//                 // well, after a short turn to the right if space is tight
//                 if ( bestHit * lookahead < owner_->GetTurnDelay() + rubberTime )
//                 {
//                     if ( otherHit < bestForward.hit * 2 && front.hit * lookahead > owner_->GetTurnDelay() * 2 )
//                     {
//                         // wait a bit, perhaps there will be a better spot
//                         wait = true;
//                     }
//                     else if (!wait)
//                     {

//                         //return (new gTurnData(otherDir,2));;

//                         // there needs to be space ahead to finish the maneuver correctly
//                         if (!( maxMoveOn < speed * owner_->GetTurnDelay() )) {
//                                 return (new gTurnData(bestDir,1,"maxMoveOn < speed"));
//                         }
//                     }
//                 }

//                 if (!wait)
//                     return (new gTurnData(bestDir,1,"!wait"));

//             }
//         }

//         return (new gTurnData(false));
//     }

bool gSmartTurning::CanMakeTurn(uActionPlayer *action)
{
    return owner_->CanMakeTurn((action == &gCycle::se_turnRight) ? 1 : -1);
}

REAL gHelper::CurrentTime() {
    return sg_helperCurrentTimeLocal ? owner_->localCurrentTime : se_GameTime();
}

gHelper::gHelper(gCycle *owner)
    : owner_(owner),
        player_(owner->Player()),
        ownerWallLength(owner->ThisWallsLength()),
        ownerTurnDelay(owner->GetTurnDelay()),
        sensors_(new gHelperSensorsData(owner_)),
        data_stored(new gHelperData(sensors_,owner_)),
        rubberData(new gHelperRubberData(this, owner_))
{
    aiCreated = false;
    ownerPos = &owner_->pos;
    ownerDir = &owner_->dir;
    tailPos = &owner_->tailPos;
    ownerSpeed = &owner_->verletSpeed_;
    enemies.owner_ = owner;
    gSmartTurning::Get(this, owner);
    gPathHelper::Get(this, owner);
    gTailHelper::Get(this, owner);
}

gCycle* gHelper::getOwner() { return owner_; }


bool gHelper::drivingStraight() {
    return ((fabs(ownerDir->x) == 1 || fabs(ownerDir->y) == 1));
}

bool gHelper::canSeeTarget(eCoord target,REAL passthrough) {
    gSensor sensor(owner_, (*ownerPos), target - (*ownerPos));
    sensor.detect(REAL(.98));
    return (sensor.hit >=passthrough);
}

eCoord gHelper::closestCorner(eCoord center, REAL radius)
{
    eCoord corner[4];
    corner[0] = eCoord(center.x - (radius), center.y + (radius));
    corner[1] = eCoord(center.x + (radius), center.y + (radius));
    corner[2] = eCoord(center.x + (radius), center.y - (radius));
    corner[3] = eCoord(center.x - (radius), center.y - (radius));

    REAL positionDifference[4];
    positionDifference[0] = st_GetDifference(corner[0], (*ownerPos));
    positionDifference[1] = st_GetDifference(corner[1], (*ownerPos));
    positionDifference[2] = st_GetDifference(corner[2], (*ownerPos));
    positionDifference[3] = st_GetDifference(corner[3], (*ownerPos));

    int minIndex = std::min_element(positionDifference, positionDifference + 4) - positionDifference;

    return corner[minIndex];
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

// See how close two coordinates are, lower the threshold the more strict the comparison
bool directionsAreClose(const eCoord &dir1, const eCoord &dir2, REAL threshold) {

    //con << dir1.Dot(dir2) << " >= " << 1 - threshold << "\n";
    return dir1.Dot(dir2) >= 1 - threshold;

}


void gHelper::detectCut(gHelperData &data, int detectionRange)
{
    if (!aliveCheck())
    {
        return;
    }
    REAL timeout = data.speedFactorF() + sg_helperDetectCutTimeout;
    gCycle *target = enemies.closestEnemy;

    if (enemies.exist(target))
    {
        closestEnemyH.setValue(target->Player()->GetColoredName());
        REAL range = ((*ownerSpeed) * (ownerTurnDelay));
        REAL closeReact = (range + detectionRange);

        // check for distance between owner and closest enemy
        if (!smartTurning->isClose(target->Position(), closeReact))
            return;

        // now we are at the center of the coordinate system facing
        // in direction (0,1).
        eCoord relativeEnemyPos = target->Position() - (*ownerPos);
        eCoord actualEnemyPos = target->Position();
        eCoord enemydir = target->Direction();
        REAL enemyspeed = target->Speed();
        // tColoredString debug;

        // 1 = facing direction
        // rules are symmetrical: exploit that.

        //debug << "relativeEnemyPos " << relativeEnemyPos << "\n";

        eCoord relativeEnemyDir = enemydir;
        relativeEnemyDir = relativeEnemyDir.Turn(ownerDir->Conj()).Turn(LEFT);
        //debug << "relativeEnemyDir " << relativeEnemyDir << "\n";

        bool enemyIsAhead = relativeEnemyPos.Dot(*ownerDir) > 0;
        bool enemyIsBehind = !enemyIsAhead;
        //debug << "enemyIsAhead " << enemyIsAhead << "\n";

        bool enemyIsOnLeft = relativeEnemyPos * (*ownerDir) > 0; //dot product greater than 0
        bool enemyIsOnRight = !enemyIsOnLeft;

        bool enemyIsFacingOurRight = directionsAreClose(enemydir, ownerDir->Turn(RIGHT),0.1);
        bool enemyIsFacingOurLeft = directionsAreClose(enemydir, ownerDir->Turn(LEFT),0.1);

        //debug << "Enemy Dir " << enemydir << "\n";
        //debug << "owner dir after conj " << ownerDir->Conj() << "\n";

        eCoord uTurn = ownerDir->Turn(LEFT).Turn(LEFT);
        //debug << "owner dir after u-turn " << uTurn << "\n";
        bool oppositeDirectionofEnemy = directionsAreClose(enemydir,uTurn,0.001);
        bool sameDirectionAsEnemy = directionsAreClose(enemydir,*ownerDir,.001);

        //debug << "sameDirectionAsEnemy " << sameDirectionAsEnemy << "\n";
        //debug << "oppositeDirectionofEnemy " << oppositeDirectionofEnemy << "\n";

        //debug << "enemyIsOnLeft ?" << enemyIsOnLeft << "\n";
        //debug << "enemyIsOnRight ?" << enemyIsOnRight << "\n";

        //debug << "enemyIsFacingOurLeft ?" << enemyIsFacingOurLeft << "\n";
        //debug << "enemyIsFacingOurRight ?" << enemyIsFacingOurRight << "\n";

        // transform coordinates relative to us:
        if (enemyIsOnLeft){
            if (enemyIsFacingOurRight) {
                relativeEnemyPos = enemydir.Turn(RIGHT);
                enemydir = enemydir.Turn(RIGHT);
            } else {

            }
        } else if (enemyIsOnRight) {
            if (enemyIsFacingOurLeft) {
                relativeEnemyPos = enemydir.Turn(LEFT);
                enemydir = enemydir.Turn(LEFT);
            } else {

            }
        }

        //debug << "relativeEnemyDirAFTER " << relativeEnemyDir << "\n";
        //debug << "relativeEnemyPosAFTER " << relativeEnemyPos << "\n";

        if (oppositeDirectionofEnemy && enemyIsAhead) {
            relativeEnemyPos = enemydir.Turn(ownerDir->Conj()).Turn(LEFT);
        } else {
            relativeEnemyPos = relativeEnemyPos.Turn(ownerDir->Conj()).Turn(0, 1);
            enemydir = enemydir.Turn(ownerDir->Conj()).Turn(0, 1);
            if (sameDirectionAsEnemy)
                //relativeEnemyPos = relativeEnemyPos.Turn(ownerDir->Conj()).Turn(LEFT);
                //enemydir = enemydir.Turn(ownerDir->Conj()).Turn(LEFT);
            if (oppositeDirectionofEnemy)
                return;
        }

        int side = LEFT;
        if (relativeEnemyPos.x < 0) //enemy is on our right?
        {
            side = RIGHT;
            relativeEnemyPos.x *= -1;
            enemydir.x *= -1;
        }

        //detectCutdebugH.setValue(debug);

        // now we can even assume the enemy is on our right side.
        // consider his ping and our reaction time
        REAL enemydist = target->Lag() * enemyspeed;

        enemydist += sg_helperDetectCutReact * enemyspeed;

        REAL ourdist = sg_helperDetectCutReact * (*ownerSpeed);

        // now we consider the worst case: we drive straight on,
        relativeEnemyPos.y -= ourdist;

        // while the enemy cuts us: he goes in front of us
        REAL forward = -relativeEnemyPos.y + .01;
        if (forward < 0) // no need to go to much ahead
            forward = 0;
        if (forward > enemydist)
            forward = enemydist;

        relativeEnemyPos.y += forward;
        enemydist -= forward;

        // and then he turns left or right.
        relativeEnemyPos.x -= enemydist;

        bool canCutUs, canCutEnemy;
        canCutUs    = relativeEnemyPos.y * enemyspeed > relativeEnemyPos.x * (*ownerSpeed); // right ahead of us?
        canCutEnemy = relativeEnemyPos.y * (*ownerSpeed) < -relativeEnemyPos.x * enemyspeed;

        if (canCutUs)
        {
            debugLine(1, 0, 0, sg_helperDetectCutHeight, timeout, (*ownerPos), actualEnemyPos);
        }
        else if (canCutEnemy)
        {
            debugLine(0, 1, 0, sg_helperDetectCutHeight, timeout, (*ownerPos), actualEnemyPos);
        } else {

        }
    }
}

void gHelper::autoBrake() {
    if (!aliveCheck()) { return; }
    REAL brakingReservoir = owner_->GetBrakingReservoir();
    bool cycleBrakeDeplete = true;

    if (sg_helperAutoBrakeDeplete && sg_helperAutoBrakeMin == 0 && sg_cycleBrakeDeplete < 0) {
        cycleBrakeDeplete = false;
    }

    if (owner_->GetBraking() == true && brakingReservoir <= sg_helperAutoBrakeMin && cycleBrakeDeplete) {
        owner_->ActBot( &gCycle::s_brake, -1 );
    }

    if (owner_->GetBraking() == false && brakingReservoir >= sg_helperAutoBrakeMax) {
        owner_->ActBot( &gCycle::s_brake, 1);
    }

}

void gHelper::enemyTracers(gHelperData &data, int detectionRange, REAL timeout)
{
    if (!aliveCheck()) { return; }
    for(auto enemy = enemies.allEnemies.begin(); enemy != enemies.allEnemies.end(); ++enemy)
    {
        gCycle *other = *enemy;
        if (!enemies.exist(other)) {
            continue;
        }

        eCoord enemyPos = other->Position();
        REAL R = .1, G = .1, B = 0;
        bool isClose = smartTurning->isClose(enemyPos, detectionRange + data.turnSpeedFactorF());
        bool enemyFaster = ((other->Speed() > ((*ownerSpeed) * sg_helperEnemyTracersSpeedMult)));
        bool isTeammate = (owner_->Team() == other->Team());

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

void gHelper::showTail(gHelperData &data)
{
    if (!aliveCheck()) { return; }
    if (owner_->tailMoving != true) {
        return;
    }

    REAL timeout = sg_helperShowTailTimeout * data.speedFactorF();

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
    for (auto enemy = enemies.allEnemies.begin(); enemy != enemies.allEnemies.end(); ++enemy)
    {
        gCycle *other = *enemy;
        bool exist = enemies.exist(other);
        if (!exist || !other->tailMoving) {
            continue;
        }
        distanceToTail = sg_helperShowEnemyTailDistanceMult * (eCoord::F(*ownerDir, (other->tailPos) - (*ownerPos)));
        timeout = fabs(distanceToTail) / 10 * data.speedFactorF();
        debugLine(other->color_.r, other->color_.g, other->color_.b, sg_helperShowEnemyTailHeight, timeout * sg_helperShowEnemyTailTimeoutMult, other->tailPos, other->tailPos, sg_helperShowEnemyTailBrightness);
    }
}

void gHelper::showTailTracer(gHelperData &data)
{
    if (!aliveCheck() || !owner_->tailMoving)
        return;

    REAL distanceToTail = sg_helperShowTailTracerTimeoutMult * eCoord::F(*ownerDir, (*tailPos) - (*ownerPos));
    REAL timeout = fabs(distanceToTail) / sg_helperShowTailTracerDistanceMult * data.speedFactorF();

    debugLine(1, 1, 1, sg_helperShowTailTracerHeight, timeout * sg_helperShowTailTracerTimeoutMult, *tailPos, *tailPos);
}

bool gHelper::findCorner(gHelperData &data, gSmartTurningCornerData &corner, const gSensor *sensor)
{

    if (!sensor->ehit) {
        corner.exist = false;
        return false;
    }

    corner.noticedTime = CurrentTime();
    corner.exist = true;

    corner.type = sensor->type;
    corner.currentPos = *sensor->ehit->Point();
    corner.distanceFromPlayer = ownerDir->Dot(corner.currentPos - *ownerPos);
    corner.turnTime = leftCorner.getTimeUntilTurn(owner_->Speed()) + CurrentTime();
    REAL secondEdgeDistance = ownerDir->Dot(*sensor->ehit->Other()->Point() - *ownerPos);

    if (corner.distanceFromPlayer < secondEdgeDistance)
    {
        corner.distanceFromPlayer = secondEdgeDistance;
        corner.currentPos = *sensor->ehit->Other()->Point();
    }

    corner.infront = corner.isInfront(*ownerPos, *ownerDir);

    if (corner.lastPos != corner.currentPos)
    {
        corner.lastPos = corner.currentPos;
        corner.ignoredTime = CurrentTime();
    }
    return true;
}


void gHelper::findCorners(gHelperData &data)
{
    findCorner(data,leftCorner,data.sensors.getSensor(LEFT));
    findCorner(data,rightCorner,data.sensors.getSensor(RIGHT));
}

void gHelper::showCorner(gHelperData &data, gSmartTurningCornerData &corner, REAL timeout) {
    if (corner.exist) {
        REAL timeout = data.speedFactorF() * sg_helperShowCornersTimeout;
        bool isClose = smartTurning->isClose(corner.currentPos, sg_helperShowCornersBoundary);


        if (isClose)// && data.sensors.getSensor(LEFT)->type != gSENSOR_RIM)
        {
            debugLine(1,.5,0,sg_helperShowCornersHeight,timeout,corner.currentPos,corner.currentPos);
        }
    }
}

void gHelper::showCorners(gHelperData &data) {
    if (!aliveCheck()) { return; }
    REAL timeout = data.speedFactorF() * sg_helperShowCornersTimeout;
    findCorners(data);
    showCorner(data,leftCorner,timeout);
    showCorner(data,rightCorner,timeout);
}

void gHelper::showHit(gHelperData &data)
{
    if (!aliveCheck() || !drivingStraight())
        return;

    REAL frontHit = data.sensors.getSensor(FRONT)->hit;
    bool wallClose = frontHit < data.turnSpeedFactorF() * sg_showHitDataRange;
    REAL timeout = data.speedFactorF() * sg_showHitDataTimeout;

    sg_helperShowHitFrontDistH.setValue(frontHit);

    if (wallClose)
    {
        eCoord frontBeforeHit = data.sensors.getSensor(FRONT)->before_hit;
        debugLine(1,.5,0,sg_showHitDataHeightFront,timeout,(*ownerPos),frontBeforeHit);
        if (sg_helperShowHitStartAtHitPos) {
            showHitDebugLines(frontBeforeHit, owner_->Direction(), timeout, data, sg_showHitDataRecursion, LEFT);
            showHitDebugLines(frontBeforeHit, owner_->Direction(), timeout, data, sg_showHitDataRecursion, RIGHT);
        } else {
            showHitDebugLines(*ownerPos, owner_->Direction().Turn(eCoord(0,1)), timeout, data, sg_showHitDataRecursion, LEFT);
            showHitDebugLines(*ownerPos, owner_->Direction().Turn(eCoord(0,-1)), timeout, data, sg_showHitDataRecursion, RIGHT);
        }
    }
}

void gHelper::showHitDebugLines(eCoord currentPos, eCoord initDir, REAL timeout, gHelperData &data, int recursion, int sensorDir)
{

    if (recursion <= 0)
        return;

    recursion--;
    eCoord newDir = initDir.Turn(eCoord(0, sensorDir * -1));

    gSensor *sensor = data.sensors.getSensor(currentPos, initDir);
    eCoord hitPos = sensor->before_hit;
    REAL hitDistance = sensor->hit;

    bool open = hitDistance > data.turnSpeedFactorF() * sg_showHitDataFreeRange;

    if (open)
    {
        debugLine(0, 1, 0, sg_showHitDataHeightSides, timeout, currentPos, hitPos);
    }
    else
    {
        debugLine(1, 0, 0, sg_showHitDataHeightSides, timeout, currentPos, hitPos);
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
    return owner_ && owner_->Alive() && owner_->Grid();
}

void gHelper::Activate()
{
    REAL start = tRealSysTimeFloat();
    ownerPosH.setValue(*ownerPos);
    ownerDirH.setValue(*ownerDir);
    tailPosH.setValue(owner_->tailPos);
    tailDirH.setValue(owner_->tailDir);

    if (!aliveCheck()) { return; }
    owner_->localCurrentTime = se_GameTime();

    enemies.detectEnemies();

    if (sg_helperSmartTurning)
        smartTurning->Activate(*data_stored);

    if (sg_pathHelper)
        pathHelper->Activate(*data_stored);

    if (sg_tailHelper)
        tailHelper->Activate(*data_stored);

    if (sg_helperEnemyTracers)
        enemyTracers(*data_stored, sg_helperEnemyTracersDetectionRange, sg_helperEnemyTracersTimeout);

    if (sg_helperDetectCut)
        detectCut(*data_stored, sg_helperDetectCutDetectionRange);

    if (sg_helperShowHit)
        showHit(*data_stored);

    if (sg_helperShowTail)
        showTail(*data_stored);

    if (sg_helperShowTailTracer)
        showTailTracer(*data_stored);

    if (sg_helperShowTailPath)
        showTailPath(*data_stored);

    if (sg_helperShowEnemyTail)
        showEnemyTail(*data_stored);

    if (sg_helperShowCorners)
        showCorners(*data_stored);

    if (sg_helperAutoBrake)
        autoBrake();

    if (sg_helperAI)
    {
        if (!aiCreated)
        {
            HelperDebug::Debug("sg_helperAI", "Creating AI", "");
            aiPlayer = new gAIPlayer(owner_);
            aiCreated = true;
        }
        else
        {
            if (aiPlayer != NULL)
                aiPlayer->Timestep(se_GameTime() + sg_helperAITime);
        }
    }

    owner_->justCreated = false;
    REAL time = tRealSysTimeFloat() - start;
    sg_helperActivateTimeH.setValue(time);
}


gHelper::~gHelper()
{
}
