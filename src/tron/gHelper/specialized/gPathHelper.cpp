#include "../../gCycle.h"
#include "../../gWall.h"
#include "../../gSensor.h"
#include "gHelperSensor.h"
#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "../gHelperHud.h"
#include "gPathHelper.h"

using namespace helperConfig;

bool helperConfig::sg_pathHelper = false;
static tConfItem<bool> sg_pathHelperC = HelperCommand::tConfItem("HELPER_SELF_PATH", helperConfig::sg_pathHelper);
bool helperConfig::sg_pathHelperRenderPath = false;
static tConfItem<bool> sg_pathHelperRenderPathC = HelperCommand::tConfItem("HELPER_SELF_PATH_RENDER", helperConfig::sg_pathHelperRenderPath);
bool helperConfig::sg_pathHelperShowTurn = false;
static tConfItem<bool> sg_pathHelperShowTurnC = HelperCommand::tConfItem("HELPER_SELF_PATH_RENDER_TURN", helperConfig::sg_pathHelperShowTurn);
bool helperConfig::sg_pathHelperShowTurnAct = false;
static tConfItem<bool> sg_pathHelperShowTurnActC = HelperCommand::tConfItem("HELPER_SELF_PATH_RENDER_TURN_ACT", helperConfig::sg_pathHelperShowTurnAct);
REAL helperConfig::sg_pathHelperShowTurnAhead = 0;
static tConfItem<REAL> sg_pathHelperShowTurnAheadC = HelperCommand::tConfItem("HELPER_SELF_PATH_RENDER_TURN_AHEAD", helperConfig::sg_pathHelperShowTurnAhead);

int helperConfig::sg_pathHelperMode = 0;
std::vector<std::pair<std::string, std::string>> sg_pathHelperModeValueMap = {
    {"0", "AUTO"},
    {"1", "TAIL"},
    {"2", "ENEMY"},
    {"3", "CORNER"}
};
static tConfItem<int> sg_pathHelperModeC = HelperCommand::tConfItem("HELPER_SELF_PATH_MODE", helperConfig::sg_pathHelperMode, sg_pathHelperModeValueMap);
REAL helperConfig::sg_pathHelperAutoCloseDistance = 150;
static tConfItem<REAL> sg_pathHelperAutoCloseDistanceC = HelperCommand::tConfItem("HELPER_SELF_PATH_AUTO_CLOSE_DISTANCE", helperConfig::sg_pathHelperAutoCloseDistance);
REAL helperConfig::se_pathHeight = 1;
static tConfItem<REAL> se_pathHeightC = HelperCommand::tConfItem("HELPER_SELF_PATH_RENDER_HEIGHT", helperConfig::se_pathHeight);
REAL helperConfig::se_pathBrightness = 1;
static tConfItem<REAL> se_pathBrightnessC = HelperCommand::tConfItem("HELPER_SELF_PATH_BRIGHTNESS", helperConfig::se_pathBrightness);
REAL helperConfig::sg_pathHelperUpdateDistance = 1;
static tConfItem<REAL> sg_pathHelperUpdateDistanceC = HelperCommand::tConfItem("HELPER_SELF_PATH_UPDATE_DISTANCE", helperConfig::sg_pathHelperUpdateDistance);

static gHelperHudItem<tColoredString> sg_pathHelperModeH("Mode", tColoredString("None"), "Path Helper");
static gHelperHudItem<REAL> sg_pathHelperDistanceH("Distance", 1000, "Path Helper");

gPathHelper::gPathHelper(gHelper &helper, gCycle &owner)
    : helper_(helper),
      owner_(owner),
      pathUpdatedTime(-999)
{}

bool gPathHelper::targetExist()
{
    return target != eCoord(0, 0) && targetCurrentFace_;
}

bool gPathHelper::cornerMode(gHelperData &data)
{
    helper_.findCorners(data);

    bool left = data.leftCorner.exist;
    bool right = data.rightCorner.exist;
    bool both = left && right;

    eCoord position;
    if (left && !right)
    {
        position = data.leftCorner.currentPos;
    }
    else if (!left && right)
    {
        position = data.rightCorner.currentPos;
    }
    else if (both)
    {
        // Calculate the distance from the owner's position to the left corner
        REAL leftDistance = eCoordDistance(owner_.Position(), data.leftCorner.currentPos);
        // Calculate the distance from the owner's position to the right corner
        REAL rightDistance = eCoordDistance(owner_.Position(), data.rightCorner.currentPos);
        position = (leftDistance < rightDistance) ? data.leftCorner.currentPos : helper_.data_stored.rightCorner.currentPos;
    }

    target = position;

    if (!DistanceCheck(data))
        return false;

    owner_.FindCurrentFace();
    targetCurrentFace_ = owner_.Grid()->FindSurroundingFace(target);

    return true;
}

