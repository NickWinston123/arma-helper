#ifndef ArmageTron_HELPER_H
#define ArmageTron_HELPER_H
#include <vector>
#include <algorithm>
#include "../gStuff.h"
//#include "eSound.h"
//#include "rTexture.h"
//#include "rModel.h"
#include "eNetGameObject.h"
#include "tList.h"
#include "nObserver.h"
#include "rDisplayList.h"

#include "ePath.h"
#include "../gCycleMovement.h"
#include "eSensor.h"
#include "eGrid.h"
#include "tString.h"

#include "gHelperVar.h"

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

//! return data of SolveTurn
struct SolveTurnData
{
    REAL turnTime;  //!< seconds to wait before we turn
    REAL quality;   //!< quality of the turn
    eCoord turnDir; //!< direction to drive in

    SolveTurnData(): turnTime(0), quality(0){}
};

class gAIPlayer;
struct gHelperSensors
{
    gSensor* front;
    gSensor* left;
    gSensor* right;

    gHelperSensors(gSensor* front_, gSensor* left_, gSensor* right_);
};



class gHelper {
    friend class gCycle;
    friend class gSmartTurning;
    friend class gSmarterBot;
    friend class gPathHelper;
    friend class gTailHelper;
    friend class gZoneHelper;

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

        void autoBrake();

        void showEnemyTail(gHelperData &data);

        void showTailPath(gHelperData &data);

        void showHitDebugLines(eCoord pos, eCoord dir, REAL timeout, gHelperData &data, int recursion, int sensorDir);
        bool isClose(eCoord pos, REAL closeFactor);

        gCycle *getOwner();
        eCoord closestCorner(eCoord center, REAL radius);
        static void debugLine(gRealColor color, REAL height, REAL timeout,
                        eCoord start,eCoord end, REAL brightness = 1);

        static void debugBox(REAL R, REAL G, REAL B, eCoord center, REAL radius, REAL timeout);
        ~gHelper();
    private:
        gCycle *owner_;
        ePlayerNetID *player_;
        gAIPlayer *aiPlayer;
        bool aiCreated;

        gCycle *closestEnemy;
        eCoord *ownerPos;
        eCoord *ownerDir;
        eCoord *tailPos;
        REAL *ownerSpeed;
        REAL ownerWallLength;
        REAL ownerTurnDelay;

        gHelperSensorsData * sensors_;
        gHelperData *data_stored;
        gSmartTurningCornerData leftCorner, rightCorner;//, frontCorner;
        gSmartTurningCornerData lastLeftCorner, lastRightCorner;//, frontCorner;
        gHelperEnemiesData enemies;
    public:
        gHelperRubberData * rubberData;
    private:
        std::unique_ptr< gSmartTurning > smartTurning;
        std::unique_ptr< gPathHelper > pathHelper;
        std::unique_ptr< gTailHelper > tailHelper;
        std::unique_ptr< gZoneHelper > zoneHelper;
};
#endif
