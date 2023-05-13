#include "gHelperUtilities.h"
#include "gHelperVar.h"
#include "../gSensor.h"
#include "eDebugLine.h"
#include "specialized/gHelperSensor.h"
extern void sg_RubberValues( ePlayerNetID const * player, REAL speed, REAL & max, REAL & effectiveness );

using namespace helperConfig;

void gHelperUtility::debugLine(tColor color, REAL height, REAL timeout,
                               eCoord start, eCoord end, REAL brightness)
{
    if (timeout >= 6)
    {
        timeout = 1;
    }
    REAL startHeight = height;
    if (start == end)
        startHeight = 0;
    eDebugLine::SetTimeout(timeout);
    eDebugLine::SetColor(color.r_ * sg_helperBrightness * brightness, color.g_ * sg_helperBrightness * brightness, color.b_ * sg_helperBrightness * brightness);
    eDebugLine::Draw(start, startHeight, end, height);
}

void gHelperUtility::debugBox(tColor color, eCoord center, REAL radius, REAL timeout, REAL height)
{
    eCoord corner[3];
    corner[0] = eCoord(center.x - (radius), center.y + (radius));
    corner[1] = eCoord(center.x + (radius), center.y + (radius));
    corner[2] = eCoord(center.x + (radius), center.y - (radius));
    corner[3] = eCoord(center.x - (radius), center.y - (radius));

    debugLine(color, height, timeout, corner[0], corner[1]);
    debugLine(color, height, timeout, corner[1], corner[2]);
    debugLine(color, height, timeout, corner[2], corner[3]);
    debugLine(color, height, timeout, corner[3], corner[0]);
}

tColor gHelperUtility::tStringTotColor(tString string)
{
    tArray<tString>rgb = string.Split(",");
    int pos;
    REAL r = atoi(rgb[0]);
    REAL g = atoi(rgb[1]);
    REAL b = atoi(rgb[2]);
    tColor color(r,g,b);
    return color;
}

// Function: isClose
// Check if the distance between a point and the player is within a certain range.
//
// Parameters:
// pos: The position to check distance to.
// closeFactor: The maximum allowed distance to be considered close.
//
// Returns:
// bool: True if the distance is within the closeFactor range, False otherwise.
 bool gHelperUtility::isClose(gCycle *owner_, eCoord pos, REAL closeFactor)
{
    eCoord distanceToPos = owner_->pos - pos;
    return distanceToPos.NormSquared() < closeFactor * closeFactor;
}

 bool gHelperUtility::isClose(gCycle *owner_, gCycle *enemy_, REAL closeFactor)
{
    return isClose(owner_, enemy_->Position(), closeFactor);
}

gHelperSensors::gHelperSensors(std::shared_ptr<gHelperSensor> front_, std::shared_ptr<gHelperSensor> left_, std::shared_ptr<gHelperSensor> right_) :
    front(front_), left(left_), right(right_) {}

gHelperSensorsData::gHelperSensorsData(gCycle *owner) :
    owner_(owner),
    front_stored(std::make_shared<gHelperSensor>(owner_, owner_->Position(), owner_->Direction())),
    left_stored(std::make_shared<gHelperSensor>(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(LEFT)))),
    right_stored(std::make_shared<gHelperSensor>(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(RIGHT))))
{}


std::shared_ptr<gHelperSensor> gHelperSensorsData::getSensor(int dir, bool newSensor) {
    return getSensor(owner_->Position(), dir, newSensor);
}

std::shared_ptr<gHelperSensor> gHelperSensorsData::getSensor(eCoord start, eCoord dir, REAL detectRange)
{
    std::shared_ptr<gHelperSensor> sensor = std::make_shared<gHelperSensor>(owner_, start, dir);
    sensor->detect(detectRange);
    return sensor;
}

std::shared_ptr<gHelperSensor> gHelperSensorsData::getSensor(eCoord start, int dir, bool newSensor) {
    if (sg_helperSensorLightUsageMode && !newSensor)
    {
        int winding = owner_->Grid()->DirectionWinding(owner_->Direction());

        switch (dir)
        {
            case LEFT:
            {
                int left_winding = winding - 1;
                eCoord left_dir = owner_->Grid()->GetDirection(left_winding);

                if (sg_helperSensorDiagonalMode)
                    left_stored->detect(sg_helperSensorRange, start, left_dir, true);
                else
                    left_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(0, 1)), true);

                return left_stored;
            }
            case FRONT:
            {
                eCoord front_dir = owner_->Grid()->GetDirection(winding);

                front_stored->detect(sg_helperSensorRange, start, front_dir, true);
                return front_stored;
            }
            case RIGHT:
            {
                int right_winding = winding + 1;
                eCoord right_dir = owner_->Grid()->GetDirection(right_winding);

                if (sg_helperSensorDiagonalMode)
                    right_stored->detect(sg_helperSensorRange, start, right_dir, true);
                else
                    right_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(0, -1)), true);
                return right_stored;
            }
        }
    }
    else
    {
        int winding = owner_->Grid()->DirectionWinding(owner_->Direction());

        std::shared_ptr<gHelperSensor> sensor;
        switch (dir)
        {
            case LEFT:
            {
                int left_winding = winding - 1;
                eCoord left_dir = owner_->Grid()->GetDirection(left_winding);

                sensor = std::make_shared<gHelperSensor>(owner_, start, left_dir);
                sensor->detect(sg_helperSensorRange, true);
                return sensor;
            }
            case FRONT:
            {
                eCoord front_dir = owner_->Grid()->GetDirection(winding);

                sensor = std::make_shared<gHelperSensor>(owner_, start, front_dir);
                sensor->detect(sg_helperSensorRange, true);
                return sensor;
            }
            case RIGHT:
            {
                int right_winding = winding + 1;
                eCoord right_dir = owner_->Grid()->GetDirection(right_winding);

                sensor = std::make_shared<gHelperSensor>(owner_, start, right_dir);
                sensor->detect(sg_helperSensorRange, true);
                return sensor;
            }
        }
    }
    return nullptr;
}

