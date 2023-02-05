// HELPER
#include "gHelper.h"
#include "nConfig.h"
#include "../gSensor.h"

#include "../gAIBase.h"

#include "gHelperHud.h"
#include "gHelperUtilities.h"
#include "specialized/gSmartTurning.h"

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



bool sg_zoneHelper = false;
static tConfItem<bool> sg_zoneHelperC("HELPER_ZONE", sg_zoneHelper);

bool sg_zoneHelperOwnerOnly = false;
static tConfItem<bool> sg_zoneHelperOwnerOnlyC("HELPER_ZONE_OWNER_ONLY", sg_zoneHelperOwnerOnly);

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
REAL sg_helperShowTailHeight = 1;
static tConfItem<REAL> sg_helperShowTailHeightConf("HELPER_SHOW_TAIL_HEIGHT", sg_helperShowTailHeight);
REAL sg_helperShowTailPassthrough = 0.5;
static tConfItem<REAL> sg_helperShowTailPassthroughConf("HELPER_SHOW_TAIL_PASSTHROUGH", sg_helperShowTailPassthrough);
REAL sg_helperShowTailTimeout = 1;
static tConfItem<REAL> sg_helperShowTailTimeoutConf("HELPER_SHOW_TAIL_TIMEOUT", sg_helperShowTailTimeout);


bool sg_helperTest = false;
static tConfItem<bool> sg_helperTestConf("HELPER_TEST", sg_helperTest);

int sg_helperTestMode = 1;
static tConfItem<int> sg_helperTestModeConf("HELPER_TEST_MODE", sg_helperTestMode);


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

// gHelper::getCorner retrieves a pointer to the gSmartTurningCornerData instance for the given direction
// dir: the direction (LEFT or RIGHT) to retrieve the gSmartTurningCornerData instance for
// Returns: a pointer to the gSmartTurningCornerData instance for the given direction
gSmartTurningCornerData *gHelper::getCorner(int dir)
{
switch (dir)
{
case LEFT:
    return &leftCorner;
    break;
case RIGHT:
    return &rightCorner;
    break;
default:
    return &leftCorner;
}
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
    gZoneHelper::Get(this, owner);
    leftCorner.linkLastCorner(&lastLeftCorner);
    rightCorner.linkLastCorner(&lastRightCorner);

}

bool gHelper::drivingStraight() {
    return ((fabs(ownerDir->x) == 1 || fabs(ownerDir->y) == 1));
}

/**
* @brief Check if the target is visible from the owner's current position
* This function uses a gSensor to detect if the target is visible from the owner's current position
* @param target The target coordinate to check visibility for
* @param passthrough The minimum value of the sensor hit to be considered as visible
* @return True if the target is visible, False otherwise
*/
bool gHelper::canSeeTarget(eCoord target, REAL passthrough) {
    gSensor sensor(owner_, (*ownerPos), target - (*ownerPos));
    sensor.detect(REAL(.98));
    return (sensor.hit >= passthrough);
}

// Returns the closest corner of the given center and radius to the owner's position
// center: the center of the corners
// radius: the distance from the center to the corners
// Returns: the closest corner
eCoord gHelper::closestCorner(eCoord center, REAL radius)
{
    // Define the 4 corners
    eCoord corner[4];
    corner[0] = eCoord(center.x - (radius), center.y + (radius));
    corner[1] = eCoord(center.x + (radius), center.y + (radius));
    corner[2] = eCoord(center.x + (radius), center.y - (radius));
    corner[3] = eCoord(center.x - (radius), center.y - (radius));

    // Calculate the difference between each corner and the owner's position
    REAL positionDifference[4];
    positionDifference[0] = st_GetDifference(corner[0], (*ownerPos));
    positionDifference[1] = st_GetDifference(corner[1], (*ownerPos));
    positionDifference[2] = st_GetDifference(corner[2], (*ownerPos));
    positionDifference[3] = st_GetDifference(corner[3], (*ownerPos));

    // Find the index of the minimum difference
    int minIndex = std::min_element(positionDifference, positionDifference + 4) - positionDifference;

    // Return the closest corner
    return corner[minIndex];
}

