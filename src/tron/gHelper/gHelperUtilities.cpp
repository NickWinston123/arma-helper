#include "gHelperUtilities.h"
#include "gHelperVar.h"
#include "../gSensor.h"
extern void sg_RubberValues( ePlayerNetID const * player, REAL speed, REAL & max, REAL & effectiveness );

using namespace helperConfig;

gCycle* gHelperEnemiesData::detectEnemies() {
    allEnemies.clear();
    closestEnemy = nullptr;
    REAL closestEnemyDistanceSquared = 1212121212;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++) {
        auto other = dynamic_cast<gCycle*>(se_PlayerNetIDs[i]->Object());
        if (other != nullptr && other->Alive() && other->Team() != owner_->Team()) {
            REAL positionDifference = st_GetDifference(other->Position(), owner_->Position());
            if (positionDifference < closestEnemyDistanceSquared) {
                closestEnemyDistanceSquared = positionDifference;
                closestEnemy = other;
            }
            allEnemies.emplace(other);
        }
    }
    return closestEnemy;
}


void gHelperRubberData::calculate() {
    if (!helper_->aliveCheck()) { return; }

    sg_RubberValues( owner_->Player(), owner_->verletSpeed_, rubberAvailable, rubberEffectiveness );
    rubberTimeLeft = ( rubberAvailable - owner_->GetRubber() )*rubberEffectiveness/owner_->verletSpeed_;
    rubberUsedRatio = owner_->GetRubber()/rubberAvailable;
    rubberFactor = owner_->verletSpeed_ * (  owner_->GetTurnDelay() - rubberTimeLeft );//* sg_helperSmartTurningRubberTimeMult );
    //rubberFactor *= rubberFactorMult; For Editing value with a mult
}


bool gHelperEnemiesData::exist(gCycle* enemy) {
    return bool(enemy) && enemy->Alive();
}

REAL gSmartTurningCornerData::getTimeUntilTurn(REAL speed) {
    return distanceFromPlayer /= speed * .98f;
}

bool gSmartTurningCornerData::isInfront(eCoord pos, eCoord dir) {
    return eCoord::F(dir, currentPos - pos) > 0;
}

gHelperSensors::gHelperSensors(gSensor* front_, gSensor* left_, gSensor* right_) :
    front(front_), left(left_), right(right_) {}

gHelperSensorsData::gHelperSensorsData(gCycle *owner) :
    owner_(owner),
    front_stored(new gSensor(owner_, owner_->Position(), owner_->Direction())),
    left_stored(new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(LEFT)))),
    right_stored(new gSensor(owner_, owner_->Position(), owner_->Direction().Turn(eCoord(RIGHT))))
{}

gSensor * gHelperSensorsData::getSensor(int dir, bool newSensor)
{
    return getSensor(owner_->Position(),dir, newSensor);
}

gSensor *gHelperSensorsData::getSensor(eCoord start, eCoord dir)
{
    gSensor * sensor = new gSensor(owner_,start, dir);
    sensor->detect(sg_helperSensorRange);
    return sensor;
}

gSensor *gHelperSensorsData::getSensor(eCoord start, int dir, bool newSensor)
{
    if (sg_helperSensorLightUsageMode && !newSensor)
    {
        switch (dir)
        {

        case LEFT:
        {

            if (sg_helperSensorDiagonalMode)
                left_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(-cos(M_PI/4), sin(M_PI/4))), true);
            else
                left_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(0, 1)), true);

            return left_stored;
        }
        case FRONT:
        {
            front_stored->detect(sg_helperSensorRange, start, owner_->Direction(), true);
            return front_stored;
        }
        case RIGHT:
        {
            if (sg_helperSensorDiagonalMode)
                right_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(-cos(M_PI/4), -sin(M_PI/4))), true);
            else
                right_stored->detect(sg_helperSensorRange, start, owner_->Direction().Turn(eCoord(0, -1)), true);
            return right_stored;
        }
        }
    }
    else
    {
        switch (dir)
        {
        case LEFT:
        {
            gSensor *left = new gSensor(owner_, start, owner_->Direction().Turn(eCoord(0, 1)));
            left->detect(sg_helperSensorRange, true);
            return left;
        }
        case FRONT:
        {
            gSensor *front = new gSensor(owner_, start, owner_->Direction());
            front->detect(sg_helperSensorRange, true);
            return front;
        }
        case RIGHT:
        {
            gSensor *right = new gSensor(owner_, start, owner_->Direction().Turn(eCoord(0, -1)));
            right->detect(sg_helperSensorRange, true);
            return right;
        }
        }
    }
}

gHelperData::gHelperData(gHelperSensorsData *sensors_, gCycle *owner)
    : sensors(*sensors_),
      owner_(owner)
{
    ownerSpeed = &owner_->verletSpeed_;
}

REAL gHelperData::turnSpeedFactorF() {
    return ((*ownerSpeed) * owner_->GetTurnDelay());
}

void gHelperData::Load(REAL a_speedFactor,
                         REAL a_turnSpeedFactor, REAL a_turnSpeedFactorPercent,
                         REAL a_turnDistance, REAL a_thinkAgain, REAL a_turnDir,
                         REAL a_turnTime)
    {
      speedFactor = (a_speedFactor);
      turnSpeedFactor = (a_turnSpeedFactor);
      turnSpeedFactorPercent = (a_turnSpeedFactorPercent);
      turnDistance = (a_turnDistance);
      thinkAgain = (a_thinkAgain);
      turnDir = (a_turnDir);
      turnTime = (a_turnTime);
      }



gHelperData::gHelperData(gHelperSensorsData &sensors_, REAL a_speedFactor,
                         REAL a_turnSpeedFactor, REAL a_turnSpeedFactorPercent,
                         REAL a_turnDistance, REAL a_thinkAgain, REAL a_turnDir,
                         REAL a_turnTime)
    : sensors(sensors_),
      speedFactor(a_speedFactor),
      turnSpeedFactor(a_turnSpeedFactor),
      turnSpeedFactorPercent(a_turnSpeedFactorPercent),
      turnDistance(a_turnDistance),
      thinkAgain(a_thinkAgain),
      turnDir(a_turnDir),
      turnTime(a_turnTime)
{ }