bool gPathHelper::tailMode(gHelperData &data)
{
    if (!owner_.tailMoving)
        return false;

    target = owner_.tailPos;
    if (!DistanceCheck(data))
        return false;

    owner_.FindCurrentFace();
    targetCurrentFace_ = owner_.Grid()->FindSurroundingFace(owner_.tailPos);
    return true;
}

bool gPathHelper::enemyMode(gHelperData &data)
{
    gCycle *enemy = data.enemies.closestEnemy.owner_;

    if (!data.enemies.exist(enemy))
        return false;

    target = enemy->Position();
    if (!DistanceCheck(data))
        return false;

    owner_.FindCurrentFace();
    targetCurrentFace_ = enemy->CurrentFace();
    return true;
}

bool gPathHelper::autoMode(gHelperData &data)
{
    gCycle *enemy = data.enemies.closestEnemy.owner_;

    bool isClose = data.enemies.exist(enemy) && gHelperUtility::isClose(&owner_, enemy->Position(), sg_pathHelperAutoCloseDistance + data.ownerData.turnSpeedFactorF());
    if (isClose)
    {
        target = enemy->Position();

        if (!DistanceCheck(data))
            return false;

        owner_.FindCurrentFace();
        targetCurrentFace_ = enemy->CurrentFace();
        return true;
    }
    else if (owner_.tailMoving)
    {
        target = owner_.tailPos;

        if (!DistanceCheck(data))
            return false;

        owner_.FindCurrentFace();
        targetCurrentFace_ = owner_.Grid()->FindSurroundingFace(target);
        return true;
    }

    return false;
}

void gPathHelper::RenderPath(gHelperData &data)
{
    if (!path_.Valid() || path_.positions.Len() == 0)
        return;

    eCoord last_c;
    eCoord owner_pos = owner_.pos; // get the current position of the owner

    gHelperUtility::debugLine(tColor(1, 1, 0), 0, data.ownerData.speedFactorF(), owner_pos, path_.positions(0) + path_.offsets(0), se_pathBrightness);

    for (int i = path_.positions.Len() - 1; i >= 0; i--)
    {
        eCoord c = path_.positions(i) + path_.offsets(i);
        if (i != path_.positions.Len() - 1)
            gHelperUtility::debugLine(tColor(1, 0, 0), se_pathHeight, data.ownerData.speedFactorF(), last_c, c, se_pathBrightness);
        last_c = c;
    }

    if (path_.current >= 0 && path_.positions.Len() > 0)
    {
        eCoord c = path_.CurrentPosition();
        gHelperUtility::debugLine(tColor(1, 1, 0), se_pathHeight, data.ownerData.speedFactorF(), c, c, 1);
        gHelperUtility::debugLine(tColor(1, 1, 0), (se_pathHeight * 2), data.ownerData.speedFactorF(), c, c, se_pathBrightness);
    }
}

