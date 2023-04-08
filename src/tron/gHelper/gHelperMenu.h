#include "gHelper.h"
#include "gHelperHud.h"
#include "nConfig.h"
#include "uMenu.h"

using namespace helperConfig;
void enemyTracersMenu() {
    uMenu enemyTracersMenu("Enemy Tracers Settings");

    uMenuItemReal enemyTracersDetectionRange(&enemyTracersMenu, "Tracer Detection Range", "Range for displaying enemy tracers", sg_helperEnemyTracersDetectionRange,0, 10, 0.1);
    uMenuItemReal enemyTracersSpeedMult(&enemyTracersMenu, "Tracer Speed Multiplier", "Speed multiplier for displaying enemy tracers", sg_helperEnemyTracersSpeedMult,0, 10, 0.1);
    uMenuItemReal enemyTracersPassthrough(&enemyTracersMenu, "Tracer Passthrough", "Passthrough for displaying enemy tracers", sg_helperEnemyTracersPassthrough,0, 10, 0.1);
    uMenuItemReal enemyTracersDelayMult(&enemyTracersMenu, "Tracer Delay Multiplier", "Delay multiplier for displaying enemy tracers", sg_helperEnemyTracersDelayMult,0, 10, 0.1);
    uMenuItemReal enemyTracersTimeout(&enemyTracersMenu, "Tracer Timeout", "Timeout for displaying enemy tracers", sg_helperEnemyTracersTimeout,0, 10, 0.1);
    uMenuItemReal enemyTracersBrightness(&enemyTracersMenu, "Tracer Brightness", "Brightness for displaying enemy tracers", sg_helperEnemyTracersBrightness,0, 10, 0.1);
    uMenuItemReal enemyTracersHeight(&enemyTracersMenu, "Tracer Height", "Height for displaying enemy tracers", sg_helperEnemyTracersHeight,0, 10, 0.1);
    uMenuItemToggle enemyTracers(&enemyTracersMenu, "Show Enemy Tracers", "Toggle display of enemy tracers", sg_helperEnemyTracers);

    enemyTracersMenu.Enter();
}

void enemyTailMenu() {
    uMenu enemyTailMenu("Enemy Tail Settings");

    uMenuItemReal enemyTailDistanceMult(&enemyTailMenu, "Enemy Tail Distance Multiplier", "Distance multiplier for displaying enemy tail", sg_helperShowEnemyTailDistanceMult, 0, 10, 0.1);
    uMenuItemReal enemyTailTimeoutMult(&enemyTailMenu, "Enemy Tail Timeout Multiplier", "Timeout multiplier for displaying enemy tail", sg_helperShowEnemyTailTimeoutMult, 0, 10, 0.1);
    uMenuItemReal enemyTailBrightness(&enemyTailMenu, "Enemy Tail Brightness", "Brightness for displaying enemy tail", sg_helperShowEnemyTailBrightness, 0, 10, 0.1);
    uMenuItemReal enemyTailHeight(&enemyTailMenu, "Enemy Tail Height", "Height for displaying enemy tail", sg_helperShowEnemyTailHeight, 0, 10, 0.1);
    uMenuItemToggle showEnemyTail(&enemyTailMenu, "Show Enemy Tail", "Toggle display of enemy tail", sg_helperShowEnemyTail);

    enemyTailMenu.Enter();
}

void enemyDetectCutMenu() {
    uMenu detectCutMenu("Enemy Cut Detection Settings");

    uMenuItemReal detectCutRange(&detectCutMenu, "Detection Range", "Range for detecting enemy cuts", sg_helperDetectCutDetectionRange, 0, 300, 1);
    uMenuItemReal detectCutTimeout(&detectCutMenu, "Timeout", "Timeout for detecting enemy cuts", sg_helperDetectCutTimeout, 0, 1, 0.001);
    uMenuItemReal detectCutHeight(&detectCutMenu, "Height", "Height for detecting enemy cuts", sg_helperDetectCutHeight, 0, 10, 0.1);
    uMenuItemReal detectCutReact(&detectCutMenu, "React Time", "Time to react to enemy cuts", sg_helperDetectCutReact, 0, 1, 0.01);
    uMenuItemToggle detectCut(&detectCutMenu, "Detect Cut", "Toggle enemy cut detection", sg_helperDetectCut);

    detectCutMenu.Enter();
}

