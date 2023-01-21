#ifndef ArmageTron_HELPER_H
#define ArmageTron_HELPER_H
#include <vector>
#include <algorithm>
#include "gStuff.h"
//#include "eSound.h"
//#include "rTexture.h"
//#include "rModel.h"
#include "eNetGameObject.h"
#include "tList.h"
#include "nObserver.h"
#include "rDisplayList.h"

#include "ePath.h"
#include "gCycleMovement.h"
#include "eSensor.h"
#include "eGrid.h"

// class HelperDebug
// {
// public:
//     template<typename T>
//     static void Debug(const std::string &sender, const std::string &description, T value, bool spamProtection = true)
//     {
//         if (!sg_helperDebug)
//         {
//             return;
//         }

//         bool lastMessageIsSame = (lastHelperDebugMessage == description && lastHelperDebugSender == sender);
//         float currentTime = tSysTimeFloat();
//         bool delayNotPassed = (currentTime - lastHelperDebugMessageTimeStamp) < sg_helperDebugDelay;

//         if (spamProtection && (lastMessageIsSame && delayNotPassed))
//         {
//             return;
//         }

//         lastHelperDebugMessageTimeStamp = currentTime;
//         lastHelperDebugSender = sender;

//         std::string debugMessage;

//         if (sg_helperDebugTimeStamp)
//         {
//             debugMessage += "[" + std::to_string(tSysTimeFloat()) + "] ";
//         }

//         debugMessage += "0xff8888HELPER DEBUG - 0xff5555" + sender + ": 0xffff88:" + description + " ";

//         if (spamProtection)
//         {
//             lastHelperDebugMessage = description;
//         }

//         debugMessage += (value) + "\n";
//         con << debugMessage;
//     }

// private:
//     static std::string lastHelperDebugMessage;
//     static std::string lastHelperDebugSender;
//     static float lastHelperDebugMessageTimeStamp;
// };

// std::string HelperDebug::lastHelperDebugMessage = "";
// std::string HelperDebug::lastHelperDebugSender = "";
// float HelperDebug::lastHelperDebugMessageTimeStamp = 0;

#ifndef G_HELPER_HUD_PUB_ITEMS_H
#define G_HELPER_HUD_PUB_ITEMS_H
#include "gHud.h"
class gHelperHudPubItems
{
public:
    static std::vector<gHelperHudPubItems> GetHudItems();
    static void InsetHudItem(tColoredString &message);

    REAL locX;
    REAL locY;
    REAL size;
    tColoredString message;

private:
    static std::vector<gHelperHudPubItems> hudItems_;
};
#endif

#ifndef ArmageTron_GHELPER_HUD
#define ArmageTron_GHELPER_HUD
class gHelperHudPub
{
public:
    // Returns the single instance of the class
    static gHelperHudPub& Instance()
    {
        static gHelperHudPub instance;
        return instance;
    }

    // Public methods
    void Activate(gTextCache & cache);

private:
    // Private constructor to prevent multiple instances
    gHelperHudPub() {}

    // Prevent copying and assignment
    gHelperHudPub(gHelperHudPub const&) = delete;
    void operator=(gHelperHudPub const&) = delete;
};



#endif
// class gHelperHudPubItems
// {

//     static gHelperHudPubItems GetHudItems() {
//         return list of hud items
//     }

//     static gHelperHudPubItems InsetHudItem(locx, locy, size, message) {
//         insert hud item
//     }
//     static REAL locX;
//     static REAL locY
//     static REAL size;
//     static tColoredString message;
// }
// class gHelperHudPub
// {
//     public:
//     static void Activate(gGLMeter * meter){

//         gHelperHudPubItems = gHelperHudPubItems::GetHudItems();

//         for ( pubItem. all gHelperHudPubItems)
//         {
//         int length = pubItem.message.Len();
//         rTextField hudDebug(pubItem.locX-((.15*pubItem.size*(pubItem.locX-1.5))/2.0),pubItem.locY,.15*pubItem.size,.3*pubItem.size);
//         hudDebug <<  message;
//         }
//     }
// };

extern bool sg_helperCurrentTimeLocal;
//! return data of SolveTurn
struct SolveTurnData
{
    REAL turnTime;  //!< seconds to wait before we turn
    REAL quality;   //!< quality of the turn
    eCoord turnDir; //!< direction to drive in

    SolveTurnData(): turnTime(0), quality(0){}
};

