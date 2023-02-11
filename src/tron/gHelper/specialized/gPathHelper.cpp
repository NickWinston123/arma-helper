#include "../../gCycle.h"
#include "../../gWall.h"
#include "../../gSensor.h"
#include "gHelperSensor.h"
#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "gPathHelper.h"

using namespace helperConfig;

gPathHelper::gPathHelper(gHelper *helper, gCycle *owner)
    : helper_(helper),
      owner_(owner),
      pathUpdatedTime(helper_->CurrentTime() - 100),
      pathUpdateTime(0)
{
    // Initialize any other member variables here
}

bool gPathHelper::targetExist()
{
    return target != eCoord(0, 0) && targetCurrentFace_;
}

bool gPathHelper::cornerMode(gHelperData data)
{
    helper_->findCorners(data);

    bool left = helper_->leftCorner.exist;
    bool right = helper_->rightCorner.exist;
    bool both = left && right;

    eCoord position;
    if (left && !right)
    {
        position = helper_->leftCorner.currentPos;
    }
    else if (!left && right)
    {
        position = helper_->rightCorner.currentPos;
    }
    else if (both)
    {
        // Calculate the distance from the owner's position to the left corner
        REAL leftDistance = eCoordDistance(owner_->Position(), helper_->leftCorner.currentPos);
        // Calculate the distance from the owner's position to the right corner
        REAL rightDistance = eCoordDistance(owner_->Position(), helper_->rightCorner.currentPos);
        position = (leftDistance < rightDistance) ? helper_->leftCorner.currentPos : helper_->rightCorner.currentPos;
    }

    target = position;

    if (!DistanceCheck(data))
        return false;

    owner_->FindCurrentFace();
    targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(target);

    return true;
}

bool gPathHelper::tailMode(gHelperData data)
{
    if (!owner_->tailMoving)
        return false;

    target = owner_->tailPos;
    if (!DistanceCheck(data))
        return false;

    owner_->FindCurrentFace();
    targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(owner_->tailPos);
    return true;
}

bool gPathHelper::enemyMode(gHelperData data)
{
    gCycle *enemy = helper_->enemies.closestEnemy;

    if (!helper_->enemies.exist(enemy))
        return false;

    target = enemy->Position();
    if (!DistanceCheck(data))
        return false;

    owner_->FindCurrentFace();
    targetCurrentFace_ = enemy->CurrentFace();
    return true;
}

bool gPathHelper::autoMode(gHelperData data)
{
    gCycle *enemy = helper_->enemies.closestEnemy;

    bool isClose = helper_->enemies.exist(enemy) && gHelperUtility::isClose(owner_, enemy->Position(), sg_pathHelperAutoCloseDistance + data.turnSpeedFactorF());
    if (isClose)
    {
        target = enemy->Position();

        if (!DistanceCheck(data))
            return false;

        owner_->FindCurrentFace();
        targetCurrentFace_ = enemy->CurrentFace();
        return true;
    }
    else if (owner_->tailMoving)
    {
        target = owner_->tailPos;

        if (!DistanceCheck(data))
            return false;

        owner_->FindCurrentFace();
        targetCurrentFace_ = owner_->Grid()->FindSurroundingFace(target);
        return true;
    }

    return false;
}

bool gPathHelper::UpdateTimeCheck(gHelperData &data)
{
    return pathUpdatedTime < pathUpdateTime;
}

