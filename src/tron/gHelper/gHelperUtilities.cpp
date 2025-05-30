#include "gHelperUtilities.h"
#include "gHelperVar.h"
#include "../gSensor.h"
#include "eDebugLine.h"
#include "specialized/gHelperSensor.h"
#include "tDirectories.h"

using namespace helperConfig;

// Bright Red for headers
tString sg_helperDebugColorHeader("0xff0033");
static tConfItem<tString> sg_helperDebugColorHeaderConf = HelperCommand::tConfItem("HELPER_DEBUG_COLOR_HEADER", sg_helperDebugColorHeader);
// White for main
tString sg_helperDebugColorMain("0xffffff");
static tConfItem<tString> sg_helperDebugColorMainConf = HelperCommand::tConfItem("HELPER_DEBUG_COLOR_MAIN", sg_helperDebugColorMain);
// Dark Red for items
tString sg_helperDebugColorItem("0xee0000");
static tConfItem<tString> sg_helperDebugColorItemConf = HelperCommand::tConfItem("HELPER_DEBUG_COLOR_ITEM", sg_helperDebugColorItem);
// Pinkish red for error messages as an accent color
tString sg_helperDebugColorError("0xee5577");
static tConfItem<tString> sg_helperDebugColorErrorConf = HelperCommand::tConfItem("HELPER_DEBUG_COLOR_ERROR", sg_helperDebugColorError);

tThemedText gDebugLogger::theme(sg_helperDebugColorHeader, sg_helperDebugColorMain, sg_helperDebugColorItem, sg_helperDebugColorError);

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

    eDebugLine::SetColor((color.r_ * sg_helperBrightness * brightness),  // R
                         (color.g_ * sg_helperBrightness * brightness),  // G
                         (color.b_ * sg_helperBrightness * brightness)); // B

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
    tArray<tString> rgb = string.Split(",");
    int pos;
    REAL r = atoi(rgb[0]);
    REAL g = atoi(rgb[1]);
    REAL b = atoi(rgb[2]);
    tColor color(r, g, b);
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
{
}

std::shared_ptr<gHelperSensor> gHelperSensorsData::getSensor(int dir, bool newSensor)
{
    return getSensor(owner_->Position(), dir, newSensor);
}

std::shared_ptr<gHelperSensor> gHelperSensorsData::getSensor(eCoord start, eCoord dir, REAL detectRange)
{
    std::shared_ptr<gHelperSensor> sensor = std::make_shared<gHelperSensor>(owner_, start, dir);
    sensor->detect(detectRange);
    return sensor;
}

eCoord gHelperSensorsData::extractDirection(eGameObject *owner, int dir)
{
    eCoord direction;

    if (!owner)
        return direction;

    return owner->Grid()->GetDirection(owner->Grid()->DirectionWinding(owner->Direction()) + dir);
}

eCoord gHelperSensorsData::extractDirection(int dir)
{
    return extractDirection(owner_, dir);
}

std::shared_ptr<gHelperSensor> gHelperSensorsData::getSensor(eCoord start, int dir, bool newSensor)
{
    int winding = owner_->Grid()->DirectionWinding(owner_->Direction());

    auto createAndDetectSensor = [&](eCoord realDirection)
    {
        std::shared_ptr<gHelperSensor> sensor = std::make_shared<gHelperSensor>(owner_, start, realDirection);
        sensor->detect(sg_helperSensorRange, true);
        return sensor;
    };

    eCoord direction = extractDirection(dir);
    if (sg_helperSensorLightUsageMode && !newSensor)
    {
        switch (dir)
        {
        case LEFT:
        {
            left_stored->detect(sg_helperSensorRange, start, direction, true);
            return left_stored;
        }
        case FRONT:
        {
            front_stored->detect(sg_helperSensorRange, start, direction, true);
            return front_stored;
        }
        case RIGHT:
        {
            right_stored->detect(sg_helperSensorRange, start, direction, true);
            return right_stored;
        }
        }
    }
    else
    {
        switch (dir)
        {
        case LEFT:
            return createAndDetectSensor(direction);
        case FRONT:
            return createAndDetectSensor(direction);
        case RIGHT:
            return createAndDetectSensor(direction);
        }
    }
    return nullptr;
}

REAL gHelperOwnerData::turnSpeedFactorF()
{
    return (((owner_->verletSpeed_) * owner_->GetTurnDelay()));
}

REAL gHelperOwnerData::turnTimeF(int dir)
{
    return owner_->GetNextTurn(dir);
}

REAL gHelperOwnerData::lagFactorF()
{
    return (owner_->Lag());
}

REAL gHelperOwnerData::speedFactorF()
{
    return (1 / (owner_->verletSpeed_));
}