void gHelper::detectCut(gHelperData &data, int detectionRange)
{
    // Check if the object is still alive
    if (!aliveCheck())
        return;

    // timeout value = speed factor + sg_helperDetectCutTimeout
    REAL timeout = data.speedFactorF() + sg_helperDetectCutTimeout;

    // Get the closest enemy
    gCycle *enemy = enemies.closestEnemy;

    // If there's no enemy, return
    if (!enemies.exist(enemy))
        return;

    closestEnemyH << (enemy->Player()->GetColoredName());

    bool canCutUs, canCutEnemy;

    // Get the position and direction of the enemy cycle
    eCoord enemyPos = enemy->Position();
    eCoord enemyDir = enemy->Direction();

    // Get the speed of the enemy cycle
    REAL enemySpeed = enemy->Speed();

    // Get the position and direction of our cycle
    eCoord ourPos = *ownerPos;
    eCoord ourDir = *ownerDir;

    // Get the speed of our cycle
    REAL ourSpeed = *ownerSpeed;

    // Transform our position and direction to be relative to the enemy cycle
    eCoord relEnemyPos = enemyPos - ourPos;
    relEnemyPos = relEnemyPos.Turn(ourDir.Conj()).Turn(LEFT);

    // Check if the enemy is facing opposite direction of ours
    bool oppositeDirectionofEnemy = directionsAreClose(enemyDir, ourDir.Turn(LEFT).Turn(LEFT), 0.001);

    // Check if the enemy is on the left side of us
    bool enemyIsOnLeft = relEnemyPos.x < 0;

    // Check if the enemy is on the right side of us
    bool enemyIsOnRight = !enemyIsOnLeft;

    // Check if the enemy is facing right direction of ours
    bool enemyIsFacingOurRight = directionsAreClose(enemyDir, ourDir.Turn(RIGHT), 0.1);

    // Check if the enemy is facing left direction of ours
    bool enemyIsFacingOurLeft = directionsAreClose(enemyDir, ourDir.Turn(LEFT), 0.1);

    // If the enemy is facing opposite direction of ours, flip the relative position
    if (oppositeDirectionofEnemy)
    {
        relEnemyPos.y *= -1;
    }
    // If the enemy is facing right direction of ours and on the left side, rotate the relative position
    else if (enemyIsOnLeft && enemyIsFacingOurRight)
    {
        relEnemyPos = relEnemyPos.Turn(LEFT);
    }
    // If the enemy is facing left direction of ours and on the right side, rotate the relative position
    else if (enemyIsOnRight && enemyIsFacingOurLeft)
    {
        relEnemyPos = relEnemyPos.Turn(RIGHT);
    }

    // If the enemy is on the left side, flip the relative position
    if (relEnemyPos.x < 0)
    {
        relEnemyPos.x *= -1;
    }

    // now we can even assume the enemy is on our right side.
    // consider his ping and our reaction time

    // Calculate the enemy's distance with consideration of its lag and our reaction time
    REAL enemydist = enemy->Lag();// * enemySpeed;

    enemydist += sg_helperDetectCutReact * enemySpeed;

    REAL ourdist = sg_helperDetectCutReact * ourSpeed;

    // now we consider the worst case: we drive straight on,
    relEnemyPos.y -= ourdist;

    // while the enemy cuts us: he goes in front of us
    REAL forward = -relEnemyPos.y + .01;
    if (forward < 0) // no need to go to much ahead
        forward = 0;
    if (forward > enemydist)
        forward = enemydist;

    relEnemyPos.y += forward;
    enemydist -= forward;

    // and then he turns left or right.
    relEnemyPos.x -= enemydist;

    canCutUs = relEnemyPos.y * enemySpeed > relEnemyPos.x * ourSpeed; // right ahead of us? (and faster)
    canCutEnemy = relEnemyPos.y * ourSpeed < -relEnemyPos.x * enemySpeed; 

    if (canCutUs)
    {
        gHelperUtility::debugLine(gRealColor(1, 0, 0), sg_helperDetectCutHeight, timeout, ourPos, enemy->pos);
    }
    else if (canCutEnemy)
    {
        gHelperUtility::debugLine(gRealColor(0, 1, 0), sg_helperDetectCutHeight, timeout, ourPos, enemy->pos);
    }
    else
    {
        gHelperUtility::debugLine(gRealColor(.4, .4, .4), sg_helperDetectCutHeight, timeout, ourPos, enemy->pos);
    }

     //detectCutdebugH << (debug);
}