void gPathHelper::RenderPath(gHelperData &data)
{
    if (!path_.Valid() || path_.positions.Len() == 0)
        return;

    eCoord last_c;
    eCoord owner_pos = owner_->pos; // get the current position of the owner

    gHelperUtility::debugLine(gRealColor(1, 1, 0), 0, data.speedFactorF(), owner_pos, path_.positions(0) + path_.offsets(0), se_pathBrightness);

    for (int i = path_.positions.Len() - 1; i >= 0; i--)
    {
        eCoord c = path_.positions(i) + path_.offsets(i);
        if (i != path_.positions.Len() - 1)
            gHelperUtility::debugLine(gRealColor(1, 0, 0), se_pathHeight, data.speedFactorF(), last_c, c, se_pathBrightness);
        last_c = c;
    }

    if (path_.current >= 0 && path_.positions.Len() > 0)
    {
        eCoord c = path_.CurrentPosition();
        gHelperUtility::debugLine(gRealColor(1, 1, 0), se_pathHeight, data.speedFactorF(), c, c, 1);
        gHelperUtility::debugLine(gRealColor(1, 1, 0), (se_pathHeight * 2), data.speedFactorF(), c, c, se_pathBrightness);
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
        eCoord opos = owner_->Position();
        eCoord odir = pos - opos;

        eCoord intermediate = opos + owner_->Direction() * eCoord::F(odir, owner_->Direction());

        // assigns a hit pointer to the memory location of the hit
        REAL *hit = &data.sensors.getSensor(opos,intermediate - opos,1.1f)->hit;
        nogood = (*hit <= .999999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);

        if (!nogood)
        {
            REAL *hit = &data.sensors.getSensor(intermediate, pos - intermediate, 1)->hit;
            nogood = (*hit <= .99999999 || eCoord::F(path_.CurrentOffset(), odir) < 0);
        }

    } while (goon && nogood);

    if (goon)
    {
        // now we have found our next goal. Try to get there.
        eCoord pos = owner_->Position();
        eCoord target = path_.CurrentPosition();

        // look how far ahead the target is:
        REAL ahead = eCoord::F(target - pos, owner_->Direction()) + eCoord::F(path_.CurrentOffset(), owner_->Direction());

        gHelperUtility::Debug("RenderTurn", "AHEAD = ", &ahead);
        if (ahead > sg_pathHelperShowTurnAhead)
        { // it is still before us. just wait a while.
          // mindist = ahead;
        }
        else
        { // we have passed it. Make a turn towards it.
            int lr;
            REAL side = (target - pos) * owner_->Direction();

            if (!((side > 0 && data.sensors.getSensor(LEFT)->hit < 3) || (side < 0 && data.sensors.getSensor(RIGHT)->hit < 3)) && (fabs(side) > 3 || ahead < -10))
            {
                lr += (side > 0 ? 1 : -1);
            }

            // Comes in opposite, flip to fit turn direction mapping set in gHelper
            lr *= -1; 

            gHelperUtility::debugLine(gRealColor(.2, 1, 0), 3, data.speedFactorF() * 3, owner_->Position(), data.sensors.getSensor(lr)->before_hit, 1);
            if (sg_pathHelperShowTurnAct)
                helper_->turnHelper->makeTurnIfPossible(data, lr, 1);
        }
    }
}

bool gPathHelper::DistanceCheck(gHelperData &data)
{
    eCoord difference = target - lastPos;
    REAL distance = difference.Norm();
    // con << distance << " > " << sg_pathHelperUpdateDistance << " \n";
    return distance >= sg_pathHelperUpdateDistance;
}

void gPathHelper::FindPath(gHelperData &data)
{
    if (!targetExist())
        return;

    if (targetCurrentFace_)
    {
        eHalfEdge::FindPath(owner_->Position(), owner_->CurrentFace(),
                            target, targetCurrentFace_,
                            owner_,
                            path_);
        pathUpdatedTime = helper_->CurrentTime();
        pathUpdateTime = pathUpdatedTime + sg_pathHelperUpdateTime;
        lastPos = target;
        gHelperUtility::Debug("FindPath", "Updated path", "");
    }

    if (!path_.Valid())
    {
        targetCurrentFace_ = NULL;
        pathUpdatedTime = -100;
    }
}

void gPathHelper::Activate(gHelperData &orig_data)
{

    if (!helper_->aliveCheck())
        return;

    if (sg_pathHelperRenderPath)
        RenderPath(orig_data);

    if (sg_pathHelperShowTurn)
        RenderTurn(orig_data);

    if (!UpdateTimeCheck(orig_data))
        return;

    // Copy of data to limit global changes to helper data
    //gHelperData data = orig_data;

    bool success = false;
    switch (sg_pathHelperMode)
    {
    case 0:
        success = autoMode(orig_data);
        break;
    case 1:
        success = tailMode(orig_data);
        break;
    case 2:
        success = enemyMode(orig_data);
        break;
    case 3:
        success = cornerMode(orig_data);
        break;
    default:
        // do nothing
        return;
    }

    if (!success)
        return;

    FindPath(orig_data);
}

gPathHelper &gPathHelper::Get(gHelper *helper, gCycle *owner)
{
    tASSERT(owner);

    // create
    if (helper->pathHelper.get() == 0)
        helper->pathHelper.reset(new gPathHelper(helper, owner));

    return *helper->pathHelper;
}
