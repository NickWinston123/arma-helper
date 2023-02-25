#include "defs.h"
#include "tString.h"

#ifndef ArmageTron_GHELPER_VAR
#define ArmageTron_GHELPER_VAR
enum turnDirection
{
    NONE  = 0,
    LEFT  = -1,
    FRONT = 0,
    RIGHT = 1,
    BOTH  = 2
};


static float lastHelperDebugMessageTimeStamp;
static std::string lastHelperDebugMessage = "", lastHelperDebugSender = "";

namespace helperConfig {
    extern bool sg_helper ;
    extern REAL sg_helperBrightness ;
    extern REAL sg_helperSensorRange ;
    extern bool sg_helperSensorsZoneDetection ;
    extern bool sg_helperSensorLightUsageMode ;
    extern bool sg_helperSensorDiagonalMode ;

    extern bool sg_helperDebug ;
    extern bool sg_helperDebugSpamFilter;
    extern tString sg_helperDebugIgnoreList ;
    extern REAL sg_helperDebugDelay ;
    extern bool sg_helperDebugTimeStamp ;

    extern bool sg_helperAI ;
    extern REAL sg_helperAITime ;

    extern bool sg_helperSmartTurning ;

    extern bool sg_helperSmartTurningFrontBot ;
    extern REAL sg_helperSmartTurningFrontBotThinkRange ;
    extern bool sg_helperSmartTurningFrontBotTurnOnce ;
    extern REAL sg_helperSmartTurningFrontBotActivationRubber ;
    extern REAL sg_helperSmartTurningFrontBotActivationSpace ;
    extern REAL sg_helperSmartTurningFrontBotDisableTime ;

    extern bool sg_helperSmartTurningTrapped;
    extern bool sg_helperSmartTurningClosedIn ;

    extern bool sg_helperSmartTurningAutoTrace ;
    extern REAL sg_helperSmartTurningAutoTraceDistance ;

    extern bool sg_helperSmartTurningSurvive ;
    extern bool sg_helperSmartTurningSurviveTrace ;
    extern REAL sg_helperSmartTurningSurviveTraceTurnTime ;
    extern REAL sg_helperSmartTurningSurviveTraceActiveTime ;
    extern REAL sg_helperSmartTurningSurviveTraceCloseFactor ;

    extern bool sg_helperSmartTurningOpposite ;
    extern REAL sg_helperSmartTurningClosedInMult ;

    extern REAL sg_helperSmartTurningRubberTimeMult ;
    extern REAL sg_helperSmartTurningRubberFactorMult ;

    extern REAL sg_helperSmartTurningSpace ;
    extern bool sg_helperSmartTurningPlan ;

    extern bool sg_helperAutoBrake ;
    extern bool sg_helperAutoBrakeDeplete ;
    extern REAL sg_helperAutoBrakeMin ;
    extern REAL sg_helperAutoBrakeMax ;

    extern bool sg_helperSmartTurningFollowTail ;
    extern REAL sg_helperSmartTurningFollowTailDelayMult ;
    extern REAL sg_helperSmartTurningFollowTailFreeSpaceMult ;

    extern bool sg_tailHelper ;
    extern REAL sg_tailHelperBrightness ;
    extern REAL sg_tailHelperDelay ;
    extern REAL sg_tailHelperUpdateTime ;

    extern bool sg_pathHelper ;
    extern bool sg_pathHelperRenderPath ;
    extern bool sg_pathHelperShowTurn ;
    extern bool sg_pathHelperShowTurnAct ;
    extern REAL sg_pathHelperShowTurnAhead ;

    extern int sg_pathHelperMode ;
    extern REAL sg_pathHelperAutoCloseDistance ;
    extern REAL sg_pathHelperUpdateTime ;
    extern REAL se_pathHeight ;
    extern REAL se_pathBrightness ;
    extern REAL sg_pathHelperUpdateDistance ;

    extern bool sg_zoneHelper;
    extern bool sg_zoneHelperOwnerOnly;

    extern bool sg_helperShowCorners ;
    extern bool sg_helperShowCornersPassed ;
    extern REAL sg_helperShowCornersPassedRange ;
    extern REAL sg_helperShowCornersBoundary ;
    extern REAL sg_helperShowCornersBoundaryPassed ;
    extern REAL sg_helperShowCornersTimeout ;
    extern REAL sg_helperShowCornersHeight ;

    extern bool sg_helperDetectCut ;
    extern REAL sg_helperDetectCutDetectionRange ;
    extern REAL sg_helperDetectCutTimeout ;
    extern REAL sg_helperDetectCutHeight ;
    extern REAL sg_helperDetectCutReact ;

    extern bool sg_helperEnemyTracers ;
    extern REAL sg_helperEnemyTracersHeight ;
    extern REAL sg_helperEnemyTracersDetectionRange ;
    extern REAL sg_helperEnemyTracersSpeedMult ;
    extern REAL sg_helperEnemyTracersPassthrough ;
    extern REAL sg_helperEnemyTracersDelayMult ;
    extern REAL sg_helperEnemyTracersTimeout ;
    extern REAL sg_helperEnemyTracersBrightness ;

    extern bool sg_helperTurningBot;
    extern REAL sg_helperTurningBotActivationSpace;
    extern REAL sg_helperTurningBotActivationRubber;
    extern int sg_helperTurningBotTurns;

    extern bool sg_helperShowTail ;
    extern REAL sg_helperShowTailHeight ;
    extern REAL sg_helperShowTailPassthrough ;
    extern REAL sg_helperShowTailTimeout ;

    extern bool sg_helperShowEnemyTail ;
    extern REAL sg_helperShowEnemyTailHeight ;
    extern REAL sg_helperShowEnemyTailDistanceMult ;
    extern REAL sg_helperShowEnemyTailTimeoutMult ;
    extern REAL sg_helperShowEnemyTailBrightness ;

    extern bool sg_helperShowTailTracer ;
    extern REAL sg_helperShowTailTracerHeight ;
    extern REAL sg_helperShowTailTracerBrightness ;
    extern REAL sg_helperShowTailTracerTimeoutMult ;
    extern REAL sg_helperShowTailTracerDistanceMult ;

    extern bool sg_helperShowHit ;
    extern REAL sg_showHitDataHeight ;
    extern REAL sg_showHitDataHeightFront ;
    extern REAL sg_showHitDataHeightSides ;
    extern REAL sg_showHitDataBrightness ;
    extern REAL sg_showHitDataRange ;
    extern REAL sg_showHitDataFreeRange ;
    extern int sg_showHitDataRecursion ;
    extern REAL sg_showHitDataTimeout ;

    extern bool sg_helperShowHitFrontLine;
    extern REAL sg_helperShowHitFrontLineHeight;
    extern bool sg_helperShowHitStartAtHitPos ;
    extern bool sg_helperCurrentTimeLocal ;

}
#endif
