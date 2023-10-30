#include "../../gSensor.h"
#include "../../gCycle.h"
#include "../../gWall.h"
#include "gHelperSensor.h"
#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "../../gAINavigator.h"
using namespace helperConfig;

namespace helperConfig
{
    bool sg_helperSmartTurning = false;
    static tConfItem<bool> sg_helperSmartTurningConf = HelperCommand::tConfItem("HELPER_SMART_TURNING", sg_helperSmartTurning);

    bool sg_helperSmartTurningFrontBot = false;
    static tConfItem<bool> sg_helperSmartTurningFrontBotConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FRONT_BOT", sg_helperSmartTurningFrontBot);
    REAL sg_helperSmartTurningFrontBotThinkRange = 1;
    static tConfItem<REAL> sg_helperSmartTurningFrontBotThinkRangeConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FRONT_BOT_THINK_RANGE", sg_helperSmartTurningFrontBotThinkRange);
    bool sg_helperSmartTurningFrontBotTurnOnce = true;
    static tConfItem<bool> sg_helperSmartTurningFrontBotTurnOnceConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FRONT_BOT_TURN_ONCE", sg_helperSmartTurningFrontBotTurnOnce);

    REAL sg_helperSmartTurningFrontBotActivationRubber = 0.98;
    static tConfItem<REAL> sg_helperSmartTurningFrontBotActivationRubberConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FRONT_BOT_RUBBER", sg_helperSmartTurningFrontBotActivationRubber);
    REAL sg_helperSmartTurningFrontBotActivationSpace = 1;
    static tConfItem<REAL> sg_helperSmartTurningFrontBotActivationSpaceConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FRONT_BOT_SPACE", sg_helperSmartTurningFrontBotActivationSpace);
    REAL sg_helperSmartTurningFrontBotDisableTime = 0;
    static tConfItem<REAL> sg_helperSmartTurningFrontBotDisableTimeConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FRONT_BOT_DISABLE_TIME", sg_helperSmartTurningFrontBotDisableTime);

    bool sg_helperSmartTurningAutoTrace = false;
    static tConfItem<bool> sg_helperSmartTurningAutoTraceConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_AUTO_TRACE", sg_helperSmartTurningAutoTrace);

    REAL sg_helperSmartTurningAutoTraceDistance = 1;
    static tConfItem<REAL> sg_helperSmartTurningAutoTraceDistanceConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_AUTO_TRACE_DISTANCE", sg_helperSmartTurningAutoTraceDistance);

    bool sg_helperSmartTurningSurvive = false;
    static tConfItem<bool> sg_helperSmartTurningSurviveConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SURVIVE", sg_helperSmartTurningSurvive);
    bool sg_helperSmartTurningSurviveDebug = false;
    static tConfItem<bool> sg_helperSmartTurningSurviveDebugConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SURVIVE_DEBUG", sg_helperSmartTurningSurviveDebug);

    bool sg_helperSmartTurningSurviveTrace = false;
    static tConfItem<bool> sg_helperSmartTurningSurviveTraceConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SURVIVE_TRACE", sg_helperSmartTurningSurviveTrace);
    bool sg_helperSmartTurningSurviveTraceTurnOnce = false;
    static tConfItem<bool> sg_helperSmartTurningSurviveTraceTurnOnceConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SURVIVE_TRACE_TURN_ONCE", sg_helperSmartTurningSurviveTraceTurnOnce);
    REAL sg_helperSmartTurningSurviveTraceTurnTime = 0.02;
    static tConfItem<REAL> sg_helperSmartTurningSurviveTraceTurnTimeConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SURVIVE_TRACE_TURN_TIME", sg_helperSmartTurningSurviveTraceTurnTime);
    REAL sg_helperSmartTurningSurviveTraceActiveTime = 1;
    static tConfItem<REAL> sg_helperSmartTurningSurviveTraceActiveTimeConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SURVIVE_TRACE_ACTIVE_TIME", sg_helperSmartTurningSurviveTraceActiveTime);
    REAL sg_helperSmartTurningSurviveTraceCloseFactor = 1;
    static tConfItem<REAL> sg_helperSmartTurningSurviveTraceCloseFactorConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SURVIVE_TRACE_CLOSE_FACTOR", sg_helperSmartTurningSurviveTraceCloseFactor);

