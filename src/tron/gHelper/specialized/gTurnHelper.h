#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"

#ifndef ArmageTron_GHELPER_TURN
#define ArmageTron_GHELPER_TURN

class gSmarterBot;
class Sensor;

struct gSurviveData
{
    bool canSurviveLeftTurn = true;
    bool canSurviveRightTurn = true;
    bool canTurnLeftRubber = true;
    bool canTurnRightRubber = true;
    bool canTurnLeftSpace = true;
    bool canTurnRightSpace = true;
    bool trapped;
    bool closedIn;
    bool blockedBySelf;
    bool exist;
    tString debug = tString("");

    gSurviveData() : exist(true){};
    gSurviveData(bool) : exist(false){};
};

// gTurnHelper is a class that manages emergency turning for a cycle in the game
// using the chatbot logic to get a turn
class gTurnHelper
{
public:
    // default constructor
    gTurnHelper();

    // constructor taking a gHelper and a gCycle as arguments
    gTurnHelper(gHelper &helper, gCycle &owner);

    // returns a reference to the gTurnHelper instance for the given helper and owner
    static gTurnHelper &Get(gHelper &helper, gCycle &owner);

    // function to check if a specific turn is possible given rubber or space factors
    bool canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor = 0);

    // function to make a turn if possible
    bool makeTurnIfPossible(int dir, REAL spaceFactor = 0);
    bool makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor = 0);

    // function to check if a turn can help the cycle survive a left or right turn given speed and rubber factors
    gSurviveData canSurviveTurn(gHelperData &data,
                                REAL freeSpaceFactor = 0,
                                bool driveStraight = false);

    // convert uActionPlayer to DIR
    static int ActToTurn(uActionPlayer *action);

    // displays a debugging message
    template <class T>
    void BotDebug(std::string description, T value = "");

    // displays a debugging message without a value
    void BotDebug(std::string description);

    // calculates the distance between two sensors
    REAL Distance(Sensor const &a, Sensor const &b);

    // solve the turn required to escape the situation based on chatbot logic
    // returns a pointer to the gTurnData object that stores the turn data
    gTurnData &getTurn();

    // solve the turn required to escape the situation based on AIBase logic
    // returns a pointer to the gTurnData object that stores the turn data
    gTurnData &getTurnAIBase(){};

private:
    // pointer to the gTurnData object that stores the turn data
    gTurnData &turnData;

    gSmarterBot &smarterBot;
    // the owner of the chatbot
    gCycle &owner_;

    // pointer to the gHelper instance
    gHelper &helper_;
};

#endif