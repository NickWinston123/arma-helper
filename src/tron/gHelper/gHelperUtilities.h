#include "defs.h"
#include "gHelper.h"
#include "gHelperVar.h"
#include "../gCycle.h"
#include "tDirectories.h"
#include "tLocale.h"
#include "tSystime.h"
#include "tColor.h"
#include <unordered_set>
#ifndef ArmageTron_GHELPER_UTILITIES
#define ArmageTron_GHELPER_UTILITIES

using namespace helperConfig;
class gHelperSensor;

struct gHelperClosestEnemyData
{
    gCycle *owner_;

    int enemySide;
    bool canCutEnemy, canCutUs;
    bool dangerous;

    bool enemyIsFacingOurLeft, enemyIsFacingOurRight;
    bool enemyIsOnLeft, enemyIsOnRight;
    bool oppositeDirectionofEnemy;

    bool faster(gCycle *helperOwner);
};

struct gHelperEnemiesData
{
    gCycle *owner_;
    std::unordered_set<gCycle *> allEnemies;
    gHelperClosestEnemyData closestEnemy;

    // exist
    // Returns true if the passed in enemy exists and is alive
    // Input: enemy - the enemy to check if it exists and is alive
    // Output: returns true if the passed in enemy exists and is alive, otherwise false
    static bool exist(gCycle *enemy);

    // detectEnemies
    // Detects all existing alive enemies and sets closestEnemy to the closest one.
    // Clears the allEnemies set.
    // Output: returns a pointer to the closest detected enemy.
    gCycle *detectEnemies();
    static gCycle *getClosestEnemy(gCycle *owner_, bool ignoreLocal, bool ignoreOwner, tString ignoreList = tString(""));
};

struct gSmartTurningCornerData
{
    gSmartTurningCornerData *lastCorner;
    eCoord currentPos;
    eCoord lastPos;
    gSensorWallType type;
    REAL distanceFromPlayer;
    REAL turnTime;
    REAL noticedTime;
    REAL ignoredTime;
    REAL updatedTime;
    bool exist;
    bool infront;
    ~gSmartTurningCornerData() {}
    void linkLastCorner(gSmartTurningCornerData *lastCorner_) { lastCorner = lastCorner_; }
    // Returns the time until required a turn should occur given a speed
    REAL getTimeUntilTurn(REAL speed);

    // Checks if a position is in front of the direction vector
    bool isInfront(eCoord pos, eCoord dir);
    bool isInfront(eCoord pos, eCoord dir, eCoord posToCheck);

    // Finds a corner based on the sensor data, updates relevant data in the
    // gSmartTurningCornerData object, and returns a boolean indicating if the corner was found
    bool findCorner(const std::shared_ptr<gHelperSensor> &sensor, gHelper &helper);
};

struct gHelperRubberData
{
    gHelper *helper_;
    gCycle *owner_;
    REAL rubberAvailable,
        rubberEffectiveness,
        rubberFactor,
        rubberUsedRatio,
        rubberTimeLeft;

    REAL rubberAvailableF()
    {
        calculate();
        return rubberAvailable;
    }
    REAL rubberEffectivenessF()
    {
        calculate();
        return rubberEffectiveness;
    }
    REAL rubberFactorF()
    {
        calculate();
        return rubberFactor;
    }
    REAL rubberUsedRatioF()
    {
        calculate();
        return rubberUsedRatio;
    }
    REAL rubberTimeLeftF()
    {
        calculate();
        return rubberTimeLeft;
    }

    gHelperRubberData() {}
    gHelperRubberData(gHelper *helper, gCycle *owner) : helper_(helper), owner_(owner) {}

    // Calculates and updates rubber related values the cycle
    // such as rubberAvailable, rubberEffectiveness, rubberFactor, rubberUsedRatio, and rubberTimeLeft.
    void calculate();
};

struct gTurnData
{
public:
    int direction;
    REAL turnTime;
    REAL nextTime;
    int numberOfTurns;
    bool exist;
    std::string reason;
    int noTurns;
    gTurnData() {}

    gTurnData(int direction_, int numberOfTurns_, std::string reason_) : direction(direction_),
                                                                         numberOfTurns(numberOfTurns_),
                                                                         exist(true),
                                                                         reason(reason_) {}

    gTurnData(bool) : exist(false) {}

    void reset()
    {
        direction = (0);
        numberOfTurns = (0);
        exist = (false);
        reason = ("");
    }

    void set(bool)
    {
        noTurns++;
        reset();
    }

    void set(int direction_, int numberOfTurns_, std::string reason_)
    {
        direction = (direction_);
        numberOfTurns = (numberOfTurns_);
        exist = true;
        reason = (reason_);
        noTurns = 0;
    }
};

class gHelperSensors
{
public:
    gHelperSensors(std::shared_ptr<gHelperSensor> front_, std::shared_ptr<gHelperSensor> left_, std::shared_ptr<gHelperSensor> right_);

    std::shared_ptr<gHelperSensor> front;
    std::shared_ptr<gHelperSensor> left;
    std::shared_ptr<gHelperSensor> right;
};