void helperEnemyMenu()
{
    uMenu enemyMenu("Enemy Menu");

    uMenuItemFunction tracersSettings(&enemyMenu, sg_helperEnemyTracers ? "0x00dd00Tracers" : "0xdd0000Tracers", "Adjust settings for tracers", &enemyTracersMenu);
    uMenuItemFunction tailSettings(&enemyMenu, sg_helperShowEnemyTail ? "0x00dd00Show Tail" : "0xdd0000Show Tail", "Adjust settings for tail", &enemyTailMenu);
    uMenuItemFunction detectCutSettings(&enemyMenu, sg_helperDetectCut ? "0x00dd00Detect Cut" : "0xdd0000Detect Cut", "Adjust settings for detect cut", &enemyDetectCutMenu);

    enemyMenu.Enter();
}

void helperCornersMenu() {
    uMenu cornersMenu("Corners Settings");

    uMenuItemReal cornersPassedRange(&cornersMenu, "Passed Corners Range", "Range for showing passed corners", sg_helperShowCornersPassedRange, 0, 10, 0.1);
    uMenuItemReal cornersBoundaryPassed(&cornersMenu, "Passed Corners Boundary", "Boundary for showing passed corners", sg_helperShowCornersBoundaryPassed, 0, 10, 0.1);
    uMenuItemReal cornersBoundary(&cornersMenu, "Corners Boundary", "Boundary for showing corners", sg_helperShowCornersBoundary, 0, 10, 0.1);
    uMenuItemReal cornersHeight(&cornersMenu, "Corners Height", "Height for showing corners", sg_helperShowCornersHeight, 0, 10, 0.1);
    uMenuItemReal cornersTimeout(&cornersMenu, "Corners Timeout", "Timeout for showing corners", sg_helperShowCornersTimeout, 0, 10, 0.1);
    uMenuItemToggle showPassedCorners(&cornersMenu, "Show Passed Corners", "Toggle display of passed corners", sg_helperShowCornersPassed);
    uMenuItemToggle showCorners(&cornersMenu, "Show Corners", "Toggle display of corners", sg_helperShowCorners);

    cornersMenu.Enter();
}

void helperShowHitMenu() {
    uMenu showHitMenu("Show Hit Settings");

    uMenuItemInt showHitStartAtHitPos(&showHitMenu, "Start at Hit Mode", "Position where calculation for side debug lines begins. 0 - At front hit point, 1 - At owners position, 2 - Both ", sg_helperShowHitStartMode,0, 2, 1);

    uMenuItemReal showHitHeightFront(&showHitMenu, "Hit Height (Front)", "Height for displaying hit in front", sg_showHitDataHeightFront,0, 5, 0.1);
    uMenuItemReal showHitHeightSides(&showHitMenu, "Hit Height (Sides)", "Height for displaying hit on sides", sg_showHitDataHeightSides,0, 5, 0.1);
    uMenuItemReal showHitRange(&showHitMenu, "Hit Range", "Range for displaying hit", sg_showHitDataRange,0, 1000, 5);
    uMenuItemReal showHitFreeRange(&showHitMenu, "Hit Open Range", "Open range for displaying hit", sg_showHitDataFreeRange,0, 20, 1);
    uMenuItemInt showHitRecursion(&showHitMenu, "Hit Recursion", "Recursion level for displaying hit", sg_showHitDataRecursion, 1, 10);
    uMenuItemReal showHitTimeout(&showHitMenu, "Hit Timeout", "Timeout for displaying hit", sg_showHitDataTimeout,0, 10, 0.1);
    uMenuItemToggle showHit(&showHitMenu, "Show Hit", "Toggle display of hit", sg_helperShowHit);

    showHitMenu.Enter();
}

void tailHelperMenu () {
    uMenu tailHelperMenu("Tail Helper Settings");

    uMenuItemToggle tailHelperToggle(&tailHelperMenu, "Tail Helper", "Toggle tail helper on/off", sg_tailHelper);
    uMenuItemReal tailHelperBrightness(&tailHelperMenu, "Brightness", "Adjust tail helper brightness", sg_tailHelperBrightness, 0, 5, 0.1);

    tailHelperMenu.Enter();
}