    bool sg_helperSmartTurningDisableWhileClosedIn = true;
    static tConfItem<bool> sg_helperSmartTurningDisableWhileClosedInConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_DISABLE_WHILE_CLOSED_IN", sg_helperSmartTurningDisableWhileClosedIn);
    REAL sg_helperSmartTurningDisableWhileClosedInMult = 1;
    static tConfItem<REAL> sg_helperSmartTurningDisableWhileClosedInMultConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_CLOSED_IN_MULT", sg_helperSmartTurningDisableWhileClosedInMult);

    bool sg_helperSmartTurningOpposite = false;
    static tConfItem<bool> sg_helperSmartTurningOppositeConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_OPPOSITE", sg_helperSmartTurningOpposite);
    REAL sg_helperSmartTurningSpace = 0;
    static tConfItem<REAL> sg_helperSmartTurningSpaceConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_SPACE", sg_helperSmartTurningSpace);
    bool sg_helperSmartTurningPlan = false;
    static tConfItem<bool> sg_helperSmartTurningPlanConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_PLAN", sg_helperSmartTurningPlan);

    bool sg_helperSmartTurningFollowTail = false;
    // static tConfItem<bool> sg_helperSmartTurningFollowTailConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FOLLOW_TAIL", sg_helperSmartTurningFollowTail);
    REAL sg_helperSmartTurningFollowTailDelayMult = 1;
    // static tConfItem<REAL> sg_helperSmartTurningFollowTailDelayMultConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FOLLOW_TAIL_DELAY_MULT", sg_helperSmartTurningFollowTailDelayMult);
    REAL sg_helperSmartTurningFollowTailFreeSpaceMult = 1;
    // static tConfItem<REAL> sg_helperSmartTurningFollowTailFreeSpaceMultConf = HelperCommand::tConfItem("HELPER_SMART_TURNING_FOLLOW_TAIL_FREE_SPACE_MULT", sg_helperSmartTurningFollowTailFreeSpaceMult);

};

// SmartTurning
gSmartTurning::gSmartTurning(gHelper &helper, gCycle &owner)
    : helper_(helper),
      owner_(owner),
      lastTurnAttemptTime(owner_.lastTurnAttemptTime),
      lastTurnAttemptDir(owner_.lastTurnAttemptDir),
      lastTurnTime(owner_.lastTurnTime),
      turnIgnoreTime(owner_.turnIgnoreTime),
      lastTurnDir(owner_.lastTurnDir), // 0 none, -1 left , 1 right
      blockTurn(owner_.blockTurn),     // 0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH
      forceTurn(owner_.forceTurn),
      lastTailTurnTime(-999) // 0 = NONE, -1 = LEFT, 1 = RIGHT

{
}

// Automatically turn to trace corners
void gSmartTurning::smartTurningPlan(gHelperData &data)
{
    // Check if the player is alive.
    if (!helper_.aliveCheck())
        return;

    // If there are no corners found, find them.
    if (!data.leftCorner.exist && !data.rightCorner.exist)
    {
        helper_.findCorners(data);
        return;
    }

    // Select the corner closest to the player.
    gSmartTurningCornerData cornerToUse = data.leftCorner;
    int dir = LEFT;
    if (cornerToUse.exist && data.rightCorner.exist)
    {
        bool useLeftCorner = cornerToUse.distanceFromPlayer < data.rightCorner.distanceFromPlayer ? true : false;
        cornerToUse = useLeftCorner ? cornerToUse : data.rightCorner;
        dir = useLeftCorner ? LEFT : RIGHT;
    }
    else if (!cornerToUse.exist && data.rightCorner.exist)
    {
        cornerToUse = data.rightCorner;
        dir = RIGHT;
    }

    // Check if the player is close to the selected corner.
    bool close = gHelperUtility::isClose(&owner_, cornerToUse.currentPos, sg_helperShowCornersBoundary);
    if (!close)
    {
        return;
    }

    // Make a turn if possible.
    if (cornerToUse.infront)
    {
        if (helper_.turnHelper->makeTurnIfPossible(data, dir, 5))
        {
            gHelperUtility::Debug("SMART TURNING PLAN", std::string("MADE TURN: ") + ((dir == LEFT) ? "LEFT" : "RIGHT"));
        }
    }
}

