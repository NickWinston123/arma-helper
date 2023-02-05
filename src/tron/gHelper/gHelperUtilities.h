#include "defs.h"
#include "gHelper.h"
#include "../gCycle.h"
#include "tDirectories.h"
#include "tLocale.h"
#include <unordered_set>
#ifndef ArmageTron_GHELPER_UTILITIES
#define ArmageTron_GHELPER_UTILITIES

class gHelperUtility
{   
    public:
    static void debugLine(gRealColor color, REAL height, REAL timeout,
                    eCoord start,eCoord end, REAL brightness = 1);

    static void debugBox(gRealColor color, eCoord center, REAL radius, REAL timeout);
};

struct gHelperSensors
{
    gSensor* front;
    gSensor* left;
    gSensor* right;

    gHelperSensors(gSensor* front_, gSensor* left_, gSensor* right_);
};


struct gHelperSensorsData
{
    gCycle *owner_;
    gSensor *front_stored;
    gSensor *left_stored;
    gSensor *right_stored;

    gHelperSensorsData(gCycle *owner);
    gSensor* getSensor(int dir, bool newSensor = false);
    gSensor* getSensor(eCoord start, int dir, bool newSensor = false);
    gSensor* getSensor(eCoord start, eCoord dir);
}; 

struct gHelperData
{
    gCycle *owner_;
    REAL *ownerSpeed;
    gHelperSensorsData &sensors;
    REAL speedFactor;
    REAL turnSpeedFactor;
    REAL turnSpeedFactorPercent;
    REAL turnDistance;
    REAL thinkAgain;
    REAL turnDir;
    REAL turnTime;

    // calculates the factor based on the owner's speed
    REAL speedFactorF() {
        return (1/(*ownerSpeed));
    }
    // calculates the factor based on the owner's turning speed
    REAL turnSpeedFactorF();

    // calculates the percentage based on the turn speed factor
    REAL turnSpeedFactorPercentF() {
        return (1/turnSpeedFactorF());
    }
    
    // calculates the distance based on the turn speed factor
    REAL turnDistanceF() {
        return (turnSpeedFactorF()/100);
    }

    void Load(  REAL a_speedFactor, REAL a_turnSpeedFactor,
                REAL a_turnSpeedFactorPercent, REAL a_turnDistance, REAL a_thinkAgain,
                REAL a_turnDir, REAL a_turnTime);
    gHelperData(gHelperSensorsData *sensors,gCycle *owner);
    gHelperData(gHelperSensorsData &sensors_, REAL a_speedFactor, REAL a_turnSpeedFactor,
                REAL a_turnSpeedFactorPercent, REAL a_turnDistance, REAL a_thinkAgain,
                REAL a_turnDir, REAL a_turnTime);
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


struct gHelperEnemiesData
{
    gCycle* owner_;
    std::unordered_set<gCycle*> allEnemies;
    gCycle* closestEnemy;

    // exist
    // Returns true if the passed in enemy exists and is alive
    // Input: enemy - the enemy to check if it exists and is alive
    // Output: returns true if the passed in enemy exists and is alive, otherwise false
    bool exist(gCycle* enemy);

    // detectEnemies
    // Detects all existing alive enemies and sets closestEnemy to the closest one. 
    // Clears the allEnemies set.
    // Output: returns a pointer to the closest detected enemy.
    gCycle* detectEnemies();
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
    bool exist;
    bool infront;
    void linkLastCorner(gSmartTurningCornerData* lastCorner_) {lastCorner = lastCorner_;}
    // Returns the time until a turn will occur given a speed
    REAL getTimeUntilTurn(REAL speed);
    
    // Checks if a position is in front of the direction vector
    bool isInfront(eCoord pos, eCoord dir);

    // Finds a corner based on the sensor data, updates relevant data in the 
    // gSmartTurningCornerData object, and returns a boolean indicating if the corner was found
    bool findCorner(gHelperData &data, const gSensor *sensor, gHelper *helper);
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

    gHelperRubberData(gHelper *helper, gCycle *owner) : helper_(helper), owner_(owner) {}
    
    // Calculates and updates rubber related values the cycle
    // such as rubberAvailable, rubberEffectiveness, rubberFactor, rubberUsedRatio, and rubberTimeLeft.
    void calculate();
};

struct gTurnData
{
public:
    int direction;
    int numberOfTurns;
    bool exist;
    std::string reason;
    int noTurns;
    gTurnData();

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
#endif
