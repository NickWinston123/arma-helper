// HELPER
#include "gHelper.h"
#include "../gSensor.h"

#include "../gAIBase.h"
#include "gHelperVar.h"
#include "gHelperHud.h"
#include "gHelperUtilities.h"
#include "specialized/gSmartTurning.h"
#include "specialized/gHelperSensor.h"

extern REAL sg_cycleBrakeDeplete;
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

gHelperHudItem<tColoredString> helperDebugH("Debug",tColoredString("0xdd0000None"));

gHelperHudItemRef<bool> sg_helperShowHitH("Show Hit",sg_helperShowHit);
gHelperHudItem<REAL> sg_helperShowHitFrontDistH("Show Hit Front Dist",1000,"Show Hit");

gHelperHudItem<REAL> sg_helperActivateTimeH("Active Time",0);
gHelperHudItem<REAL> sg_helperGameTimeH("Game Time",0);

// gHelper::getCorner retrieves a pointer to the gSmartTurningCornerData instance for the given direction
// dir: the direction (LEFT or RIGHT) to retrieve the gSmartTurningCornerData instance for
// Returns: a pointer to the gSmartTurningCornerData instance for the given direction
gSmartTurningCornerData &gHelper::getCorner(int dir)
{
switch (dir)
{
case LEFT:
    return data_stored.leftCorner;
    break;
case RIGHT:
    return data_stored.rightCorner;
    break;
default:
    return data_stored.leftCorner;
}
}


REAL gHelper::CurrentTime() {
    return sg_helperCurrentTimeLocal ? owner_.localCurrentTime : se_GameTime();
}


void gHelper::turningBot(gHelperData &data){
    data.rubberData.calculate();
    if ( (sg_helperTurningBotActivationRubber > 0 && data.rubberData.rubberUsedRatio + data.ownerData.lagFactorF() >= sg_helperTurningBotActivationRubber) ||
         (sg_helperTurningBotActivationSpace  > 0 && data.sensors.getSensor(FRONT)->hit <= sg_helperTurningBotActivationSpace)
       )
    {
        int direction = owner_.lastBotTurnDir;

        // if (direction == -999)
            direction = data.sensors.getSensor(LEFT,true)->hit > data.sensors.getSensor(RIGHT,true)->hit ? LEFT : RIGHT;
        // else
        //     direction = direction * -1;

        for (int i = 0; i < sg_helperTurningBotTurns; i++) {
            owner_.ActTurnBot(direction);
        }
    }
}


gHelper::gHelper(gCycle &owner)
    :   owner_(owner),
        player_(*(owner.Player())),
        data_stored(*(new gHelperData())),
        ownerWallLength(owner.ThisWallsLength()),
        ownerTurnDelay(owner.GetTurnDelay()),
        ownerPos(owner.pos),
        ownerDir(owner.dir),
        tailPos(owner.tailPos),
        ownerSpeed(owner.verletSpeed_),
        closestEnemy(owner)
{

    data_stored.ownerData.owner_   = &owner_;
    data_stored.sensors.owner_     = &owner_;
    data_stored.enemies.owner_     = &owner_;
    data_stored.rubberData.owner_  = &owner_;
    data_stored.rubberData.helper_ = this;
    data_stored.leftCorner.linkLastCorner(&(data_stored.lastLeftCorner));
    data_stored.rightCorner.linkLastCorner(&(data_stored.lastRightCorner));

      gTurnHelper::Get(*this, owner_);
    gSmartTurning::Get(*this, owner_);
      gPathHelper::Get(*this, owner_);
      gTailHelper::Get(*this, owner_);
      gZoneHelper::Get(*this, owner_);


}


bool gHelper::drivingStraight() {
    return ((fabs(ownerDir.x) == 1 || fabs(ownerDir.y) == 1));
}


/**
* @brief Check if the target is visible from the owner's current position
* This function uses a gHelperSensor to detect if the target is visible from the owner's current position
* @param target The target coordinate to check visibility for
* @param passthrough The minimum value of the sensor hit to be considered as visible
* @return True if the target is visible, False otherwise
*/
bool gHelper::canSeeTarget(eCoord target, REAL passthrough) {
    gHelperSensor sensor(&owner_, ownerPos, target - (ownerPos));
    sensor.detect(REAL(.98));
    return (sensor.hit >= passthrough);
}

