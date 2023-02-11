#include "gHelperVar.h"
#include "nConfig.h"

namespace helperConfig {
bool sg_helper = false;
static tConfItem<bool> sg_helperConf("HELPER", sg_helper);

REAL sg_helperBrightness = 1;
static tConfItem<REAL> sg_helperBrightnessConf("HELPER_BRIGHTESS", sg_helperBrightness);

bool sg_helperSensorsZoneDetection = false;
static tConfItem<bool> sg_helperSensorsZoneDetectionC("HELPER_SENSORS_ZONE_DETECTION", sg_helperSensorsZoneDetection);

REAL sg_helperSensorRange = 1000;
static tConfItem<REAL> sg_helperSensorRangeConf("HELPER_SENSOR_RANGE", sg_helperSensorRange);
bool sg_helperSensorLightUsageMode = false;
static tConfItem<bool> sg_helperSensorLightUsageModeConf("HELPER_SENSOR_LIGHT_USAGE_MODE", sg_helperSensorLightUsageMode);
bool sg_helperSensorDiagonalMode = false;
static tConfItem<bool> sg_helperSensorDiagonalModeConf("HELPER_SENSOR_DIAGONAL_MODE", sg_helperSensorDiagonalMode);

bool sg_helperDebug = false;
static tConfItem<bool> sg_helperDebugConf("HELPER_DEBUG", sg_helperDebug);
bool sg_helperDebugSpamFilter = true;
static tConfItem<bool> sg_helperDebugSpamFilterConf("HELPER_DEBUG_SPAM_FILTER", sg_helperDebugSpamFilter);
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