void gHelper::autoBrake()
{
    // Check if the cycle is still alive
    if (!aliveCheck())
    {
        return;
    }

    // Get the current used braking percentage of the cycle ( always out of 1 )
    REAL brakeUsagePercent = owner_->GetBrakingReservoir();

    // Check if the brake depletion is enabled
    bool cycleBrakeDeplete = true;
    if (sg_helperAutoBrakeDeplete && sg_helperAutoBrakeMin == 0 && sg_cycleBrakeDeplete < 0)
    {
        cycleBrakeDeplete = false;
    }

    // Check if the cycle is already braking and the used brake percentage is below the minimum brake limit
    // and brake depletion is enabled
    if (owner_->GetBraking() == true && brakeUsagePercent <= sg_helperAutoBrakeMin && cycleBrakeDeplete)
    {
        // Stop braking
        owner_->ActBot(&gCycle::s_brake, -1);
    }

    // Check if the cycle is not braking and the used brake percentage is above the maximum brake limit
    if (owner_->GetBraking() == false && brakeUsagePercent >= sg_helperAutoBrakeMax)
    {
        // brake
        owner_->ActBot(&gCycle::s_brake, 1);
    }
}

// This function adds tracers to the position of enemies to make it easier for the player to see their position.
void gHelper::enemyTracers(gHelperData & data, int detectionRange, REAL timeout)
{
    // If the player is not alive, return from the function
    if (!aliveCheck())
    {
        return;
    }
    // Iterate over all enemies
    for (auto enemy = enemies.allEnemies.begin(); enemy != enemies.allEnemies.end(); ++enemy)
    {
        // Get the current enemy cycle
        gCycle *other = *enemy;
        // If the enemy doesn't exist, skip to the next iteration
        if (!enemies.exist(other))
        {
            continue;
        }

        // Get the position of the enemy cycle
        eCoord enemyPos = other->Position();
        // Initialize RGB values for the tracer color
        REAL R = .1, G = .1, B = 0;
        // Check if the enemy is close to the player
        bool isClose = smartTurning->isClose(enemyPos, detectionRange + data.turnSpeedFactorF());
        // Check if the enemy cycle is faster than the player cycle
        bool enemyFaster = ((other->Speed() > ((*ownerSpeed) * sg_helperEnemyTracersSpeedMult)));
        // Check if the enemy cycle is a teammate
        bool isTeammate = (owner_->Team() == other->Team());

        // If the enemy is not close or not faster, return from the function
        if (!(isClose || enemyFaster))
            return;

        // Set tracer color based on enemy type
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

        // Draw the tracer line on the screen
        gHelperUtility::debugLine(gRealColor(R, G, B), sg_helperEnemyTracersHeight, timeout, (*ownerPos), enemyPos, sg_helperEnemyTracersBrightness);
    }
}

/**
 * gHelper::showTail
 *
 * Function that visualizes the tail of the owner. The tail is visualized as a line with a color
 * corresponding to the owner's color and a height of sg_helperShowTailHeight. The timeout of the visualization is
 * determined by the product of sg_helperShowTailTimeout and the result of calling the speedFactorF function on the data
 * object. The visualization is only shown if the owner is alive and its tail is moving. The function also checks if the
 * target can be seen by calling the canSeeTarget function with the tail position, owner position and
 * sg_helperShowTailPassthrough as arguments. If the target is not visible, the function returns without doing anything.
 *
 * @param data  a reference to a gHelperData object, containing information used by the helper functions
 */