class gHelperSensorsData
{
public:
    gHelperSensorsData(){};
    gHelperSensorsData(gCycle *owner);

    std::shared_ptr<gHelperSensor> getSensor(int dir, bool newSensor = false);
    std::shared_ptr<gHelperSensor> getSensor(eCoord start, int dir, bool newSensor = false);

    std::shared_ptr<gHelperSensor> getSensor(eCoord start, eCoord dir, REAL detectRange = sg_helperSensorRange);

    gCycle *owner_;
    std::shared_ptr<gHelperSensor> front_stored;
    std::shared_ptr<gHelperSensor> left_stored;
    std::shared_ptr<gHelperSensor> right_stored;
};

struct gHelperOwnerData
{
    gCycle *owner_;
    // calculates the factor based on the owner's speed
    REAL speedFactorF();

    // calculates the factor based on the owner's turning speed
    REAL turnSpeedFactorF();

    REAL turnTimeF(int dir);

    // calculates the percentage based on the turn speed factor
    REAL turnSpeedFactorPercentF()
    {
        return (1 / turnSpeedFactorF());
    }
    // lag factor
    REAL lagFactorF();

    // calculates the distance based on the turn speed factor
    REAL turnDistanceF()
    {
        return (turnSpeedFactorF() / 100);
    }

    gHelperOwnerData() {}
    gHelperOwnerData(gCycle *owner) : owner_(owner) {}
};

struct gHelperData
{
    gHelperData() {}

    gHelperOwnerData ownerData;

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
    gHelperRubberData rubberData;

    // Pointer to the gHelperSensorsData object
    gHelperSensorsData sensors;
};

struct debugParams
{
    bool emptyString;
    bool spamProtection;
    debugParams(bool empty, bool spamProtect = true) : emptyString(empty), spamProtection(spamProtect) {}
};

using namespace helperConfig;
class gHelperUtility
{
public:
    static void debugLine(tColor color, REAL height, REAL timeout,
                          eCoord start, eCoord end, REAL brightness = 1);

    static void debugBox(tColor color, eCoord center,
                         REAL radius, REAL timeout, REAL height = 1);

    static tColor tStringTotColor(tString string);

    template <typename T>
    static void Debug(const std::string &sender, const std::string &description, T value, debugParams *params)
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

        if (sg_helperDebugSpamFilter && params->spamProtection && (lastMessageIsSame || delayNotPassed))
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

        if (params->spamProtection)
        {
            lastHelperDebugMessage = description;
        }

        if (params->emptyString)
        {
            debugMessage += "\n";
        }
        else if constexpr (std::is_same<T, std::string>::value)
        {
            if (value == "")
            {
                debugMessage += "\n";
            }
            else
            {
                debugMessage += value + "\n";
            }
        }
        else if constexpr (std::is_pointer<T>::value)
        {
            debugMessage += std::to_string(*value) + "\n";
        }
        else
        {
            tString str;
            str << value;
            debugMessage += (str) + "\n";
        }
        delete params;
        con << debugMessage;
        if (helperConfig::sg_helperDebugLog)
        {
            DebugLog(debugMessage);
        }
    }

    static void DebugLog(std::string message)
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

    template <typename T>
    static void Debug(const std::string &sender, const std::string &description, T value, bool spamProtection = true)
    {
        Debug(sender, description, value, new debugParams(false, spamProtection));
    }

    // Required to stop making empty strings printing as 0's
    static void Debug(const std::string &sender, const std::string &description, bool spamProtection = true)
    {
        Debug(sender, description, "", new debugParams(true, spamProtection));
    }

    static REAL BytesToMB(REAL bytes)
    {
        return bytes / 1024 / 1024;
    }
    // isClose checks if the distance between the owner cycle's position and the given position
    // is within a certain factor of closeness.
    // pos: the position to check the distance to
    // closeFactor: the factor of closeness to compare the distance to
    // Returns: true if the distance is within the closeFactor, false otherwise
    static bool isClose(gCycle *owner_, eCoord pos, REAL closeFactor);
    static bool isClose(gCycle *owner_, gCycle *enemy_, REAL closeFactor);
};

// See how close two coordinates are, lower the threshold the more strict the comparison
static bool directionsAreClose(const eCoord &dir1, const eCoord &dir2, REAL threshold = 0.1)
{

    return dir1.Dot(dir2) >= 1 - threshold;
}

// Calculates the Euclidean distance between two points
static REAL eCoordDistance(const eCoord &p1, const eCoord &p2)
{
    eCoord difference = p1 - p2;
    return difference.Norm();
}

// Rounds a real number value to the specified precision
static REAL customRound(REAL value, int precision)
{
    REAL multiplier = pow(10, precision);
    return round(value * multiplier) / multiplier;
}

// Rounds each coordinate of an eCoord
static eCoord roundeCoord(eCoord coord, int precision = 1)
{
    return eCoord(customRound(coord.x, precision), customRound(coord.y, precision));
}

#endif