void tailTracerMenu() {
    uMenu tailTracerMenu("Tail Tracer Settings");

    uMenuItemToggle showTailTracer(&tailTracerMenu, "Show Tail Tracer", "Toggle tail tracer on/off", sg_helperShowTailTracer);
    uMenuItemReal tailTracerHeight(&tailTracerMenu, "Tail Tracer Height", "Height of tail tracer", sg_helperShowTailTracerHeight, 0, 10, 0.1);
    uMenuItemReal tailTracerTimeoutMult(&tailTracerMenu, "Tail Tracer Timeout Multiplier", "Timeout multiplier for tail tracer", sg_helperShowTailTracerTimeoutMult, 0, 10, 0.1);
    uMenuItemReal tailTracerDistanceMult(&tailTracerMenu, "Tail Tracer Distance Multiplier", "Distance multiplier for tail tracer", sg_helperShowTailTracerDistanceMult, 0, 100, 0.1);

    tailTracerMenu.Enter();
}

void helperTailMenu () {
    uMenu tailMenu("Tail Settings");

    uMenuItemFunction tailTracer(&tailMenu, "Tail Tracer", "Settings for Tail Tracer", &tailTracerMenu);
    uMenuItemFunction tailHelper(&tailMenu, "Tail Helper", "Settings for Tail Helper", &tailHelperMenu);
    uMenuItemReal tailPassthrough(&tailMenu, "Tail Passthrough", "Passthrough for displaying tail", sg_helperShowTailPassthrough,0, 10, 0.1);
    uMenuItemReal tailHeight(&tailMenu, "Tail Height", "Height for displaying tail", sg_helperShowTailHeight,0, 10, 0.1);
    uMenuItemReal tailTimeout(&tailMenu, "Tail Timeout", "Timeout for displaying tail", sg_helperShowTailTimeout,0, 10, 0.1);
    uMenuItemToggle showTail(&tailMenu, "Show Tail", "Toggle display of tail", sg_helperShowTail);

    tailMenu.Enter();
}

void helperPathMenu() {
    uMenu pathMenu("Path Settings");

    uMenuItemReal showTurnAhead(&pathMenu, "Show Turn Ahead", "Distance from owner to current path target must be less than this value before rendering turn", sg_pathHelperShowTurnAhead, 0, 30, 0.1);
    uMenuItemToggle showTurn(&pathMenu, "Show Turn", "Show turn you should take to follow path", sg_pathHelperShowTurn);
    uMenuItemToggle pathTurn(&pathMenu, "Render Path", "Rendering the generated path", sg_pathHelperRenderPath);
    uMenuItemReal autoRange(&pathMenu, "Auto Range", "Distance enemy needs to less than from player before switching from tail to enemy", sg_pathHelperAutoCloseDistance, 0, 500, 10);
    uMenuItemInt mode(&pathMenu, "Mode", "0 = Auto (Switch Between tail and close enemies), 1 = Tail Mode, 2 = Closest Enemy Mode, 3 = Corner Mode", sg_pathHelperMode, 0, 3);
    uMenuItemReal updateDistance(&pathMenu, "Update Distance", "Set the distance the target needs to move before a new path is generated", sg_pathHelperUpdateDistance, 0, 50, 5);
    uMenuItemReal pathHeight(&pathMenu, "Path Height", "Set the height of the drawn path", se_pathHeight, 0, 5, 0.1);
    uMenuItemReal pathBrightness(&pathMenu, "Path Brightness", "Set the brightness of the drawn path", se_pathBrightness, 0, 5, 0.1);
    uMenuItemToggle pathHelper(&pathMenu, "Path Helper", "Toggle path helper on/off", sg_pathHelper);

    pathMenu.Enter();
}

void helperAutoBrakeMenu(){
    uMenu helperAutoBrakeMenu("Auto Brake");

    // uMenuItemToggle autoBrakeDeplete(&helperAutoBrakeMenu, "Auto Brake Deplete", "Toggle auto brake deplete on/off - when on, will try to keep brake down", sg_helperAutoBrakeDeplete);
    uMenuItemReal autoBrakeMin(&helperAutoBrakeMenu, "Auto Brake Min", "Minimum value that needs to be reached before un braking", sg_helperAutoBrakeMin, -0.1, 1, 0.05);
    uMenuItemReal autoBrakeMax(&helperAutoBrakeMenu, "Auto Brake Max", "Maximum value that can be reached before braking", sg_helperAutoBrakeMax, 0, 1.1, 0.05);
    uMenuItemToggle autoBrake(&helperAutoBrakeMenu, "Auto Brake", "Toggle auto brake on/off", sg_helperAutoBrake);

    helperAutoBrakeMenu.Enter();
}


