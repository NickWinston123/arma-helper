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

static float lastHelperDebugMessageTimeStamp;
static std::string lastHelperDebugMessage = "", lastHelperDebugSender = "";

#ifndef ArmageTron_GHELPER_Debug
#define ArmageTron_GHELPER_Debug
#include "tSysTime.h"
class HelperDebug
{
public:
template <typename T>
static void Debug(const std::string &sender, const std::string &description, T value, bool spamProtection = true)
{
    if (!helperConfig::sg_helperDebug)
    {
        return;
    }

    if (tIsInList(helperConfig::sg_helperDebugIgnoreList, tString(sender)))
    {
        return;
    }

    bool lastMessageIsSame = (lastHelperDebugMessage == description && lastHelperDebugSender == sender);
    float currentTime = tSysTimeFloat();
    bool delayNotPassed = (currentTime - lastHelperDebugMessageTimeStamp) < helperConfig::sg_helperDebugDelay;

    if (spamProtection && (lastMessageIsSame || delayNotPassed))
    {
        return;
    }

    lastHelperDebugMessageTimeStamp = currentTime;
    lastHelperDebugSender = sender;

    std::string debugMessage;

    if (helperConfig::sg_helperDebugTimeStamp)
    {
        debugMessage += "[" + std::to_string(currentTime) + "] ";
    }

    debugMessage += "0xff8888HELPERDEBUG 0xaaaaaa[0xff8888" + sender + "0xaaaaaa]0xffff88: " + description + " ";

    if (spamProtection)
    {
        lastHelperDebugMessage = description;
    }

    if constexpr (std::is_same<T, std::string>::value) {
        debugMessage += value + "\n";
    } else if constexpr (std::is_pointer<T>::value) {
        debugMessage += std::to_string(*value) + "\n";
    } else {
        debugMessage += std::to_string(value) + "\n";
    }

    con << debugMessage;
}

};
#endif

class gAIPlayer;

class gHelper {
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
        gHelper(gCycle *owner);

        // function to get the instance of gSmartTurning
        static gHelper &Get(gCycle *owner);

        /**
         * Activates the helper, setting up necessary data and pointers.
         */
        void Activate();

        /**
         * Returns the current time.
         *
         * @return Current time.
         */
        REAL CurrentTime();

        /**
         * Detects cuts (collisions) in front of the cycle.
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
        void enemyTracers(gHelperData &data, int detectionRange, REAL timeout);

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
        bool canSeeTarget(eCoord target,REAL passthrough);


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
        void showHitDebugLines(eCoord pos, eCoord dir, REAL timeout, gHelperData &data, int recursion, int sensorDir);

        /**
         * isClose - Check if a point is close to another
         * @param pos: position of the point
         * @param closeFactor: value to determine proximity
         * @return true if the point is close, false otherwise
         */
        bool isClose(eCoord pos, REAL closeFactor);

        /**
         * getCorner - Retrieve data for a specified corner
         * @param corner: integer representing the corner (0 for left, 1 for right)
         * @return data structure representing the corner
         */
        gSmartTurningCornerData * getCorner(int corner);

        /**
         * closestCorner - Determine the closest corner to a specified point
         * @param center: center point
         * @param radius: distance around the center point to search for a corner
         * @return the closest corner
         */
        eCoord closestCorner(eCoord center, REAL radius);

        /**
         * Destructor for gHelper
         */
        ~gHelper();

        public:
        // Pointer to the owner gCycle object
        gCycle *owner_;

        // Pointer to the player ePlayerNetID object
        ePlayerNetID *player_;

        // Pointer to the AI player gAIPlayer object
        gAIPlayer *aiPlayer;

        // Boolean flag indicating if the AI player was created
        bool aiCreated;

        // Pointer to the closest enemy gCycle object
        gCycle *closestEnemy;

        // Pointer to the owner position as an eCoord object
        eCoord *ownerPos;

        // Pointer to the owner direction as an eCoord object
        eCoord *ownerDir;

        // Pointer to the tail position as an eCoord object
        eCoord *tailPos;

        // Pointer to the owner speed as a REAL
        REAL *ownerSpeed;

        // Length of the owner's wall
        REAL ownerWallLength;

        // Delay in the owner's turn
        REAL ownerTurnDelay;

        // Pointer to the gHelperSensorsData object
        gHelperSensorsData * sensors_;

        // Pointer to the stored gHelperData object
        gHelperData *data_stored;

        // gSmartTurningCornerData object for the left corner
        gSmartTurningCornerData leftCorner;

        // gSmartTurningCornerData object for the right corner
        gSmartTurningCornerData rightCorner;

        // gSmartTurningCornerData object for the last left corner
        gSmartTurningCornerData lastLeftCorner;

        // gSmartTurningCornerData object for the last right corner
        gSmartTurningCornerData lastRightCorner;

        // gHelperEnemiesData object to store information about enemies
        gHelperEnemiesData enemies;

        // Pointer to the gHelperRubberData object
        gHelperRubberData * rubberData;

        // emergency turn object
        std::unique_ptr< gTurnHelper > turnHelper;

        // Unique pointer to the gSmartTurning object
        std::unique_ptr< gSmartTurning > smartTurning;

        // Unique pointer to the gPathHelper object
        std::unique_ptr< gPathHelper > pathHelper;

        // Unique pointer to the gTailHelper object
        std::unique_ptr< gTailHelper > tailHelper;

        // Unique pointer to the gZoneHelper object
        std::unique_ptr< gZoneHelper > zoneHelper;
        
};
#endif
