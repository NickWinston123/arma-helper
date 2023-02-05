#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_SMARTTURNING
#define ArmageTron_GHELPER_SMARTTURNING


class Sensor;

// gHelperEmergencyTurn is a class that manages emergency turning for a cycle in the game
// using the chatbot logic to get a turn 
class gHelperEmergencyTurn
{
public:
    // default constructor
    gHelperEmergencyTurn();

    // constructor taking a gHelper and a gCycle as arguments
    gHelperEmergencyTurn(gHelper *helper, gCycle *owner) : helper_(helper), owner_(owner) {}

    // returns a reference to the gHelperEmergencyTurn instance for the given helper and owner
    static gHelperEmergencyTurn &Get(gHelper *helper, gCycle *owner);

    // solve the turn required to escape the situation based on chatbot logic
    // returns a pointer to the gTurnData object that stores the turn data
    gTurnData *getTurn();

    // solve the turn required to escape the situation based on AIBase logic
    // returns a pointer to the gTurnData object that stores the turn data
    gTurnData *getTurnAIBase() {};
    
    // calculates the distance between two sensors
    REAL Distance(Sensor const &a, Sensor const &b);

    // performs the actual turn
    int ActToTurn(uActionPlayer *action);

    // displays a debugging message
    template <class T>
    void BotDebug(std::string description, T value = "");

    // displays a debugging message without a value
    void BotDebug(std::string description);

private:
    // pointer to the gTurnData object that stores the turn data
    gTurnData* turnData;

    // the owner of the chatbot
    gCycle *owner_; 

    // pointer to the gHelper instance
    gHelper *helper_;
};


class gSmartTurning
{
public:
    // constructor to create an instance of gSmartTurning with a helper and an owner cycle
    gSmartTurning(gHelper *helper, gCycle *owner);
    // function to get the instance of gSmartTurning
    static gSmartTurning &Get(gHelper *helper, gCycle *owner);

    // Activate function to activate the smart turning feature
    void Activate(gHelperData &data);

    // isClose checks if the distance between the owner cycle's position and the given position
    // is within a certain factor of closeness.
    // pos: the position to check the distance to
    // closeFactor: the factor of closeness to compare the distance to
    // Returns: true if the distance is within the closeFactor, false otherwise
    bool isClose(eCoord pos, REAL closeFactor);

    // function to make the cycle turn to automatically trace corners
    void smartTurningPlan(gHelperData &data);

    // function to make the cycle survive by determining which turns are possible and blocking if required
    void smartTurningSurvive(gHelperData &data);

    // function to make the cycle turn in the opposite direction when a direction is blocked
    void smartTurningOpposite(gHelperData &data);

    // function to make the cycle follow its tail
    void followTail(gHelperData &data);

    // function to make the cycle turn in response to a front bot
    void smartTurningFrontBot(gHelperData &data);

    // function to check if a specific turn is possible given rubber or space factors
    bool canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor = 0);

    // function to make the cycle survive by tracing the wall in the direction the last turn was blocked
    void smartTurningSurviveTrace(gHelperData &data);

    // function to calculate the rubber factor
    void calculateRubberFactor(REAL rubberMult, REAL rubberFactorMult);

    // function to make a turn if possible
    bool makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor = 0);

    // function to check if a turn can help the cycle survive a left or right turn given speed and rubber factors
    void canSurviveTurn(gHelperData &data,
                        REAL &canSurviveLeftTurn,
                        REAL &canSurviveRightTurn,
                        bool &closedIn,
                        bool &blockedBySelf,
                        REAL freeSpaceFactor = 0);

private:
    // owner cycle
    gCycle *owner_;

    // helper
    gHelper *helper_;

    // data
    gHelperData *data_;

    // time of the last turn attempt
    REAL &lastTurnAttemptTime;

    // direction of the last turn attempt
    REAL &lastTurnAttemptDir;

    // time of the last turn
    REAL &lastTurnTime;

    // time to ignore turning
    REAL &turnIgnoreTime;

    // direction of the last turn
    REAL &lastTurnDir; // -1 left , 1 right

    // block turn value (0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH)
    REAL &blockTurn;

    // force turn value (0 = NONE, -1 = LEFT, 1 = RIGHT)
    REAL &forceTurn;

public:
    // emergency turn object
    std::unique_ptr<gHelperEmergencyTurn> emergencyTurn;

private:
    // last time turn to tail was taken
    REAL lastTailTurnTime;
};

#endif