REAL gHelperOwnerData::turnSpeedFactorF() {
    return (((owner_->verletSpeed_) * owner_->GetTurnDelay()));
}

REAL gHelperOwnerData::turnTimeF(int dir) {
    return owner_->GetNextTurn(dir);

}

REAL gHelperOwnerData::lagFactorF() {
    return (owner_->Lag());
}

REAL gHelperOwnerData::speedFactorF() {
    return (1/(owner_->verletSpeed_));
}


bool gHelperClosestEnemyData::faster(gCycle *helperOwner) {
    return  owner_->Speed() > helperOwner->Speed();
}

bool gHelperEnemiesData::exist(gCycle* enemy) {
    return (enemy != nullptr) && enemy->Alive();
}

gCycle * gHelperEnemiesData::getClosestEnemy(gCycle *owner_, bool ignoreLocal, bool ignoreOwner) {
    gCycle * closestEnemy = nullptr;
    REAL closestEnemyDistanceSquared = 999999999;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        auto other = dynamic_cast<gCycle*>(se_PlayerNetIDs[i]->Object());
        if (other != nullptr && other->Alive() && se_PlayerNetIDs[i]->pID != owner_->Player()->pID) {
            bool isOwner = se_PlayerNetIDs[i]->pID == 0;
            bool isLocal = !isOwner && ePlayer::NetToLocalPlayer(se_PlayerNetIDs[i]) != nullptr;

            if (ignoreLocal && isLocal || ignoreOwner && isOwner)
                continue;

            REAL positionDifference = st_GetDifference(other->Position(), owner_->Position());
            if (positionDifference < closestEnemyDistanceSquared) {
                closestEnemyDistanceSquared = positionDifference;
                closestEnemy = other;
            }

        }
    }
    return closestEnemy;
}


gCycle* gHelperEnemiesData::detectEnemies() {
    allEnemies.clear();
    closestEnemy.owner_ = nullptr;
    REAL closestEnemyDistanceSquared = 999999999;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        auto other = dynamic_cast<gCycle*>(se_PlayerNetIDs[i]->Object());
        if (other != nullptr && other->Alive() && se_PlayerNetIDs[i]->pID != owner_->Player()->pID) {
            REAL positionDifference = st_GetDifference(other->Position(), owner_->Position());
            if (positionDifference < closestEnemyDistanceSquared) {
                closestEnemyDistanceSquared = positionDifference;
                closestEnemy.owner_ = other;
            }
            allEnemies.insert(other);
        }
    }
    return closestEnemy.owner_;
}


REAL gSmartTurningCornerData::getTimeUntilTurn(REAL speed) {
    return distanceFromPlayer /= speed * .98f;
}

bool gSmartTurningCornerData::isInfront(eCoord pos, eCoord dir) {
    return eCoord::F(dir, currentPos - pos) > 0;
}

bool gSmartTurningCornerData::isInfront(eCoord pos, eCoord dir, eCoord posToCheck) {
    return eCoord::F(dir, posToCheck - pos) > 0;
}

bool gSmartTurningCornerData::findCorner(const std::shared_ptr<gHelperSensor>& sensor, gHelper &helper)
{
    if (!sensor->ehit)
    {
        exist = false;
        return false;
    }
    lastCorner = this;

    noticedTime = helper.CurrentTime();
    exist = true;

    type = sensor->type;
    currentPos = *sensor->ehit->Point();
    distanceFromPlayer = helper.ownerDir.Dot(currentPos - helper.ownerPos);
    turnTime = getTimeUntilTurn(helper.ownerSpeed + helper.CurrentTime());
    REAL secondEdgeDistance = helper.ownerDir.Dot(*sensor->ehit->Other()->Point() - helper.ownerPos);

    infront = isInfront(helper.ownerPos, helper.ownerDir);

    if (distanceFromPlayer < secondEdgeDistance || !infront)
    {
        // other Corner Infront
        if (isInfront(helper.ownerPos, helper.ownerDir, *sensor->ehit->Other()->Point())) {
            distanceFromPlayer = secondEdgeDistance;
            currentPos = *sensor->ehit->Other()->Point();
            infront = true;
        }
    }

    if (lastPos != currentPos)
    {
        lastPos = currentPos;
        ignoredTime = helper.CurrentTime();
    }
    return true;
}


void gHelperRubberData::calculate() {
    if (!helper_->aliveCheck()) { return; }

    sg_RubberValues( owner_->Player(), owner_->verletSpeed_, rubberAvailable, rubberEffectiveness );
    rubberTimeLeft = ( rubberAvailable - owner_->GetRubber() )*rubberEffectiveness/owner_->verletSpeed_;
    rubberUsedRatio = owner_->GetRubber()/rubberAvailable;
    rubberFactor = owner_->verletSpeed_ * (  owner_->GetTurnDelay() - rubberTimeLeft );//* sg_helperSmartTurningRubberTimeMult );
    //rubberFactor *= rubberFactorMult; For Editing value with a mult
}


