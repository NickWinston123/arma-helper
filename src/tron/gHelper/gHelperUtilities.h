#include "defs.h"
#include "gHelper.h"
#include "gHelperVar.h"
#include "../gCycle.h"
#include "tDirectories.h"
#include "tLocale.h"
#include "tSystime.h"
#include <unordered_set>
#ifndef ArmageTron_GHELPER_UTILITIES
#define ArmageTron_GHELPER_UTILITIES

using namespace helperConfig;
class gHelperSensor;



struct gHelperEnemiesData
{
    gCycle* owner_;
    std::unordered_set<gCycle*> allEnemies;
    gCycle* closestEnemy;

    // exist
    // Returns true if the passed in enemy exists and is alive
    // Input: enemy - the enemy to check if it exists and is alive
    // Output: returns true if the passed in enemy exists and is alive, otherwise false
    static bool exist(gCycle* enemy);

    // detectEnemies
    // Detects all existing alive enemies and sets closestEnemy to the closest one.
    // Clears the allEnemies set.
    // Output: returns a pointer to the closest detected enemy.
    gCycle* detectEnemies();
    static gCycle* getClosestEnemy(gCycle *owner_);
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
    ~gSmartTurningCornerData(){}
    void linkLastCorner(gSmartTurningCornerData* lastCorner_) {lastCorner = lastCorner_;}
    // Returns the time until required a turn should occur given a speed
    REAL getTimeUntilTurn(REAL speed);

    // Checks if a position is in front of the direction vector
    bool isInfront(eCoord pos, eCoord dir);
    bool isInfront(eCoord pos, eCoord dir, eCoord posToCheck);

    // Finds a corner based on the sensor data, updates relevant data in the
    // gSmartTurningCornerData object, and returns a boolean indicating if the corner was found
    bool findCorner(const gHelperSensor *sensor, gHelper &helper);
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


struct gHelperSensors
{
    gHelperSensor* front;
    gHelperSensor* left;
    gHelperSensor* right;

    gHelperSensors(gHelperSensor* front_, gHelperSensor* left_, gHelperSensor* right_);
};


struct gHelperSensorsData
{
    gCycle *owner_;
    gHelperSensor *front_stored;
    gHelperSensor *left_stored;
    gHelperSensor *right_stored;

    gHelperSensorsData() {};
    ~gHelperSensorsData();
    gHelperSensorsData(gCycle *owner);
    gHelperSensor* getSensor(int dir, bool newSensor = false);
    gHelperSensor* getSensor(eCoord start, int dir, bool newSensor = false);
    gHelperSensor* getSensor(eCoord start, eCoord dir, REAL detectRange = sg_helperSensorRange);
};

struct gHelperOwnerData {
    gCycle *owner_;
    // calculates the factor based on the owner's speed
    REAL speedFactorF();

    // calculates the factor based on the owner's turning speed
    REAL turnSpeedFactorF();

    // calculates the percentage based on the turn speed factor
    REAL turnSpeedFactorPercentF() {
        return (1/turnSpeedFactorF());
    }
    // lag factor
    REAL lagFactorF();

    // calculates the distance based on the turn speed factor
    REAL turnDistanceF() {
        return (turnSpeedFactorF()/100);
    }

    gHelperOwnerData() {}
    gHelperOwnerData(gCycle *owner) : owner_(owner) {}

};

struct gHelperData
{
    gHelperData(){}

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

using namespace helperConfig;
class gHelperUtility
{
public:
    static void debugLine(gRealColor color, REAL height, REAL timeout,
                          eCoord start, eCoord end, REAL brightness = 1);

    static void debugBox(gRealColor color, eCoord center, REAL radius, REAL timeout);

    template <typename T>
    static void Message(const std::string &label, const std::string &sender,
                        const std::string &description, T value, bool showTimeStamp = true,
                        bool emptyString = false, bool spamProtection = false, bool showTime = true)
    {

        if (spamProtection)
        {
            static std::string lastMessage;
            static std::string lastSender;
            static float lastMessageTimeStamp = 0;

            bool lastMessageIsSame = (lastMessage == description && lastSender == sender);
            float currentTime = tSysTimeFloat();
            bool delayNotPassed = (currentTime - lastMessageTimeStamp) < sg_helperDebugDelay;

            if (lastMessageIsSame || delayNotPassed)
            {
                return;
            }

            lastMessageTimeStamp = currentTime;
            lastSender = sender;
            lastMessage = description;
        }

        tString message;
        if (showTimeStamp)
        {
            message << (showTime ? getTimeString(false) + " ": "[" + std::to_string(tSysTimeFloat()) + "] ");
        }


        message << label << " [" << sender << "]: " << description << " ";

        if (emptyString)
        {
            message << "\n";
        }
        else if constexpr (std::is_same<T, std::string>::value)
        {
            if (value == "")
            {
                message << "\n";
            }
            else
            {
                message << value + "\n";
            }
        }
        else if constexpr (std::is_pointer<T>::value)
        {
            message << std::to_string(*value) + "\n";
        }
        else
        {
            tString str;
            str << value;
            message << str + "\n";
        }

        con << message;

    }

    template <typename T>
    static void Debug(const std::string &sender, const std::string &description, T value, bool spamProtection = true)
    {
        Message("HELPERDEBUG", sender, description, value, true, false, spamProtection, false);
    }

    static void Debug(const std::string &sender, const std::string &description, bool spamProtection = true)
    {
        Message("HELPERDEBUG", sender, description, "", true, true, spamProtection, false);
    }

    template <typename T>
    static void Message(const std::string &sender, const std::string &description, T value, bool spamProtection = true)
    {
        Message("HELPERDEBUG", sender, description, value, true, false, spamProtection, true);
    }

    static void Message(const std::string &sender, const std::string &description, bool spamProtection = true)
    {
        Message("HELPERDEBUG", sender, description, "", true, true, spamProtection, true);
    }

    // isClose checks if the distance between the owner cycle's position and the given position
    // is within a certain factor of closeness.
    // pos: the position to check the distance to
    // closeFactor: the factor of closeness to compare the distance to
    // Returns: true if the distance is within the closeFactor, false otherwise
    static bool isClose(gCycle *owner_, eCoord pos, REAL closeFactor);
};

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


// See how close two coordinates are, lower the threshold the more strict the comparison
static bool directionsAreClose(const eCoord &dir1, const eCoord &dir2, REAL threshold = 0.1) {

    return dir1.Dot(dir2) >= 1 - threshold;

}

// Calculates the Euclidean distance between two points
static REAL eCoordDistance(const eCoord &p1, const eCoord &p2)
{
    eCoord difference = p1 - p2;
    return difference.Norm();
}

// Rounds a real number value to the specified precision
static REAL customRound(REAL value, int precision) {
    REAL multiplier = pow(10, precision);
    return round(value * multiplier) / multiplier;
}

// Rounds each coordinate of an eCoord
static eCoord roundeCoord(eCoord coord, int precision = 1){
    return eCoord(customRound(coord.x,precision),customRound(coord.y,precision));
}

#endif
