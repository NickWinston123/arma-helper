#include "../../gSensor.h"
#include "../../gCycle.h"
#include "../../gWall.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"

using namespace helperConfig;

// SmartTurning
gSmartTurning::gSmartTurning(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner),
      lastTurnAttemptTime(owner_->lastTurnAttemptTime),
      lastTurnAttemptDir(owner_->lastTurnAttemptDir),
      lastTurnTime(owner_->lastTurnTime),
      turnIgnoreTime(owner_->turnIgnoreTime),
      lastTurnDir(owner_->lastTurnDir), // 0 none, -1 left , 1 right
      blockTurn(owner_->blockTurn),     // 0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH
      forceTurn(owner_->forceTurn),
      lastTailTurnTime(-999) // 0 = NONE, -1 = LEFT, 1 = RIGHT

{
}


// Function: isClose
// Check if the distance between a point and the player is within a certain range.
//
// Parameters:
// pos: The position to check distance to.
// closeFactor: The maximum allowed distance to be considered close.
//
// Returns:
// bool: True if the distance is within the closeFactor range, False otherwise.
bool gSmartTurning::isClose(eCoord pos, REAL closeFactor)
{
    eCoord distanceToPos = owner_->pos - pos;
    return distanceToPos.NormSquared() < closeFactor * closeFactor;
}