void gHelper::showTail(gHelperData & data)
{
    if (!aliveCheck() || owner_->tailMoving != true)
    {
        return;
    }

    REAL timeout = sg_helperShowTailTimeout * data.speedFactorF();

    if (canSeeTarget((*tailPos), sg_helperShowTailPassthrough))
    {
        gHelperUtility::debugLine(gRealColor(owner_->color_.r, owner_->color_.g, owner_->color_.b), sg_helperShowTailHeight, timeout, (*ownerPos), (*tailPos));
    }
}

/**
 * gHelper::showEnemyTail
 *
 * Function that visualizes the tails of all enemies. The tails are visualized as
 * lines with a color corresponding to the color of the enemy and a height of sg_helperShowEnemyTailHeight. The timeout
 * of the visualization is determined by the product of sg_helperShowEnemyTailTimeoutMult and the result of calling the
 * speedFactorF function on the data object and the distance from the tail to the owner of the gHelper object. The
 * visualization is only shown if the enemy is alive and its tail is moving. The function first iterates through all the
 * enemies stored in the allEnemies member of the enemies object and checks if the enemy exists and if its tail is moving.
 * If the enemy exists and its tail is moving, the function calculates the distance to the tail and calls the debugLine
 * function of the gHelperUtility object to visualize the tail.
 *
 * @param data  a reference to a gHelperData object, containing information used by the helper functions
 */
void gHelper::showEnemyTail(gHelperData & data)
{
    // return if the current cycle is not alive
    if (!aliveCheck())
    {
        return;
    }
    // variables to store the distance to the tail and timeout
    REAL distanceToTail, timeout;

    // loop over all enemies in the game
    for (auto enemy = enemies.allEnemies.begin(); enemy != enemies.allEnemies.end(); ++enemy)
    {
        // get the cycle object
        gCycle *other = *enemy;
        // continue if the cycle doesn't exist or its tail is not moving
        if (!enemies.exist(other) || !other->tailMoving)
            continue;

        // calculate the distance to the tail
        distanceToTail = sg_helperShowEnemyTailDistanceMult * (eCoord::F(*ownerDir, (other->tailPos) - (*ownerPos)));
        // calculate the timeout
        timeout = fabs(distanceToTail) / 10 * data.speedFactorF();
        // draw a debug line to show the enemy's tail
        gHelperUtility::debugLine(
            gRealColor(other->color_.r, other->color_.g, other->color_.b),
            sg_helperShowEnemyTailHeight,
            timeout * sg_helperShowEnemyTailTimeoutMult,
            other->tailPos,
            other->tailPos,
            sg_helperShowEnemyTailBrightness
        );
    }
}

/**
 * gHelper::showTailTracer
 *
 * This function is used to show the tail tracer. It draws a white line at the tail position to indicate its
 * movement.
 *
 * @param data: Reference to the gHelperData object containing all the required data.
 *
 * The function first checks if the owner of this helper object is alive and if its tail is moving. If either of
 * these conditions is not met, the function returns immediately.
 *
 * The distance from the owner's position to the tail position is then calculated using the dot product of the
 * direction vector and the vector pointing from the owner's position to the tail position.
 *
 * The timeout value for the tail tracer is calculated as the absolute value of the distance to the tail divided
 * by the sg_helperShowTailTracerDistanceMult and multiplied by the speed factor of the data object.
 *
 * Finally, the debugLine function of the gHelperUtility class is called to draw a white line at the tail position
 * with a height equal to sg_helperShowTailTracerHeight and a timeout equal to the calculated timeout value
 * multiplied by sg_helperShowTailTracerTimeoutMult.
 */
void gHelper::showTailTracer(gHelperData & data)
{
    // checks if the object exists and if its tail is moving
    if (!aliveCheck() || !owner_->tailMoving)
        return;

    // calculates the distance between the object's position and its tail position
    REAL distanceToTail = sg_helperShowTailTracerTimeoutMult * eCoord::F(*ownerDir, (*tailPos) - (*ownerPos));
    // calculates the timeout for the debug line based on the distance and speed
    REAL timeout = fabs(distanceToTail) / sg_helperShowTailTracerDistanceMult * data.speedFactorF();

    // draws a debug line at the tail position with a specified height, color, and timeout
    gHelperUtility::debugLine(gRealColor(1, 1, 1), sg_helperShowTailTracerHeight, timeout * sg_helperShowTailTracerTimeoutMult, *tailPos, *tailPos);
}