void helperSelfMenu()
{
    uMenu helperSelfMenu("Self");
    uMenuItemFunction autoBrakeMenu(&helperSelfMenu, sg_helperAutoBrake ? "0x00dd00Auto Brake" : "0xdd0000Auto Brake", "Settings for Auto Brake", &helperAutoBrakeMenu);

    uMenuItemFunction cornersMenu(&helperSelfMenu, "Corners", "Settings for Corners", &helperCornersMenu);
    uMenuItemFunction showHitMenu(&helperSelfMenu, sg_helperShowHit ? "0x00dd00Show Hit" : "0xdd0000Show Hit", "Settings for Show Hit", &helperShowHitMenu);
    uMenuItemFunction tailMenu(&helperSelfMenu, sg_helperShowTail ? "0x00dd00Tail" : "0xdd0000Tail", "Settings for Tail", &helperTailMenu);
    uMenuItemFunction pathMenu(&helperSelfMenu, sg_pathHelper ? "0x00dd00Path" : "0xdd0000Path", "Settings for Path", &helperPathMenu);

    helperSelfMenu.Enter();
}

void helperSmartTurningSurviveTraceMenu(){
    uMenu helperSmartTurningSurviveTraceMenu("Trace");

    uMenuItemReal surviveTraceTurnTime(&helperSmartTurningSurviveTraceMenu, "Survive Trace Turn Time", "Turn time for survive trace", sg_helperSmartTurningSurviveTraceTurnTime, 0, 10, 0.1);
    uMenuItemReal surviveTraceActiveTime(&helperSmartTurningSurviveTraceMenu, "Survive Trace Active Time", "Active time for survive trace", sg_helperSmartTurningSurviveTraceActiveTime, 0, 10, 0.1);
    uMenuItemReal surviveTraceCloseFactor(&helperSmartTurningSurviveTraceMenu, "Survive Trace Close Factor", "How close you must be to a corner before logic begins", sg_helperSmartTurningSurviveTraceCloseFactor, 0, 10, 0.1);
    uMenuItemToggle surviveTrace(&helperSmartTurningSurviveTraceMenu, "Survive Trace", "Toggle survive trace on/off", sg_helperSmartTurningSurviveTrace);

    helperSmartTurningSurviveTraceMenu.Enter();
}

void helperSmartTurningSurviveFrontBotMenu(){
    uMenu helperSmartTurningSurviveFrontBotMenu("Front Bot");

    uMenuItemReal frontBotActivationRange(&helperSmartTurningSurviveFrontBotMenu, "Front Bot Think Range", "Think range for front bot", sg_helperSmartTurningFrontBotThinkRange, 0, 10, 0.1);
    uMenuItemReal frontBotActivationRubber(&helperSmartTurningSurviveFrontBotMenu, "Front Bot Activation Rubber", "Activation rubber for front bot", sg_helperSmartTurningFrontBotActivationRubber, 0, 1, 0.01);
    uMenuItemReal frontBotActivationSpace(&helperSmartTurningSurviveFrontBotMenu, "Front Bot Activation Space", "Activation space for front bot", sg_helperSmartTurningFrontBotActivationSpace, 0, 10, 0.1);
    uMenuItemReal frontBotDisableTime(&helperSmartTurningSurviveFrontBotMenu, "Front Bot Disable Time", "Disable time for front bot", sg_helperSmartTurningFrontBotDisableTime, 0, 10, 0.1);
    uMenuItemToggle smartTurningFrontBot(&helperSmartTurningSurviveFrontBotMenu, "Smart Turning Front Bot", "Toggle smart turning front bot on/off", sg_helperSmartTurningFrontBot);

    helperSmartTurningSurviveFrontBotMenu.Enter();
}