// Automatically turn to trace corners
void gSmartTurning::smartTurningPlan(gHelperData &data)
{
    // Check if the player is alive.
    if (!helper_->aliveCheck())
    {
        return;
    }

    // If there are no corners found, find them.
    if (!helper_->leftCorner.exist && !helper_->rightCorner.exist)
    {
        helper_->findCorners(data);
        return;
    }

    // Select the corner closest to the player.
    gSmartTurningCornerData cornerToUse = helper_->leftCorner;
    int dir = LEFT;
    if (cornerToUse.exist && helper_->rightCorner.exist)
    {
        bool useLeftCorner = cornerToUse.distanceFromPlayer < helper_->rightCorner.distanceFromPlayer ? true : false;
        cornerToUse = useLeftCorner ? cornerToUse : helper_->rightCorner;
        dir = useLeftCorner ? LEFT : RIGHT;
    }
    else if (!cornerToUse.exist && helper_->rightCorner.exist)
    {
        cornerToUse = helper_->rightCorner;
        dir = RIGHT;
    }

    // Check if the player is close to the selected corner.
    bool close = isClose(cornerToUse.currentPos, sg_helperShowCornersBoundary);
    if (!close)
    {
        return;
    }

    // Make a turn if possible.
    if (cornerToUse.infront)
    {
        if (helper_->turnHelper->makeTurnIfPossible(data, dir, 5))
        {
            HelperDebug::Debug("SMART TURNING PLAN", std::string("MADE TURN: ") + ((dir == LEFT) ? "LEFT" : "RIGHT"), "");
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
    if (!helper_->aliveCheck() || !owner_->tailMoving ||
       (!isClose(owner_->tailPos, data.turnSpeedFactorF() * 3)))
    {
        return;
    }

    // Calculate the delay between turning actions
    REAL delay = data.turnSpeedFactorF() * sg_helperSmartTurningFollowTailDelayMult;

    // Check if the cycle is driving straight
    bool drivingStraight = helper_->drivingStraight();

    // Check if the cycle can survive turning left or right
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;
    helper_->turnHelper->canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningFollowTailFreeSpaceMult);

    // If the cycle is closed in or blocked by itself, return
    if (closedIn || blockedBySelf)
    {
        return;
    }

    // Calculate the direction from the cycle to the tail
    eCoord directionToTail = owner_->tailPos - owner_->pos;
    directionToTail.Normalize();
    int turnDirection = NONE;
    // Determine if the tail is in front of or behind the cycle
    if (directionToTail * owner_->dir < 0)
    {
        // The tail is behind the cycle, turn towards it

        // Check if there are walls to the left or right of the cycle
        gSensor leftSensor(owner_, owner_->pos, owner_->dir.Turn(0, 1));
        leftSensor.detect(sg_helperSensorRange);
        gSensor rightSensor(owner_, owner_->pos, owner_->dir.Turn(0, -1));
        rightSensor.detect(sg_helperSensorRange);

        // Check if the tail is to the right or left of the cycle
        if (directionToTail * owner_->dir > 0 && canSurviveRightTurn && rightSensor.hit > .999999)
        {
            turnDirection = RIGHT;
        }
        else if (canSurviveLeftTurn && leftSensor.hit > .999999)
        {
            turnDirection = LEFT;
        }
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
    bool turnedRecently = !(lastTailTurnTime < 0) && owner_->lastTurnTime + delay > helper_->CurrentTime();

    // Check if the cycle is ready to turn again
    bool readyToTurn = helper_->CurrentTime() > lastTailTurnTime + delay;

    // If the cycle has not turned recently or if the cycle is not ready to turn, return
    if (!readyToTurn)
    {
        return;
    }

    // If the turn direction is LEFT and the cycle can survive a left turn,
    // turn the cycle left and update the last tail turn time
    if (canSurviveRightTurn && turnDirection == LEFT)
    {
        owner_->ActTurnBot(LEFT);
        lastTailTurnTime = helper_->CurrentTime();
    }

    // If the turn direction is RIGHT and the cycle can survive a right turn,
    // turn the cycle right and update the last tail turn time
    if (canSurviveLeftTurn && turnDirection == RIGHT)
    {
        owner_->ActTurnBot(RIGHT);
        lastTailTurnTime = helper_->CurrentTime();
    }
}

void gSmartTurning::smartTurningOpposite(gHelperData &data)
{
    // Check if the cycle is alive
    if (!helper_->aliveCheck())
    {
        // If the cycle is not alive, return
        return;
    }

    // Variables to store the result of the canSurviveTurn function
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    bool closedIn, blockedBySelf;

    // Determine if the cycle can survive a turn to the left or right according to rubber and space
    helper_->turnHelper->canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);

    // Check if the cycle is closed in or blocked by itself
    if ((closedIn) || (closedIn) && blockedBySelf)
    {
        // Skip the survive logic turn if the cycle is closed in or blocked by itself
        goto SKIP_FORCETURN;
    }

    // Check if the cycle can survive a right turn but not a left turn
    if (!canSurviveLeftTurn && canSurviveRightTurn)
    {
        // Set the force turn to RIGHT
        owner_->forceTurn = RIGHT;
        // Set the block turn to NONE
        owner_->blockTurn = NONE;
        // Return from the function
        return;
    }
    // Check if the cycle can survive a left turn but not a right turn
    else if (canSurviveLeftTurn && !canSurviveRightTurn)
    {
        // Set the force turn to LEFT
        owner_->forceTurn = LEFT;
        // Set the block turn to NONE
        owner_->blockTurn = NONE;
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
void gSmartTurning::smartTurningSurvive(gHelperData &data)
{
    // Check if the player is alive. If not, return.
    // Check if the player has any pending turns. If yes, return.
    if (!helper_->aliveCheck() || !owner_->pendingTurns.empty())
    {
        return;
    }

    // Variables to store the result of whether the player can survive a left or right turn.
    REAL canSurviveLeftTurn, canSurviveRightTurn;
    // Variables to store if the player is closed in or blocked by themselves.
    bool closedIn, blockedBySelf;

    // Check if the player can survive a turn.
    helper_->turnHelper->canSurviveTurn(data, canSurviveLeftTurn, canSurviveRightTurn, closedIn, blockedBySelf, sg_helperSmartTurningSpace);

    // If the player has a turn ignore time set and it is not over, block both turns and return.
    if (owner_->turnIgnoreTime > owner_->localCurrentTime)
    {
        this->blockTurn = BOTH;
        return;
    }
    // If the player is closed in or blocked by themselves and closedIn is set, unblock both turns and return.
    if ((closedIn && sg_helperSmartTurningClosedIn) || (closedIn) && blockedBySelf)
    {
        goto SKIP_BLOCKTURN;
    }

    // If the player cannot survive either of the turn, block both turns.
    if (!canSurviveLeftTurn && !canSurviveRightTurn)
    {
        if (this->blockTurn != BOTH)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING BOTH TURNS", "");

        this->blockTurn = BOTH;
        return;
    }

    // If the player cannot survive the left turn, block the left turn.
    if (!canSurviveLeftTurn)
    {
        if (this->blockTurn != LEFT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING LEFT TURNS", "");

        this->blockTurn = LEFT;
        return;
    }

    // If the player cannot survive the right turn, block the right turn.
    if (!canSurviveRightTurn)
    {
        if (this->blockTurn != RIGHT)
            HelperDebug::Debug("SMART TURNING SURVIVE", "BLOCKING RIGHT TURN", "");

        this->blockTurn = RIGHT;
            return;
    }

    SKIP_BLOCKTURN:
        // Unblock the turns if the player can survive a turn.
        owner_->lastBlockedTurn = this->blockTurn;

        if (this->blockTurn != NONE)
            HelperDebug::Debug("SMART TURNING SURVIVE", "UNBLOCKING TURNS", "");

        // Set the blockTurn to NONE to indicate that the player can now turn in either direction.
        this->blockTurn = NONE;

        // Check if the smartTurningSurviveTrace is enabled and call the function if it is.
        if (sg_helperSmartTurningSurviveTrace)
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
 * the function calls the helper_->turnHelper->makeTurnIfPossible function to attempt to turn in
 * the blocked direction as soon as possible.
 @param data - the helper data struct containing information used by the helper logic

 @return - the function returns void and has no return value
*/
void gSmartTurning::smartTurningSurviveTrace(gHelperData &data)
{
    // Return if the bot is not alive or not driving straight
    // Return if the last turn attempt was more recent than the last successful turn
    if (!helper_->aliveCheck() ||
        !helper_->drivingStraight() ||
        (owner_->lastTurnAttemptTime < owner_->lastTurnTime))
    {
        return;
    }

    // Get the direction of the last blocked turn attempt
    int lastBlockedDir = owner_->lastTurnAttemptDir;

    // Get the information about the blocked corner
    gSmartTurningCornerData* corner = helper_->getCorner(lastBlockedDir);

    // Calculate the time factor for the turn
    REAL turnTimeFactor = corner->getTimeUntilTurn(owner_->Speed());

    // If the following conditions are met, try to make a turn:
    // 1. The corner exists
    // 2. The bot is close enough to the corner
    // 3. The time since the last turn attempt is greater than a certain threshold
    // 4. The time until the turn is greater than 0
    // 5. The time until the turn is less than a certain threshold
    if (corner->exist && isClose(corner->currentPos, sg_helperSmartTurningSurviveTraceCloseFactor * data.turnSpeedFactorF()) &&
        owner_->lastTurnAttemptTime + (sg_helperSmartTurningSurviveTraceActiveTime / (10 * data.turnSpeedFactorF())) > corner->noticedTime &&
        corner->getTimeUntilTurn(owner_->Speed()) > 0 &&
        (turnTimeFactor <= sg_helperSmartTurningSurviveTraceTurnTime))
    {
        // Attempt to make a turn in the direction of the blocked attempt
        if (helper_->turnHelper->makeTurnIfPossible(data, lastBlockedDir, 1))
        {
            // Print the turn
            HelperDebug::Debug("SMART TURNING TRACE", std::string("TURNED") + ((lastBlockedDir == LEFT) ? "LEFT" : "RIGHT"), "");}
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

    if (!owner_->pendingTurns.empty() ||
        (sg_helperSmartTurningFrontBotTurnOnce && owner_->lastBotTurnTime > owner_->lastTurnTime))
    {
        return;
    }

    // Get the distance from the front 
    REAL hitRange = data.sensors.getSensor(FRONT)->hit;

    // If the range to the front bot is within the sg_helperSmartTurningFrontBotThinkRange
    if (hitRange <= sg_helperSmartTurningFrontBotThinkRange * data.turnSpeedFactorF())
    {
        // Calculate the rubber data
        helper_->rubberData->calculate();

        // If the rubber used ratio is greater than or equal to sg_helperSmartTurningFrontBotActivationRubber
        // or the range to the front bot is less than or equal to sg_helperSmartTurningFrontBotActivationSpace
        if (helper_->rubberData->rubberUsedRatio >= sg_helperSmartTurningFrontBotActivationRubber ||
            hitRange <= sg_helperSmartTurningFrontBotActivationSpace)
        {
            // Flag to check if a turn was made
            bool turnMade = false;

            // Get the turn data from the emergency turn object
            gTurnData* turnData = helper_->turnHelper->getTurn();

            // If the turn data exists and the number of turns is greater than 0
            if (turnData->exist && turnData->numberOfTurns > 0)
            {
                // Loop through the number of turns
                for (int i = 0; i < turnData->numberOfTurns; i++)
                {
                    // Try to make the turn in the specified direction (left or right)
                    if (helper_->turnHelper->makeTurnIfPossible(data, turnData->direction, 0))
                    {
                        // Log the direction of the turn made
                        HelperDebug::Debug("smartTurningFrontBot",
                                           "Turn made: " + std::string(turnData->direction == -1 ? "LEFT" : "RIGHT") + " Reason: " + (turnData->reason), "");
                        turnMade = true;
                    }
                }
            } else if (turnData->noTurns > 100) {
                HelperDebug::Debug("smartTurningFrontBot",
                                   "Failed to find turn in " + std::to_string(turnData->noTurns) + " tries!", "");
            }
            // If smartTurningFrontBotDisableTime is greater than 0 and a turn has been made
            if (sg_helperSmartTurningFrontBotDisableTime > 0 && turnMade)
            {
                // Set the turnIgnoreTime for the owner
                owner_->turnIgnoreTime = helper_->CurrentTime() + (sg_helperSmartTurningFrontBotDisableTime);
            }
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

    if (sg_helperSmartTurningFollowTail)
        followTail(data);

    if (sg_helperSmartTurningFrontBot)
        smartTurningFrontBot(data);
}

gSmartTurning &gSmartTurning::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(helper);

    // create
    if (helper->smartTurning.get() == 0)
        helper->smartTurning.reset(new gSmartTurning(helper, owner));

    return *helper->smartTurning;
}