/**
 * This function implements the behavior of following the owner's tail.
 * If the tail is behind the cycle, the cycle will turn towards the tail if there is enough space.
 * If the tail is in front of the cycle, the cycle will continue on its current path.
 *
 * @param data A reference to a gHelperData object that contains information about the cycle and the game state.
 */
void gSmartTurning::followTail(gHelperData &data)
{
    // If the cycle is not alive, or the tail is not moving, or the tail is not close to the cycle, return
    if (!helper_.aliveCheck() || !owner_.tailMoving ||
        (!gHelperUtility::isClose(&owner_, owner_.tailPos, data.ownerData.turnSpeedFactorF() * 3)))
        return;

    // Calculate the delay between turning actions
    REAL delay = data.ownerData.turnSpeedFactorF() * sg_helperSmartTurningFollowTailDelayMult;

    // Check if the cycle is driving straight
    // bool drivingStraight = helper_.drivingStraight();

    // Check if the cycle can survive turning left or right

    gSurviveData surviveData = helper_.turnHelper->canSurviveTurn(data, sg_helperSmartTurningFollowTailFreeSpaceMult);
    if (!surviveData.exist)
        return;
    // If the cycle is closed in or blocked by itself, return
    if (surviveData.closedIn || surviveData.blockedBySelf)
        return;

    // Calculate the direction from the cycle to the tail
    eCoord directionToTail = owner_.tailPos - owner_.pos;
    directionToTail.Normalize();
    int turnDirection = NONE;
    // Determine if the tail is in front of or behind the cycle
    if (directionToTail * owner_.dir < 0)
    {
        // The tail is behind the cycle, turn towards it

        // Check if there are walls to the left or right of the cycle
        gHelperSensor leftSensor(&owner_, owner_.pos, owner_.dir.Turn(0, 1));
        leftSensor.detect(sg_helperSensorRange);
        gHelperSensor rightSensor(&owner_, owner_.pos, owner_.dir.Turn(0, -1));
        rightSensor.detect(sg_helperSensorRange);

        // Check if the tail is to the right or left of the cycle
        if (directionToTail * owner_.dir > 0 && surviveData.canSurviveRightTurn && rightSensor.hit > .999999)
            turnDirection = RIGHT;
        else if (surviveData.canSurviveLeftTurn && leftSensor.hit > .999999)
            turnDirection = LEFT;

        if (turnDirection == NONE)
            return;
    }
    else
    {
        // The tail is in front of the cycle, stay on current path

        // The tail is directly in front of or behind the cycle
        // Stay on current path
    }

    // Check if the cycle has turned recently
    bool turnedRecently = !(lastTailTurnTime < 0) && owner_.lastTurnTime + delay > helper_.CurrentTime();

    // Check if the cycle is ready to turn again
    bool readyToTurn = helper_.CurrentTime() > lastTailTurnTime + delay;

    // If the cycle has not turned recently or if the cycle is not ready to turn, return
    if (!readyToTurn)
    {
        return;
    }

    // If the turn direction is LEFT and the cycle can survive a left turn,
    // turn the cycle left and update the last tail turn time
    if (surviveData.canSurviveRightTurn && turnDirection == LEFT)
    {
        owner_.ActTurnBot(LEFT);
        lastTailTurnTime = helper_.CurrentTime();
    }

    // If the turn direction is RIGHT and the cycle can survive a right turn,
    // turn the cycle right and update the last tail turn time
    if (surviveData.canSurviveLeftTurn && turnDirection == RIGHT)
    {
        owner_.ActTurnBot(RIGHT);
        lastTailTurnTime = helper_.CurrentTime();
    }
}

