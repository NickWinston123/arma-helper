#include "defs.h"
#include "gHelper.h"
#include "../gCycle.h"
#include "tDirectories.h"
#include "tLocale.h"
#ifndef ArmageTron_GHELPER_UTILITIES
#define ArmageTron_GHELPER_UTILITIES


#include <unordered_set>

// void DebugLog(std::string message);
// REAL eCoordDistance(const eCoord &p1, const eCoord &p2);
// REAL customRound(REAL value, int precision);
// eCoord roundeCoord(eCoord coord);

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



static REAL eCoordDistance(const eCoord &p1, const eCoord &p2)
{
    eCoord difference = p1 - p2;
    return difference.Norm();
}

static REAL customRound(REAL value, int precision) {
    REAL multiplier = pow(10, precision);
    return round(value * multiplier) / multiplier;
}


static eCoord roundeCoord(eCoord coord){
    return eCoord(customRound(coord.x,1),customRound(coord.y,1));
}

struct gHelperEnemiesData
{
    gCycle* owner_;
    std::unordered_set<gCycle*> allEnemies;
    gCycle* closestEnemy;

    bool exist(gCycle* enemy);
    gCycle* detectEnemies();
};

struct gSmartTurningCornerData
{
    eCoord currentPos;
    eCoord lastPos;
    gSensorWallType type;
    REAL distanceFromPlayer;
    REAL turnTime;
    REAL noticedTime;
    REAL ignoredTime;
    bool exist;
    bool infront;

    REAL getTimeUntilTurn(REAL speed);
    bool isInfront(eCoord pos, eCoord dir);
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
    void calculate();
};

struct gTurnData{
    public:
        int direction;
        int numberOfTurns;
        bool exist;
        std::string reason;
    gTurnData   (int direction_, int numberOfTurns_, std::string reason_):
    direction(direction_),
    numberOfTurns(numberOfTurns_),
    exist(true),
    reason(reason_) { }

    gTurnData   (bool):
    exist(false) { }
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

    REAL speedFactorF() {
        return (1/(*ownerSpeed));
    }
    REAL turnSpeedFactorF();

    REAL turnSpeedFactorPercentF() {
        return (1/turnSpeedFactorF());
    }
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


#endif