void helperSmartTurningSurviveFollowTailMenu(){
    uMenu helperSmartTurningSurviveFollowTailMenu("Follow Tail");

   uMenuItemReal followTailDelayMult(&helperSmartTurningSurviveFollowTailMenu, "Follow Tail Delay Mult", "Multiplier for follow tail delay", sg_helperSmartTurningFollowTailDelayMult, 0, 10, 0.1);
    uMenuItemReal followTailFreeSpaceMult(&helperSmartTurningSurviveFollowTailMenu, "Follow Tail Free Space Mult", "Multiplier for follow tail free space", sg_helperSmartTurningFollowTailFreeSpaceMult, 0, 10, 0.1);
    uMenuItemToggle followTail(&helperSmartTurningSurviveFollowTailMenu, "Follow Tail", "Toggle follow tail on/off", sg_helperSmartTurningFollowTail);

    helperSmartTurningSurviveFollowTailMenu.Enter();
}

void helperSmartTurningMenu() {
    uMenu smartTurningMenu("Smart Turning Settings");

    uMenuItemReal closedInMult(&smartTurningMenu, "Closed In Mult", "Multiplier for closed in", sg_helperSmartTurningClosedInMult, 0, 10, 0.1);
    uMenuItemToggle closedIn(&smartTurningMenu, sg_helperSmartTurningClosedIn ? "0x00dd00Closed In" : "0xdd0000Closed In", "Toggle closed in on/off", sg_helperSmartTurningClosedIn);
    uMenuItemReal rubberTimeMult(&smartTurningMenu, "Rubber Time Mult", "Multiplier for rubber time", sg_helperSmartTurningRubberTimeMult, 0, 10, 0.1);
    uMenuItemReal rubberFactorMult(&smartTurningMenu, "Rubber Factor Mult", "Multiplier for rubber factor", sg_helperSmartTurningRubberFactorMult, 0, 10, 0.1);
    uMenuItemReal space(&smartTurningMenu, "Space", "Smart turning space", sg_helperSmartTurningSpace, 0, 10, 0.1);
    uMenuItemToggle plan(&smartTurningMenu, sg_helperSmartTurningPlan ? "0x00dd00Plan" : "0xdd0000Plan", "Toggle plan on/off", sg_helperSmartTurningPlan);
    uMenuItemToggle survive(&smartTurningMenu, sg_helperSmartTurningSurvive ? "0x00dd00Survive" : "0xdd0000Survive", "Toggle survive on/off", sg_helperSmartTurningSurvive);
    uMenuItemToggle opposite(&smartTurningMenu, sg_helperSmartTurningOpposite ? "0x00dd00Opposite" : "0xdd0000Opposite", "Toggle opposite on/off", sg_helperSmartTurningOpposite);

    uMenuItemFunction followTailMenu(&smartTurningMenu, sg_helperSmartTurningFollowTail ? "0x00dd00Follow Tail" : "0xdd0000Follow Tail", "Settings for Follow Tail", &helperSmartTurningSurviveFollowTailMenu);
    uMenuItemFunction surviveTraceMenu(&smartTurningMenu, sg_helperSmartTurningSurviveTrace ? "0x00dd00Survive Trace" : "0xdd0000Survive Trace", "Settings for Survive Trace", &helperSmartTurningSurviveTraceMenu);
    uMenuItemFunction frontBotMenu(&smartTurningMenu, sg_helperSmartTurningFrontBot ? "0x00dd00Front Bot" : "0xdd0000Front Bot", "Settings for Front Bot", &helperSmartTurningSurviveFrontBotMenu);

    uMenuItemToggle smartTurning(&smartTurningMenu, "Smart Turning", "Toggle smart turning on/off", sg_helperSmartTurning);

    smartTurningMenu.Enter();
}

void helperDebugMenu() {
    uMenu smartDebugMenu("Debug Menu");

    uMenuItemReal helperDebugDelay(&smartDebugMenu,
                                    "Helper Debug Delay",
                                    "Helper Debug Message delay",
                                    sg_helperDebugDelay,
                                    0,
                                    1,
                                    0.01);

    uMenuItemToggle helperDebugTimeStamp(&smartDebugMenu,
                                        "Helper Debug Timestamp",
                                        "Helper Debug Timestamp",
                                        sg_helperDebugTimeStamp);

    uMenuItemString helperDebugIgnoreLis(&smartDebugMenu, "Helper Debug Ignore",
                                        "Comma delimited list, any helper debug with a sensor matching any values will be suppressed",
                                        sg_helperDebugIgnoreList);

    uMenuItemToggle helperDebug(&smartDebugMenu,
                                       "Helper Debug",
                                       "Enable Helper Debug",
                                       sg_helperDebug);
    smartDebugMenu.Enter();

}