void gHelper::detectCut(gHelperData &data, int detectionRange)
{
    // Check if the object is still alive
    if (!aliveCheck())
        return;

    // timeout value = speed factor + sg_helperDetectCutTimeout
    REAL timeout = data.ownerData.speedFactorF() + sg_helperDetectCutTimeout;

    // Get the closest enemy
    gCycle *enemy = data.enemies.closestEnemy;

    // If there's no enemy, return
    if (!data.enemies.exist(enemy))
        return;

    if (sg_helperHud)
        closestEnemyH << (enemy->Player()->GetColoredName());

    // canCutUs: Can the enemy either turn left or right and over turn our cycle? or continue to drive in their current direction and cut our cycle
    // canCutEnemy: Can the owner turn left or right and over turn the enemy? or continue to drive forward in our current direction and cut the enemy cycle
    bool canCutUs, canCutEnemy;

    // Get the position and direction of the enemy cycle
    eCoord enemyPos = enemy->Position();
    eCoord enemyDir = enemy->Direction();

    // Get the speed of the enemy cycle
    REAL enemySpeed = enemy->Speed();

    // Get the position and direction of our cycle
    eCoord ourPos = ownerPos;
    eCoord ourDir = ownerDir;

    // Get the speed of our cycle
    REAL ourSpeed = ownerSpeed;

    // enemyPos - ourPos: enemy now becomes the center of our coordinate system at (0,0)
    eCoord relEnemyPos = enemyPos - ourPos;
    // Transform relative position relative to direction of our cycle
    relEnemyPos = relEnemyPos.Turn(ourDir.Conj()).Turn(LEFT);

    /*
    Now our transformed relative coordinate system:
    .x: How far left / right
    .y: How far up / down
    */

    // Goal: Exploit symmetry along the x axis, put enemy on the right side of
    // our relative coordinate system ( so .x > 0 )

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
    REAL enemydist = enemy->Lag();

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
}


void gHelper::autoBrake()
{
    // Check if the cycle is still alive
    if (!aliveCheck())
        return;

    // Get the current used braking percentage of the cycle ( always out of 1 )
    REAL brakeUsagePercent = owner_.GetBrakingReservoir();

    // Check if the brake depletion is enabled
    bool cycleBrakeDeplete = true;
    if (sg_helperAutoBrakeDeplete && sg_helperAutoBrakeMin == 0 && sg_cycleBrakeDeplete < 0)
    {
        cycleBrakeDeplete = false;
    }

    // Check if the cycle is already braking and the used brake percentage is below the minimum brake limit
    // and brake depletion is enabled
    if (owner_.GetBraking() && brakeUsagePercent <= sg_helperAutoBrakeMin && cycleBrakeDeplete)
    {
        // Stop braking
        owner_.ActBot(&gCycle::s_brake, -1);
    }

    // Check if the cycle is not braking and the used brake percentage is above the maximum brake limit
    if (!owner_.GetBraking() && brakeUsagePercent >= sg_helperAutoBrakeMax)
    {
        // brake
        owner_.ActBot(&gCycle::s_brake, 1);
    }
}