class gAIPlayer;
void helperMenu();
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
    gHelperSensorsData &sensors;
    REAL &speedFactor;
    REAL &turnSpeedFactor;
    REAL &turnSpeedFactorPercent;
    REAL &turnDistance;
    REAL thinkAgain;
    REAL turnDir;
    REAL turnTime;

    gHelperData(gHelperSensorsData &sensors_, REAL &a_speedFactor, REAL &a_turnSpeedFactor,
                REAL &a_turnSpeedFactorPercent, REAL &a_turnDistance, REAL a_thinkAgain,
                REAL a_turnDir, REAL a_turnTime);
};

class gTailHelper
{
    public:
        // Constructor
        gTailHelper(gHelper* helper, gCycle *owner);

        void Activate(gHelperData &data);
        std::vector<eCoord> getPathToTail(double delay);
        static gTailHelper &Get(gHelper* helper, gCycle *owner);

    private:
        gCycle *owner_; // Pointer to the owner cycle
        gHelper *helper_;
        eCoord *ownerPos;
        eCoord *ownerDir;
        eCoord *tailPos;
        REAL *ownerSpeed;
};

class gPathHelper
{
public:
    // Constructor
    gPathHelper(gHelper* helper, gCycle *owner);

    bool targetExist();

    void FindPath(gHelperData &data);

    void Activate(gHelperData &data);

    bool autoMode(gHelperData orig_data);
    bool enemyMode(gHelperData orig_data);
    bool tailMode(gHelperData orig_data);
    bool cornerMode(gHelperData orig_data);
    bool DistanceCheck(gHelperData & data);
    bool UpdateTimeCheck(gHelperData &data);
    void RenderPath(gHelperData &data);
    void RenderTurn(gHelperData &data);

    static gPathHelper &Get(gHelper* helper, gCycle *owner);

private:

    eCoord target;
    tJUST_CONTROLLED_PTR<eFace> targetCurrentFace_;
    gCycle *owner_; // Pointer to the owner cycle
    gHelper *helper_;
    REAL lastTime;
    REAL nextTime;
    REAL lastPath;
    eCoord lastPos;
    ePath path_;
    bool pathInvalid;
};

struct gTurnData{
    public:
        int direction;
        int numberOfTurns;
    gTurnData   (int direction_, int numberOfTurns_):
    direction(direction_),
    numberOfTurns(numberOfTurns_) { }
};

class gSmartTurning
{
public:
    gSmartTurning(gHelper *helper, gCycle *owner);
    //void findTurnToTarget(eCoord target);
    //void SolveTurn( int direction, eCoord const & targetVelocity, eCoord const & targetPosition, SolveTurnData & data );
    void findTurnToTarget( eCoord const & target, eCoord const & velocity );
    void Activate(gHelperData &data);
    bool isClose(eCoord pos, REAL closeFactor);
    void smartTurningPlan(gHelperData &data);
    void smartTurningSurvive(gHelperData &data);
    void smartTurningOpposite(gHelperData &data);
    void autoUnBrake();
    void followTail(gHelperData &data);
    void smartTurningFrontBot(gHelperData &data);
    void thinkSurvive(gHelperData &data);
    int thinkPath( eCoord pos, gHelperData &data );
    bool canSurviveTurnSpecific(gHelperData &data, int dir, REAL spaceFactor = 0);
    void smartTurningSurviveTrace(gHelperData &data);
    void calculateRubberFactor(REAL rubberMult, REAL rubberFactorMult);
    bool makeTurnIfPossible(gHelperData &data, int dir, REAL spaceFactor = 0);
    void canSurviveTurn(gHelperData &data,
                        REAL &canSurviveLeftTurn,
                        REAL &canSurviveRightTurn,
                        bool &closedIn,
                        bool &blockedBySelf,
                        REAL freeSpaceFactor = 0 );
    static gSmartTurning & Get( gHelper * helper, gCycle *owner);
    gTurnData* getEmergencyTurn(gHelperData &data);
    bool CanMakeTurn(uActionPlayer *action);
    private:
        gCycle *owner_;
        gHelper *helper_;
        gHelperData *data_;
        REAL &lastTurnAttemptTime;
        REAL &lastTurnAttemptDir;
        REAL &lastTurnTime;
        REAL &turnIgnoreTime;
        REAL &lastTurnDir; // -1 left , 1 right
        REAL &blockTurn; // 0 = NONE, -1 = LEFT, 1 = RIGHT, 2 = BOTH
        REAL &forceTurn; // 0 = NONE, -1 = LEFT, 1 = RIGHT
        REAL rubberFactor;
        REAL rubberTime;
        REAL rubberRatio;
        ePath path;
    };