void helperConfigMenu() {
    uMenu helperConfigMenu("Config Settings");

    uMenuItemReal brightness(&helperConfigMenu, "Brightness", "Adjust the brightness of debug lines", sg_helperBrightness, 0, 10, 0.1);
    uMenuItemReal sensorRange(&helperConfigMenu, "Sensor Range", "Adjust the range of sensors", sg_helperSensorRange, 0, 2000, 10);
    uMenuItemToggle sensorLightUsage(&helperConfigMenu, "Sensor Light Usage Mode", "If sg_helperHudSize, only one sensor object is used. If disabled, an instance is created eachtime a sensor is needed.", sg_helperSensorLightUsageMode);

    helperConfigMenu.Enter();
}

void helperZoneMenu() {
    uMenu helperZoneMenu("Zone Helper Settings");

    uMenuItemToggle zoneHelper(&helperZoneMenu, "Zone Helper", "Enable or disable zone helper", sg_zoneHelper);
    helperZoneMenu.Enter();
}

void helperExperimentalMenu(){
    uMenu helperExperimentalMenu("Experimental Settings");

    uMenuItemFunction zoneMenu(&helperExperimentalMenu,
                               "Zone Helper",
                               "Zone Helper Settings",
                               helperZoneMenu);

    helperExperimentalMenu.Enter();
}

void helperHudMenu() {
    uMenu helperHudMenu("Helper HUD Settings");
    uMenuItemToggle helperHud(&helperHudMenu, "Helper HUD", "Toggle the visibility of the helper HUD", sg_helperHud);

    uMenuItemReal helperHudX(&helperHudMenu, "Helper HUD X Position", "Adjust the X position of the helper HUD", sg_helperHudX, -2, 2, 0.01);
    uMenuItemReal helperHudY(&helperHudMenu, "Helper HUD Y Position", "Adjust the Y position of the helper HUD", sg_helperHudY, -2, 2, 0.01);
    uMenuItemReal helperHudSize(&helperHudMenu, "Helper HUD Size", "Adjust the size of the helper HUD", sg_helperHudSize, 0, 5, 0.01);

    helperHudMenu.Enter();
}

void helperMenu()
{
    uMenu helperSettingsMenu("Helper Settings");

    uMenuItemFunction experimentalMenu(&helperSettingsMenu,
                            "Helper Experimental",
                            "Helper Experimental Settings",
                            helperExperimentalMenu);

    uMenuItemFunction hudMenu(&helperSettingsMenu,
                            sg_helperHud ? "0x00dd00Helper Hud" : "0xdd0000Helper Hud",
                            "Helper HUD Settings",
                            helperHudMenu);

    uMenuItemFunction debugMenu(&helperSettingsMenu,
                            sg_helperDebug ? "0x00dd00Helper Debug" : "0xdd0000Helper Debug",
                            "Helper Debug Settings",
                            helperDebugMenu);

    uMenuItemFunction configMenu(&helperSettingsMenu,
                            "Helper Config",
                            "Helper Config Settings",
                            helperConfigMenu);

    uMenuItemFunction smartTurningMenu(&helperSettingsMenu,
                            sg_helperSmartTurning ? "0x00dd00Smart Turning" : "0xdd0000Smart Turning",
                            "Smart turning tools",
                            helperSmartTurningMenu);

    uMenuItemFunction selfMenu(&helperSettingsMenu,
                            "Self",
                            "Helper tools for your self",
                            helperSelfMenu);

    uMenuItemFunction enemiesMenu(&helperSettingsMenu,
                            "Enemies",
                            "Helper tools for enemies",
                            helperEnemyMenu);

    uMenuItemToggle helper(&helperSettingsMenu,
                                "Helper",
                                "Enable Helper",
                                sg_helper);

    helperSettingsMenu.Enter();
}

void helperMenuPub(std::istream &s) {
    helperMenu();
}
static tConfItemFunc HelperMenuConf("HELPER_MENU",&helperMenuPub);