// Function: findCorners
// Purpose: Finds the corners on the left and right of the vehicle.
// Input: data - A reference to the gHelperData object containing all necessary data for the helper.
// Output: None.
void gHelper::findCorners(gHelperData & data)
{
    // Find the left corner
    leftCorner.findCorner(data, data.sensors.getSensor(LEFT),this);
    // Find the right corner
    rightCorner.findCorner(data, data.sensors.getSensor(RIGHT),this);
}

// Function: showCorner
// Purpose: Visualizes the corner information.
// Input: data - A reference to the gHelperData object containing all necessary data for the helper.
//        corner - The corner information to be visualized.
//        timeout - The time for which the visualization should be shown.
// Output: None.
void gHelper::showCorner(gHelperData & data, gSmartTurningCornerData & corner, REAL timeout)
{
    // Check if the corner exists
    if (corner.exist)
    {
        // Calculate the timeout based on speed factor
        timeout = data.speedFactorF() * sg_helperShowCornersTimeout;
        // Check if the corner is close to the vehicle
        bool isClose = smartTurning->isClose(corner.currentPos, sg_helperShowCornersBoundary);
        // If the corner is close, visualize the corner
        if (isClose)
        {
            gHelperUtility::debugLine(gRealColor(1, .5, 0), sg_helperShowCornersHeight, timeout, corner.currentPos, corner.currentPos);
        }
    }
}

// Function: showCorners
// Purpose: Finds and visualizes the corners on the left and right of the vehicle.
// Input: data - A reference to the gHelperData object containing all necessary data for the helper.
// Output: None.
void gHelper::showCorners(gHelperData & data)
{
    // Check if the vehicle is alive and has a grid
    if (!aliveCheck())
    {
        return;
    }
    // Calculate the timeout based on speed factor
    REAL timeout = data.speedFactorF() * sg_helperShowCornersTimeout;
    // Find the corners
    findCorners(data);
    // Visualize the left corner
    showCorner(data, leftCorner, timeout);
    // Visualize the right corner
    showCorner(data, rightCorner, timeout);
}


/**
* @brief Display a visual representation of the hit data from the sensors
* If the cycle is alive and driving straight, the function calculates the distance
* to the wall in front using the sensor information. If the wall is close enough,
* it draws lines to represent the wall's location. The height, color and timeout of
* these lines can be configured using config items. The lines can start from the
* hit position or the cycle's position, which can also be configured using config items.
* @param data The data object containing information about the cycle and its surroundings
*/
void gHelper::showHit(gHelperData & data)
{
    // check if the owner is alive and driving straight
    if (!aliveCheck() || !drivingStraight())
    {
        return;
    }

    // get the front hit distance and check if it's close to the wall
    REAL frontHit = data.sensors.getSensor(FRONT)->hit;
    bool wallClose = frontHit < data.turnSpeedFactorF() * sg_showHitDataRange;

    // calculate the timeout value
    REAL timeout = data.speedFactorF() * sg_showHitDataTimeout;

    // write the front hit distance to the stream
    sg_helperShowHitFrontDistH << (frontHit);

    // return if the wall is not close
    if (!wallClose)
    {
        return;
    }

    // get the front before hit position
    eCoord frontBeforeHit = data.sensors.getSensor(FRONT)->before_hit;

    // draw a line from the owner's current position to the front before hit position
    gHelperUtility::debugLine(gRealColor(1, .5, 0), sg_showHitDataHeightFront, timeout, (*ownerPos), frontBeforeHit);

    // check if the start position is at the hit position or not
    if (sg_helperShowHitStartAtHitPos)
    {
        // draw debug lines from the front before hit position in the left and right directions
        showHitDebugLines(frontBeforeHit, owner_->Direction(), timeout, data, sg_showHitDataRecursion, LEFT);
        showHitDebugLines(frontBeforeHit, owner_->Direction(), timeout, data, sg_showHitDataRecursion, RIGHT);
    }
    else
    {
        // draw debug lines from the owner's current position in the left and right directions
        showHitDebugLines(*ownerPos, owner_->Direction().Turn(LEFT), timeout, data, sg_showHitDataRecursion, LEFT);
        showHitDebugLines(*ownerPos, owner_->Direction().Turn(RIGHT), timeout, data, sg_showHitDataRecursion, RIGHT);
    }
}