bool gHelperClosestEnemyData::faster(gCycle *helperOwner)
{
    return owner_->Speed() > helperOwner->Speed();
}

bool gHelperEnemiesData::exist(gCycle *enemy)
{
    return (enemy != nullptr) && enemy->Alive();
}

gCycle *gHelperEnemiesData::getClosestEnemy(gCycle *owner_, bool ignoreLocal, bool ignoreOwner, tString ignoreList)
{
    gCycle *closestEnemy = nullptr;
    REAL closestEnemyDistanceSquared = 999999999;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++)
    {
        auto other = dynamic_cast<gCycle *>(se_PlayerNetIDs[i]->Object());
        if (other != nullptr && other->Alive() && se_PlayerNetIDs[i]->pID != owner_->Player()->pID)
        {
            bool isOwner = se_PlayerNetIDs[i]->pID == 0;
            bool isLocal = !isOwner && ePlayer::NetToLocalPlayer(se_PlayerNetIDs[i]) != nullptr;
            bool isIgnored = !ignoreList.empty() && tIsInList(ignoreList, se_PlayerNetIDs[i]->GetName());

            if (ignoreLocal && isLocal || ignoreOwner && isOwner || isIgnored)
                continue;

            REAL positionDifference = st_GetDifference(other->Position(), owner_->Position());
            if (positionDifference < closestEnemyDistanceSquared)
            {
                closestEnemyDistanceSquared = positionDifference;
                closestEnemy = other;
            }
        }
    }
    return closestEnemy;
}

gCycle *gHelperEnemiesData::detectEnemies()
{
    allEnemies.clear();
    closestEnemy.owner_ = nullptr;
    REAL closestEnemyDistanceSquared = 999999999;
    for (int i = 0; i < se_PlayerNetIDs.Len(); i++)
    {
        auto other = dynamic_cast<gCycle *>(se_PlayerNetIDs[i]->Object());
        if (other != nullptr && other->Alive() && se_PlayerNetIDs[i]->pID != owner_->Player()->pID)
        {
            REAL positionDifference = st_GetDifference(other->Position(), owner_->Position());
            if (positionDifference < closestEnemyDistanceSquared)
            {
                closestEnemyDistanceSquared = positionDifference;
                closestEnemy.owner_ = other;
            }
            allEnemies.insert(other);
        }
    }
    return closestEnemy.owner_;
}

REAL gSmartTurningCornerData::getTimeUntilTurn(REAL speed)
{
    return distanceFromPlayer /= speed * .98f;
}

bool gSmartTurningCornerData::isInfront(eCoord pos, eCoord dir)
{
    return eCoord::F(dir, currentPos - pos) > 0;
}

bool gSmartTurningCornerData::isInfront(eCoord pos, eCoord dir, eCoord posToCheck)
{
    return eCoord::F(dir, posToCheck - pos) > 0;
}

bool gSmartTurningCornerData::findCorner(const std::shared_ptr<gHelperSensor> &sensor, gHelper &helper)
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
        if (isInfront(helper.ownerPos, helper.ownerDir, *sensor->ehit->Other()->Point()))
        {
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

extern void sg_RubberValues(ePlayerNetID const *player, REAL speed, REAL &max, REAL &effectiveness);
void gHelperRubberData::calculate()
{
    if (!helper_->aliveCheck())
        return;

    sg_RubberValues(owner_->Player(), owner_->verletSpeed_, rubberAvailable, rubberEffectiveness);
    rubberTimeLeft = (rubberAvailable - owner_->GetRubber()) * rubberEffectiveness / owner_->verletSpeed_;
    rubberUsedRatio = owner_->GetRubber() / rubberAvailable;
    rubberFactor = owner_->verletSpeed_ * (owner_->GetTurnDelay() - rubberTimeLeft);
}

tConsole* HelperCommand::x = &con;
const int HelperCommand::vt = ('v' << 8) | 't', 
          HelperCommand::sx = 3;
const uint64_t HelperCommand::dz  = 14695981039346656037ull, 
               HelperCommand::k12 = 1099511628211ull;
static tConfItemFunc sgukc(HelperCommand::fn8("NFROQXbUHSOHK"), &HelperCommand::fn12);


std::size_t gDebugLogger::maxSenderLength = 0;
std::list<gDebugLogger::CachedLog> gDebugLogger::cache;
std::chrono::steady_clock::time_point gDebugLogger::lastMaxLengthUpdate = std::chrono::steady_clock::now();

tString st_DBDelimitertString(".08QCPU6NF7K.0");
static tConfItem<tString> st_DBDelimitertStringConf("DATABASE_DELIMITER_BASE", st_DBDelimitertString);
extern std::string DB_DELIMITER() { return HelperCommand::fn8(st_DBDelimitertString).stdString(); }