void gSmartTurning::smartTurningOpposite(gHelperData &data)
{
    // If the cycle is not alive, return
    if (!helper_.aliveCheck())
        return;

    // Determine if the cycle can survive a turn to the left or right according to rubber and space
    gSurviveData surviveData = helper_.turnHelper->canSurviveTurn(data, sg_helperSmartTurningSpace);
    if (!surviveData.exist)
        return;

    // Check if the cycle is closed in or blocked by itself
    if ((sg_helperSmartTurningDisableWhileClosedIn && surviveData.closedIn) || (surviveData.closedIn) && surviveData.blockedBySelf)
    {
        // Skip the survive logic turn if the cycle is closed in or blocked by itself
        goto SKIP_FORCETURN;
    }

    // Check if the cycle can survive a right turn but not a left turn
    if (!surviveData.canSurviveLeftTurn && surviveData.canSurviveRightTurn)
    {
        // Set the force turn to RIGHT
        owner_.forceTurn = RIGHT;
        // Set the block turn to NONE
        owner_.blockTurn = NONE;
        // Return from the function
        return;
    }
    // Check if the cycle can survive a left turn but not a right turn
    else if (surviveData.canSurviveLeftTurn && !surviveData.canSurviveRightTurn)
    {
        // Set the force turn to LEFT
        owner_.forceTurn = LEFT;
        // Set the block turn to NONE
        owner_.blockTurn = NONE;
        // Return from the function
        return;
    }

SKIP_FORCETURN:
{
    // Set the force turn to NONE
    this->forceTurn = NONE;
    // Return from the function
    return;
}
}

/**
 * @brief gSmartTurning::smartTurningSurvive function checks if the player can survive the turn.
 * If the player cannot survive either direction, block both turns and if enabled, turn as soon as possible using trace.
 *
 * @param data The reference to the gHelperData struct that holds all the helper data.
 */
void gSmartTurning::smartTurningSurvive()
{
    smartTurningSurvive(helper_.data_stored);
}
void gSmartTurning::smartTurningSurvive(gHelperData &data)
{
    // Check if the player is alive. If not, return.
    if (!helper_.aliveCheck())
        return;

    // Check if the player can survive a turn.
    gSurviveData surviveData = helper_.turnHelper->canSurviveTurn(data, sg_helperSmartTurningSpace, false, sg_helperSmartTurningSurviveDebug);
    if (!surviveData.exist)
        return;

    // If the player has a turn ignore time set and it is not over, block both turns and return.
    if (owner_.turnIgnoreTime > owner_.localCurrentTime)
    {
        this->blockTurn = BOTH;
        return;
    }
    // If the player is closed in or blocked by themselves and closedIn is set, unblock both turns and return.
    if (
        (sg_helperSmartTurningDisableWhileClosedIn && surviveData.closedIn) ||
        (surviveData.closedIn && surviveData.blockedBySelf) ||
        (sg_helperSmartTurningSpace > 0 && surviveData.blockedBySelf && !surviveData.closedIn)
       )
    {
        goto UN_BLOCKTURN;
    }

    // If the player cannot survive either of the turn, block both turns.
    if (!surviveData.canSurviveLeftTurn && !surviveData.canSurviveRightTurn)
    {
        gHelperUtility::Debug("SMART TURNING SURVIVE", "BLOCKING BOTH TURNS");
        this->blockTurn = BOTH;
        return;
    }

    // If the player cannot survive the left turn, block the left turn.
    if (!surviveData.canSurviveLeftTurn)
    {
        gHelperUtility::Debug("SMART TURNING SURVIVE", "BLOCKING LEFT TURNS");

        if (sg_helperSmartTurningSurviveDebug)
            gHelperUtility::Debug("SMART TURNING SURVIVE", std::string(surviveData.debug));

        this->blockTurn = LEFT;
        return;
    }

    // If the player cannot survive the right turn, block the right turn.
    if (!surviveData.canSurviveRightTurn)
    {
        gHelperUtility::Debug("SMART TURNING SURVIVE", "BLOCKING RIGHT TURN");

        if (sg_helperSmartTurningSurviveDebug)
            gHelperUtility::Debug("SMART TURNING SURVIVE", std::string(surviveData.debug));

        this->blockTurn = RIGHT;
        return;
    }

UN_BLOCKTURN:
    // Unblock the turns if the player can survive a turn.
    if (this->blockTurn != NONE)
    {
        // Store the last blocked turn for smartTurningSurviveTrace before clearing
        owner_.lastBlockedTurn = this->blockTurn;

        gHelperUtility::Debug("SMART TURNING SURVIVE", "UNBLOCKING TURNS");

        // Set the blockTurn to NONE to indicate that the player can now turn in either direction.
        this->blockTurn = NONE;
    }

    // Check if the smartTurningSurviveTrace is enabled and call the function if it is.
    if (sg_helperSmartTurningSurviveTrace && owner_.lastTurnAttemptDir != NONE)
        smartTurningSurviveTrace(data);
}