void gPathHelper::RenderTurn(gHelperData &data)
{
    if (!path_.Valid())
        return;

    for (int z = 10; z >= 0; z--)
        path_.Proceed();

    bool goon = path_.Proceed();
    bool nogood = false;

    do
    {
        if (goon)
            goon = path_.GoBack();
        else
            goon = true;

        eCoord pos = path_.CurrentPosition() + path_.CurrentOffset() * 0.1f;
        eCoord opos = owner_.Position();
        eCoord odir = pos - opos;

        eCoord intermediate = opos + owner_.Direction() * eCoord::F(odir, owner_.Direction());

        std::shared_ptr<gHelperSensor> sensor = std::make_shared<gHelperSensor>(&owner_, opos, intermediate - opos);
        sensor->detect(1.1f);


        nogood = (sensor->hit <= .999999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);
        if (!nogood)
        {
            std::shared_ptr<gHelperSensor> intermediateSensor = std::make_shared<gHelperSensor>(&owner_, intermediate, pos - intermediate);
            intermediateSensor->detect(1);

            nogood = (intermediateSensor->hit <= .99999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);
        }

    } while (goon && nogood);

    if (goon)
    {
        // now we have found our next goal. Try to get there.
        eCoord pos = owner_.Position();
        eCoord target = path_.CurrentPosition();

        // look how far ahead the target is:
        REAL ahead = eCoord::F(target - pos, owner_.Direction()) + eCoord::F(path_.CurrentOffset(), owner_.Direction());

        // gHelperUtility::Debug("RenderTurn", "AHEAD = ", ahead);
        if (ahead > sg_pathHelperShowTurnAhead)
        { // it is still before us. just wait a while.
          // mindist = ahead;
        }
        else
        { // we have passed it. Make a turn towards it.
            int lr;
            REAL side = (target - pos) * owner_.Direction();

            std::shared_ptr<gHelperSensor> left = std::make_shared<gHelperSensor>(&owner_, owner_.Position(), LEFT);
            left->detect(sg_helperSensorRange);

            std::shared_ptr<gHelperSensor> right = std::make_shared<gHelperSensor>(&owner_, owner_.Position(), RIGHT);
            right->detect(sg_helperSensorRange);

            if (!((side > 0 && left->hit < 3) || (side < 0 && right->hit < 3)) && (fabs(side) > 3 || ahead < -10))
            {
                lr += (side > 0 ? 1 : -1);
            }

            // Comes in opposite, flip to fit turn direction mapping set in gHelper
            lr *= -1;

            std::shared_ptr<gHelperSensor> turnDirection = std::make_shared<gHelperSensor>(&owner_, owner_.Position(), lr);
            turnDirection->detect(sg_helperSensorRange);

            gHelperUtility::debugLine(tColor(.2, 1, 0), 3, data.ownerData.speedFactorF() * 3, owner_.Position(), turnDirection->before_hit, 1);
            if (sg_pathHelperShowTurnAct)
                helper_.turnHelper->makeTurnIfPossible(data, lr, 1);
        }
    }
}

REAL gPathHelper::DistanceToTarget()
{
    eCoord difference = target - lastPos;
    return difference.Norm();
}

bool gPathHelper::DistanceCheck(gHelperData &data)
{
    return DistanceToTarget() >= sg_pathHelperUpdateDistance;
}

tColoredString gPathHelper::ModeString()
{
    tColoredString mode;
    switch (sg_pathHelperMode)
    {
    case AUTO:
        mode << "Auto";
        break;
    case TAIL:
        mode << "Tail";
        break;
    case ENEMY:
        mode << "Enemy";
        break;
    case CORNER:
        mode << "Corner";
        break;
    default:
        mode << "Auto";
    }
    return mode;
}

void gPathHelper::FindPath(gHelperData &data)
{
    if (!targetExist())
        return;

    if (targetCurrentFace_)
    {
        eHalfEdge::FindPath(owner_.Position(), owner_.CurrentFace(),
                            target, targetCurrentFace_,
                            &owner_,
                            path_);
        pathUpdatedTime = helper_.CurrentTime();
        lastPos = target;

        if (sg_helperDebug)
        {
            tString debugUpdateStr;

            debugUpdateStr << ("Updated path at '");
            debugUpdateStr << customRound(pathUpdatedTime,2) << "' ";

            tString debugModeStr("Current Mode: ");

            debugModeStr << ModeString();

            gHelperUtility::Debug("FindPath", debugModeStr.stdString(), debugUpdateStr, false);
        }
    }
    else
    {
        gHelperUtility::Debug("FindPath", "Missing targetCurrentFace");
    }

    if (!path_.Valid())
    {
        targetCurrentFace_ = NULL;
        pathUpdatedTime = -999;
    }
}

void gPathHelper::Activate(gHelperData &orig_data)
{

    if (!helper_.aliveCheck())
        return;

    if (sg_pathHelperRenderPath)
        RenderPath(orig_data);

    if (sg_pathHelperShowTurn)
        RenderTurn(orig_data);

    if (sg_helperHud) {
        sg_pathHelperModeH     << ModeString();
        sg_pathHelperDistanceH << DistanceToTarget();
    }

    bool success = false;
    switch (sg_pathHelperMode)
    {
    case AUTO:
        success = autoMode(orig_data);
        break;
    case TAIL:
        success = tailMode(orig_data);
        break;
    case ENEMY:
        success = enemyMode(orig_data);
        break;
    case CORNER:
        success = cornerMode(orig_data);
        break;
    default:
        success = autoMode(orig_data);
    }

    if (!success)
        return;

    FindPath(orig_data);
}

gPathHelper &gPathHelper::Get(gHelper &helper, gCycle &owner)
{
    tASSERT(&owner);

    // create
    if (helper.pathHelper.get() == 0)
        helper.pathHelper.reset(new gPathHelper(helper, owner));

    return *helper.pathHelper;
}