// This function adds tracers to the position of enemies to make it easier for the player to see their position.
void gHelper::enemyTracers(gHelperData &data)
{
    // If the player is not alive, return from the function
    if (!aliveCheck())
        return;

    // Iterate over all enemies
    for (auto enemy = data.enemies.allEnemies.begin(); enemy != data.enemies.allEnemies.end(); ++enemy)
    {
        // Get the current enemy cycle
        gCycle *other = *enemy;
        // If the enemy doesn't exist, skip to the next iteration
        if (!data.enemies.exist(other))
            continue;

        // Get the position of the enemy cycle
        eCoord enemyPos = other->Position();
        // Initialize RGB values for the tracer color
        REAL R = .1, G = .1, B = 0;
        // Check if the enemy is close to the player
        bool isClose = gHelperUtility::isClose(&owner_, enemyPos, sg_helperEnemyTracersDetectionRange + data.ownerData.turnSpeedFactorF());
        // Check if the enemy cycle is faster than the player cycle
        bool enemyFaster = ((other->Speed() > ((ownerSpeed) * sg_helperEnemyTracersSpeedMult)));
        // Check if the enemy cycle is a teammate
        bool isTeammate = (owner_.Team() == other->Team());

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
        gHelperUtility::debugLine(gRealColor(R, G, B), sg_helperEnemyTracersHeight, sg_helperEnemyTracersTimeout * data.ownerData.speedFactorF(), (ownerPos), enemyPos, sg_helperEnemyTracersBrightness);
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
void gHelper::showTail(gHelperData &data)
{
    if (!aliveCheck() || owner_.tailMoving != true)
        return;


    REAL timeout = sg_helperShowTailTimeout * data.ownerData.speedFactorF();

    if (canSeeTarget((tailPos), sg_helperShowTailPassthrough))
    {
        gHelperUtility::debugLine(gRealColor(owner_.color_.r, owner_.color_.g, owner_.color_.b), sg_helperShowTailHeight, timeout, (ownerPos), (tailPos));
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
void gHelper::showEnemyTail(gHelperData &data)
{
    // return if the current cycle is not alive
    if (!aliveCheck())
        return;

    // variables to store the distance to the tail and timeout
    REAL distanceToTail, timeout;

    // loop over all enemies in the game
    for (auto enemy = data.enemies.allEnemies.begin(); enemy != data.enemies.allEnemies.end(); ++enemy)
    {
        // get the cycle object
        gCycle *other = *enemy;
        // continue if the cycle doesn't exist or its tail is not moving
        if (!data.enemies.exist(other) || !other->tailMoving)
            continue;

        // calculate the distance to the tail
        distanceToTail = sg_helperShowEnemyTailDistanceMult * (eCoord::F(ownerDir, (other->tailPos) - (ownerPos)));
        // calculate the timeout
        timeout = fabs(distanceToTail) / 10 * data.ownerData.speedFactorF();
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
void gHelper::showTailTracer(gHelperData &data)
{
    // checks if the object exists and if its tail is moving
    if (!aliveCheck() || !owner_.tailMoving)
        return;

    // calculates the distance between the object's position and its tail position
    REAL distanceToTail = sg_helperShowTailTracerTimeoutMult * eCoord::F(ownerDir, (tailPos) - (ownerPos));
    // calculates the timeout for the debug line based on the distance and speed
    REAL timeout = fabs(distanceToTail) / sg_helperShowTailTracerDistanceMult * data.ownerData.speedFactorF();

    // draws a debug line at the tail position with a specified height, color, and timeout
    gHelperUtility::debugLine(gRealColor(1, 1, 1), sg_helperShowTailTracerHeight, timeout * sg_helperShowTailTracerTimeoutMult, tailPos, tailPos, sg_helperShowTailTracerBrightness);
}


// Function: findCorners
// Purpose: Finds the corners on the left and right of the vehicle.
// Input: data - A reference to the gHelperData object containing all necessary data for the helper.
// Output: None.
void gHelper::findCorners(gHelperData &data)
{
    // Find the left corner
    data.leftCorner.findCorner(data.sensors.getSensor(LEFT),*this);
    // Find the right corner
    data.rightCorner.findCorner(data.sensors.getSensor(RIGHT),*this);
}

// Function: showCornergHelperUtiltiy::
// Purpose: Visualizes the corner information.
// Input: data - A reference to the gHelperData object containing all necessary data for the helper.
//        corner - The corner information to be visualized.
//        timeout - The time for which the visualization should be shown.
// Output: None.
void gHelper::showCorner(gHelperData &data, gSmartTurningCornerData & corner, REAL timeout)
{
    // Check if the corner exists
    if (corner.exist)
    {
        // Calculate the timeout based on speed factor
        timeout = data.ownerData.speedFactorF() * sg_helperShowCornersTimeout;
        // Check if the corner is close to the vehicle
        bool isClose = gHelperUtility::isClose(&owner_, corner.currentPos, sg_helperShowCornersBoundary);
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
void gHelper::showCorners(gHelperData &data)
{
    // Check if the vehicle is alive and has a grid
    if (!aliveCheck())
        return;

    // Calculate the timeout based on speed factor
    REAL timeout = data.ownerData.speedFactorF() * sg_helperShowCornersTimeout;
    // Find the corners
    findCorners(data);
    // Visualize the left corner
    showCorner(data, data.leftCorner, timeout);
    // Visualize the right corner
    showCorner(data, data.rightCorner, timeout);
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
void gHelper::showHit(gHelperData &data)
{
    // check if the owner is alive and driving straight
    if (!aliveCheck() || !drivingStraight())
        return;


    // get the front hit distance and check if it's close to the wall
    REAL frontHit = data.sensors.getSensor(FRONT)->hit;
    bool wallClose = frontHit < data.ownerData.turnSpeedFactorF() * sg_showHitDataRange;

    // calculate the timeout value
    REAL timeout = data.ownerData.speedFactorF() * sg_showHitDataTimeout;

    if (sg_helperHud)
    // write the front hit distance to the stream
        sg_helperShowHitFrontDistH << (frontHit);

    // return if the wall is not close
    if (!wallClose)
        return;

    // get the front before hit position
    eCoord frontBeforeHit = data.sensors.getSensor(FRONT)->before_hit;

    // draw a line from the owner's current position to the front before hit position
    gHelperUtility::debugLine(gRealColor(1, .5, 0), sg_showHitDataHeightFront, timeout, (ownerPos), frontBeforeHit);

    // check if the start position is at the hit position or not
    if (sg_helperShowHitStartAtHitPos)
    {
        // draw debug lines from the front before hit position in the left and right directions
        showHitDebugLines(frontBeforeHit, owner_.Direction(), timeout, data, sg_showHitDataRecursion-1, LEFT);
        showHitDebugLines(frontBeforeHit, owner_.Direction(), timeout, data, sg_showHitDataRecursion-1, RIGHT);
    }
    else
    {
        // draw debug lines from the owner's current position in the left and right directions
        showHitDebugLines(ownerPos, owner_.Direction().Turn(LEFT), timeout, data, sg_showHitDataRecursion-1, LEFT);
        showHitDebugLines(ownerPos, owner_.Direction().Turn(RIGHT), timeout, data, sg_showHitDataRecursion-1, RIGHT);
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
void gHelper::showHitDebugLines(eCoord currentPos, eCoord initDir, REAL timeout, gHelperData &data, int recursion, int sensorDir)
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
    gHelperSensor *sensor = data.sensors.getSensor(currentPos, initDir);
    eCoord hitPos = sensor->before_hit;
    REAL hitDistance = sensor->hit;

    // Check if the hit distance is greater than a certain value.
    bool open = hitDistance > data.ownerData.turnSpeedFactorF() * sg_showHitDataFreeRange;

    // Draw a green line if the hit distance is greater than the specified value, indicating that the path is clear.
    if (open)
    {
        gHelperUtility::debugLine(gRealColor(0, 1, 0), sg_showHitDataHeightSides, timeout, currentPos, hitPos,sg_showHitDataBrightness);
    }
    // Draw a red line if the hit distance is smaller than the specified value, indicating an obstacle in the way.
    else
    {
        gHelperUtility::debugLine(gRealColor(1, 0, 0), sg_showHitDataHeightSides, timeout, currentPos, hitPos,sg_showHitDataBrightness);
    }

    // Recursively call the function to visualize multiple sensor hits.
    showHitDebugLines(hitPos, newDir, timeout, data, recursion, sensorDir);
}

// Function to check if the owner is alive and exists on the grid.
bool gHelper::aliveCheck()
{
    // Return true if the owner exists, is alive, and grid exist.
    return &owner_ && owner_.Alive() && owner_.Grid();
}

#include "../gWall.h"
void gHelper::Activate()
{
    if (!aliveCheck())
        return;

    REAL start;
    if (sg_helperHud) {
        start = tRealSysTimeFloat();
        tColoredString debug;
        debug << "gCycles: " << eGameObject::number_of_gCycles << "\n";
        debug << "Walls: " << sg_netPlayerWalls.Len() << "\n";
        debug << "eGameObjects: " << eGrid::CurrentGrid()->gameObjects.Len() << "\n";
        helperDebugH << debug;
        ownerPosH << roundeCoord(ownerPos);
        ownerDirH << roundeCoord(ownerDir);
        tailPosH << roundeCoord(tailPos);
        tailDirH << roundeCoord(owner_.tailDir);
        sg_helperGameTimeH << se_GameTime();
    }

    owner_.localCurrentTime = se_GameTime();

    data_stored.enemies.detectEnemies();

    if (sg_helperSmartTurning)
        smartTurning->Activate(data_stored);

    if (sg_pathHelper)
        pathHelper->Activate(data_stored);

    if (sg_tailHelper)
        tailHelper->Activate(data_stored);

    if (sg_zoneHelper)
        zoneHelper->Activate(data_stored);

    if (sg_helperEnemyTracers)
        enemyTracers(data_stored);

    if (sg_helperDetectCut)
        detectCut(data_stored, sg_helperDetectCutDetectionRange);

    if (sg_helperShowHit)
        showHit(data_stored);

    if (sg_helperShowTail)
        showTail(data_stored);

    if (sg_helperShowTailTracer)
        showTailTracer(data_stored);

    if (sg_helperShowEnemyTail)
        showEnemyTail(data_stored);

    if (sg_helperShowCorners)
        showCorners(data_stored);

    if (sg_helperAutoBrake)
        autoBrake();

    if (sg_helperTurningBot)
        turningBot(data_stored);

    if (sg_helperHud) {
        REAL time = tRealSysTimeFloat() - start;
        sg_helperActivateTimeH << (time);
    }

}

gHelper &gHelper::Get(gCycle &cycle)
{
    tASSERT(&cycle);

    // create
    if (cycle.helper_.get() == 0)
        cycle.helper_.reset(new gHelper(cycle));

    return  *cycle.helper_;
}

gHelper::~gHelper()
{
}
