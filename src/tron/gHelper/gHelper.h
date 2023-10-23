#ifndef ArmageTron_HELPER_H
#define ArmageTron_HELPER_H

#include "../gStuff.h"

#include "tString.h"

#include "gHelperVar.h"

#include "specialized/gTurnHelper.h"
#include "specialized/gSmartTurning.h"
#include "specialized/gZoneHelper.h"
#include "specialized/gPathHelper.h"
#include "specialized/gTailHelper.h"

class gAIPlayer;

class gHelper
{
    friend class gCycle;
    friend class gSmartTurning;
    friend class gSmarterBot;
    friend class gPathHelper;
    friend class gTailHelper;
    friend class gZoneHelper;

public:
    /**
     * Constructor for the `gHelper` class.
     *
     * @param owner Pointer to the `gCycle` object that this helper is associated with.
     */
    gHelper(gCycle &owner);

    /**
     * Returns the current time.
     *
     * @return Current time.
     */
    REAL CurrentTime();

    /**
     * Detects cuts (collisions) between owner and the closest enemy's cycle.
     *
     * @param data The `gHelperData` object to store the detection results.
     * @param detectionRange The range for detection, in units.
     */
    void detectCut(gHelperData &data, int detectionRange);

    /**
     * Detects enemy tracers (trails) in front of the cycle.
     *
     * @param data The `gHelperData` object to store the detection results.
     * @param detectionRange The range for detection, in units.
     * @param timeout Time after which the detection will stop.
     */
    void enemyTracers(gHelperData &data);

    /**
     * Shows the tail of the cycle.
     *
     * @param data The `gHelperData` object to store the detection results.
     */
    void showTail(gHelperData &data);

    /**
     * Shows the hit position.
     *
     * @param data The `gHelperData` object to store the detection results.
     */
    void showHit(gHelperData &data);

    /**
     * Checks if the cycle is alive.
     *
     * @return True if the cycle is alive, false otherwise.
     */
    bool aliveCheck();

    /**
     * Checks if the cycle is driving straight.
     *
     * @return True if the cycle is driving straight, false otherwise.
     */
    bool drivingStraight();

    /**
     * Makes the cycle follow its tail.
     */
    void followTail();

    /**
     * Determines if the cycle can see the specified target.
     *
     * @param target The target to check visibility for.
     * @param passthrough The minimum distance the target must be visible for, in units.
     *
     * @return True if the target is visible, false otherwise.
     */
    bool canSeeTarget(eCoord target, REAL passthrough);

    /**
     * findCorners - Identify the left and right corners in the cycle's field of view
     * @param data: structure to store the data gathered by the sensors
     */
    void findCorners(gHelperData &data);

    /**
     * showCorner - Visualize the specified corner
     * @param data: structure to store the data gathered by the sensors
     * @param corner: data structure representing the corner
     * @param timeout: time in seconds for the visualization to persist
     */
    void showCorner(gHelperData &data, gSmartTurningCornerData &corner, REAL timeout);

    /**
     * showCorners - Visualize both the left and right corners
     * @param data: structure to store the data gathered by the sensors
     */
    void showCorners(gHelperData &data);

    /**
     * showTailTracer - Visualize the tail of the cycle
     * @param data: structure to store the data gathered by the sensors
     */
    void showTailTracer(gHelperData &data);

    /**
     * autoBrake - Automatically apply brakes to the cycle
     */
    void autoBrake();
    static void autoBrake(gCycle &owner, REAL min, REAL max);
    static bool autoBrakeShouldBrake(gCycle &owner, REAL min, REAL max, bool current = false);

    void rubberRatioBrake(gHelperData &data);

    /**
     * showEnemyTail - Visualize the tail of the closest enemy cycle
     * @param data: structure to store the data gathered by the sensors
     */
    void showEnemyTail(gHelperData &data);

    /**
     * showHitDebugLines - Visualize the line between the position and direction
     * @param pos: starting point of the line
     * @param dir: direction of the line
     * @param timeout: time in seconds for the visualization to persist
     * @param data: structure to store the data gathered by the sensors
     * @param recursion: number of times the function has been called recursively
     * @param sensorDir: direction of the sensor being used
     */
    void showHitDebugLines(eCoord pos, eCoord dir, REAL timeout, gHelperData &data, int recursion, int sensorDir, bool firstRun = false);

    /**
     * getCorner - Retrieve data for a specified corner
     * @param corner: integer representing the corner (0 for left, 1 for right)
     * @return data structure representing the corner
     */
    gSmartTurningCornerData &getCorner(int corner);

    /**
     * closestCorner - Determine the closest corner to a specified point
     * @param center: center point
     * @param radius: distance around the center point to search for a corner
     * @return the closest corner
     */
    eCoord closestCorner(eCoord center, REAL radius);

    void turningBot(gHelperData &data);

    void traceLeft();
    void traceRight();
    void trace(gHelperData &data, int dir);
    /**
     * Activates the helper, setting up necessary data and pointers.
     */
    void Activate();

    // function to get the instance of gHelper
    static gHelper &Get(gCycle &owner);

    /**
     * Destructor for gHelper
     */
    ~gHelper();

public:
    // Reference to the owner gCycle object
    gCycle &owner_;

    // Reference to the player ePlayerNetID object
    ePlayerNetID &player_;

    // Reference to the closest enemy gCycle object
    gCycle &closestEnemy;

    // Reference to the owner position as an eCoord object
    eCoord &ownerPos;

    // Reference to the owner direction as an eCoord object
    eCoord &ownerDir;

    // Reference to the tail position as an eCoord object
    eCoord &tailPos;

    // Reference to the owner speed as a REAL
    REAL &ownerSpeed;

    // Length of the owner's wall
    REAL ownerWallLength;

    // Delay in the owner's turn
    REAL ownerTurnDelay;

    // Reference to the stored gHelperData object
    gHelperData &data_stored;

    // emergency turn object
    std::unique_ptr<gTurnHelper> turnHelper;

    // Unique pointer to the gSmartTurning object
    std::unique_ptr<gSmartTurning> smartTurning;

    // Unique pointer to the gPathHelper object
    std::unique_ptr<gPathHelper> pathHelper;

    // Unique pointer to the gTailHelper object
    std::unique_ptr<gTailHelper> tailHelper;

    // Unique pointer to the gZoneHelper object
    std::unique_ptr<gZoneHelper> zoneHelper;
};
#endif