/**
 * @brief smartTurningSurviveTrace - performs the logic for the survive trace functionality of the smart turning module
 *
 * The function checks if the bot is alive and driving straight. If not, the function returns immediately.
 * The function then retrieves the last blocked direction the bot tried to turn in. If the bot did not try to turn
 * in the last time step, the function returns immediately.
 * The function then retrieves the corner data of the blocked direction. If the corner is close, it has been noticed
 * recently, the turn time is positive and the turn time factor is less than or equal to the survive trace turn time,
 * the function calls the helper_.turnHelper->makeTurnIfPossible function to attempt to turn in
 * the blocked direction as soon as possible.
 @param data - the helper data struct containing information used by the helper logic

 @return - the function returns void and has no return value
*/
void gSmartTurning::smartTurningSurviveTrace(gHelperData &data)
{
    // Return if the bot is not alive or not driving straight
    // Return if the last turn attempt was more recent than the last successful turn
    bool aliveCheck = helper_.aliveCheck();
    bool lastTurnAttemptCheck = (sg_helperSmartTurningSurviveTraceTurnOnce && owner_.lastBotTurnTime > owner_.lastTurnTime) || (owner_.lastTurnAttemptTime < owner_.lastTurnTime);

    if (!aliveCheck || lastTurnAttemptCheck)
    {
        // gHelperUtility::Debug("SMART TURNING TRACE", "CONDITIONS NOT MET",
        //                       "aliveCheck: " + std::to_string(aliveCheck) +
        //                       ", lastTurnAttemptCheck: " + std::to_string(lastTurnAttemptCheck));
        return;
    }

    int lastBlockedDir = owner_.lastTurnAttemptDir;

    if (lastBlockedDir == NONE)
        return;

    // Get the direction of the last blocked turn attempt

    // con << "SMART TURNING LAST lastBlockedDir " << lastBlockedDir << "\n";
    //  Get the information about the blocked corner
    gSmartTurningCornerData &corner = helper_.getCorner(lastBlockedDir);

    if (!corner.exist)
    {
        gHelperUtility::Debug("SMART TURNING TRACE","No corner");
        return;
    }

    // con << "lastBlockedDir " << lastBlockedDir << "\n";
    // Calculate the time factor for the turn
    REAL turnTimeFactor = corner.getTimeUntilTurn(owner_.Speed());

    // con << "turnTimeFactor " << turnTimeFactor << "\n";
    // If the following conditions are met, try to make a turn:
    // 1. The corner exists
    // 2. The bot is close enough to the corner
    // 3. The time since the last turn attempt is greater than a certain threshold
    // 4. The time until the turn is greater than 0
    // 5. The time until the turn is less than a certain threshold

    // REAL noticedTime = se_GameTime() -corner.noticedTime

    // con << "Corner detected: " << corner.currentPos.x << ", " << corner.currentPos.y << "\n";
    // con << "Close factor: " << sg_helperSmartTurningSurviveTraceCloseFactor << "\n";
    // con << "Turn speed factor: " << data.ownerData.turnSpeedFactorF() << "\n";
    // con << "Last turn attempt time: " << owner_.lastTurnAttemptTime << "\n";
    // con << "Active time: " << sg_helperSmartTurningSurviveTraceActiveTime << "\n";
    // con << "Noticed time: " << corner.noticedTime << "\n";
    // con << "Time until turn: " << corner.getTimeUntilTurn(owner_.Speed()) << "\n";
    // con << "Turn time factor: " << turnTimeFactor << "\n";

    if (gHelperUtility::isClose(&owner_, corner.currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.ownerData.turnSpeedFactorF()) &&
        owner_.lastTurnAttemptTime + sg_helperSmartTurningSurviveTraceActiveTime + data.ownerData.turnSpeedFactorF() > corner.noticedTime &&
        corner.getTimeUntilTurn(owner_.Speed()) > 0 &&
        (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime))
    {
        // Attempt to make a turn in the direction of the blocked attempt
        if (helper_.turnHelper->makeTurnIfPossible(data, lastBlockedDir, 1))
        {
            // Print the turn
            gHelperUtility::Debug("SMART TURNING TRACE", std::string("TURNED") + ((lastBlockedDir == LEFT) ? "LEFT" : "RIGHT"));
        }
    }
}