// gHelper::showHitDebugLines
// This function is used to visualize the sensor information of a cycle by drawing lines on the game screen.
//
// Parameters:
//   currentPos: The current position of the cycle.
//   initDir: The initial direction of the sensor.
//   timeout: The duration for which the lines will be displayed.
//   data: The data structure that holds all the helper data.
//   recursion: The number of times the function will recurse to visualize multiple sensor hits.
//   sensorDir: The direction of the sensors, either left or right.
void gHelper::showHitDebugLines(eCoord currentPos, eCoord initDir, REAL timeout, gHelperData & data, int recursion, int sensorDir)
{
    // If the recursion limit is reached, exit the function.
    if (recursion <= 0)
        return;

    // Decrement the recursion counter.
    recursion--;

    // Get the direction of the new sensor by turning the initial direction by a specified angle in the opposite direction 
    // of the sensorDir.
    eCoord newDir = initDir.Turn(eCoord(0, sensorDir * -1));

    // Get the information of the sensor at the current position and direction.
    gSensor *sensor = data.sensors.getSensor(currentPos, initDir);
    eCoord hitPos = sensor->before_hit;
    REAL hitDistance = sensor->hit;

    // Check if the hit distance is greater than a certain value.
    bool open = hitDistance > data.turnSpeedFactorF() * sg_showHitDataFreeRange;

    // Draw a green line if the hit distance is greater than the specified value, indicating that the path is clear.
    if (open)
    {
        gHelperUtility::debugLine(gRealColor(0, 1, 0), sg_showHitDataHeightSides, timeout, currentPos, hitPos);
    }
    // Draw a red line if the hit distance is smaller than the specified value, indicating an obstacle in the way.
    else
    {
        gHelperUtility::debugLine(gRealColor(1, 0, 0), sg_showHitDataHeightSides, timeout, currentPos, hitPos);
    }

    // Recursively call the function to visualize multiple sensor hits.
    showHitDebugLines(hitPos, newDir, timeout, data, recursion, sensorDir);
}

// Function to check if the owner is alive and exists on the grid.
bool gHelper::aliveCheck()
{
    // Return true if the owner exists, is alive, and grid exist.
    return owner_ && owner_->Alive() && owner_->Grid();
}

void gHelper::Activate()
{
    REAL start = tRealSysTimeFloat();
    ownerPosH << roundeCoord(*ownerPos);
    ownerDirH << roundeCoord(*ownerDir);
    tailPosH << roundeCoord(owner_->tailPos);
    tailDirH << roundeCoord(owner_->tailDir);

    if (!aliveCheck())
    {
        return;
    }
    owner_->localCurrentTime = se_GameTime();

    enemies.detectEnemies();

    if (sg_helperSmartTurning)
        smartTurning->Activate(*data_stored);

    if (sg_pathHelper)
        pathHelper->Activate(*data_stored);

    if (sg_tailHelper)
        tailHelper->Activate(*data_stored);

    if (sg_zoneHelper)
        zoneHelper->Activate(*data_stored);

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
    REAL time = tRealSysTimeFloat() - start;
    sg_helperActivateTimeH << (time);
}

    gHelper &gHelper::Get(gCycle * cycle)
    {
        tASSERT(cycle);

        // create
        if (cycle->helper_.get() == 0)
            cycle->helper_.reset(new gHelper(cycle));

        return *cycle->helper_;
    }

    gHelper::~gHelper()
    {
    }