/*
struct gHelperEnemiesData;
{
    gCycle* owner_;
    std::vector<gCycle*> allEnemies;
    gCycle* closestEnemy;

    bool exist(gCycle* enemy);
    gCycle* detectEnemies();
};
*/

#include <unordered_set>

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

struct gHelperData;

class gHelper {
    friend class gCycle;

    public:
    static gHelper & Get( gCycle * cycle );
    gHelper(gCycle *owner);
    void Activate();
    REAL CurrentTime();

    void detectCut(gHelperData &data, int detectionRange);
    void enemyTracers(gHelperData &data, int detectionRange, REAL timeout);
    void showTail(gHelperData &data);
    void showHit(gHelperData &data);

    bool aliveCheck();
    bool drivingStraight();
    void followTail();
    bool canSeeTarget(eCoord target,REAL passthrough);

    bool findCorner(gHelperData &data, gSmartTurningCornerData &corner, const gSensor *sensor);
    void findCorners(gHelperData &data);
    void showCorner(gHelperData &data, gSmartTurningCornerData &corner, REAL timeout);
    void showCorners(gHelperData &data);
    void showTailTracer(gHelperData &data);

    void showEnemyTail(gHelperData &data);

    void showTailPath(gHelperData &data);

    void showHitDebugLines(eCoord pos, eCoord dir, REAL timeout, gHelperData &data, int recursion, int sensorDir);
    bool isClose(eCoord pos, REAL closeFactor);
    template <class T>
    void HelperDebug(const char *from, const char *description, T value, bool spamProtection = true)
    {/*
        if (!sg_helperDebug)
        {
            return;
        }
        bool lastMessageIsSame = lastHelperDebugMessage == description;
        bool delayNotPassed = false;owner_->localCurrentTime - lastHelperDebugMessageTimeStamp > sg_helperDebugDelay;

        if (spamProtection && (lastMessageIsSame && delayNotPassed || delayNotPassed))
        {
            return;
        }

        lastHelperDebugMessageTimeStamp = owner_->localCurrentTime;

        tString debugMessage;
        if (sg_helperDebugTimeStamp)
        {
            debugMessage << "(" << owner_->localCurrentTime << ") ";
        }

        debugMessage << "0xff8888HELPER DEBUG - "
                     << "0xff5555" << from << "0xffff88: " << description << " ";

        if (spamProtection)
        {
            lastHelperDebugMessage = description;
        }
        debugMessage << value << "\n ";
        con << debugMessage;*/
    }

    template <class T>
    void renderHud(T value, const char *t);

    //static gHelper& Get( gCycle * cycle );
    gCycle *getOwner();
    eCoord closestCorner(eCoord center, REAL radius);
    static void debugLine(REAL R, REAL G, REAL B, REAL height, REAL timeout,
                   eCoord start,eCoord end, REAL brightness = 1);
    ~gHelper();
    private:
    gAIPlayer *aiPlayer;
    bool aiCreated;
    friend class gSmartTurning;
    friend class gSmarterBot;
    friend class gPathHelper;
    friend class gTailHelper;
    const char * lastHelperDebugMessage;
    REAL lastHelperDebugMessageTimeStamp;
    gSmartTurningCornerData leftCorner, rightCorner;//, frontCorner;
    gSmartTurningCornerData lastLeftCorner, lastRightCorner;//, frontCorner;
    gHelperEnemiesData enemies;
    gCycle *owner_;
    ePlayerNetID *player_;
    gCycle *closestEnemy;
    eCoord *ownerPos;
    eCoord *ownerDir;
    eCoord *tailPos;
    REAL *ownerSpeed;
    REAL ownerWallLength;
    REAL ownerTurnDelay;
    gHelperSensorsData * sensors_;
    std::unique_ptr< gSmartTurning > smartTurning;
    std::unique_ptr< gPathHelper > pathHelper;
    std::unique_ptr< gTailHelper > tailHelper;
    //std::unique_ptr< gSmarterBot > smarterBot;
    gHelperData *data_stored;
};

#endif