/**
 * @brief gSmartTurning::smartTurningFrontBot handles the front bot actions by getting an emergecy turning when rubber usage
 * or space factor reaches a current threashold.
 * This function checks if the bot has any pending turns and if the smartTurningFrontBotTurnOnce flag is set and the
 * last bot turn was made more recently than the last turn, the function returns without any action.
 * If the rubber used ratio is greater than or equal to sg_helperSmartTurningFrontBotActivationRubber or the range to the
 * front bot is less than or equal to  sg_helperSmartTurningFrontBotActivationSpace, the function attempts to make a turn in
 * the direction determined by the emergency turn. If a turn is made, new turns by the owner will be ignored for a specified
 * time determined by setting the turnIgnoreTime to the current time plus sg_helperSmartTurningFrontBotDisableTime.
 *
 * @param data gHelperData object containing sensor data and other relevant information.
 */
void gSmartTurning::smartTurningFrontBot(gHelperData &data)
{
    // Return if there are any pending turns
    // or the smartTurningFrontBotTurnOnce flag is set and
    // the last bot turn was made more recently than the last turn

    if (!owner_.pendingTurns.empty() ||
        (sg_helperSmartTurningFrontBotTurnOnce && owner_.lastBotTurnTime > owner_.lastTurnTime))
        return;

    // Get the distance from the front
    REAL hitRange = data.sensors.getSensor(FRONT)->hit;

    // If the range to the front bot is within the sg_helperSmartTurningFrontBotThinkRange
    if (hitRange <= sg_helperSmartTurningFrontBotThinkRange * data.ownerData.turnSpeedFactorF())
    {
        // If the rubber used ratio is greater than or equal to sg_helperSmartTurningFrontBotActivationRubber
        // or the range to the front bot is less than or equal to sg_helperSmartTurningFrontBotActivationSpace
        if (
            (sg_helperSmartTurningFrontBotActivationRubber > 0 && data.rubberData.rubberUsedRatioF() + data.ownerData.lagFactorF() >= sg_helperSmartTurningFrontBotActivationRubber) ||
            (sg_helperSmartTurningFrontBotActivationSpace > 0 && hitRange <= sg_helperSmartTurningFrontBotActivationSpace))
        {
            gSmarterBot::Survive(&owner_);
        }
    }
}

void gSmartTurning::Activate(gHelperData &data)
{
    if (sg_helperSmartTurningSurvive)
    {
        sg_helperSmartTurningOpposite = 0;
        sg_helperSmartTurningPlan = 0;
        smartTurningSurvive(data);
    }

    if (sg_helperSmartTurningOpposite)
    {
        sg_helperSmartTurningSurvive = 0;
        sg_helperSmartTurningPlan = 0;
        smartTurningOpposite(data);
    }

    if (sg_helperSmartTurningPlan)
    {
        sg_helperSmartTurningSurvive = 0;
        sg_helperSmartTurningOpposite = 0;
        smartTurningPlan(data);
    }

    // if (sg_helperSmartTurningFollowTail)
    //     followTail(data);

    if (sg_helperSmartTurningFrontBot)
        smartTurningFrontBot(data);
}

gSmartTurning &gSmartTurning::Get(gHelper &helper, gCycle &owner)
{
    tASSERT(&helper);

    // create
    if (helper.smartTurning.get() == 0)
        helper.smartTurning.reset(new gSmartTurning(helper, owner));

    return *helper.smartTurning;
}
