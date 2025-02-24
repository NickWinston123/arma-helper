#include "../../gZone.h"
#include "../../gSensor.h"
#include "gHelperSensor.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "../gHelperHud.h"

using namespace helperConfig;

namespace helperConfig
{
    bool sg_zoneHelper = false;
    static tConfItem<bool> sg_zoneHelperC = HelperCommand::tConfItem("HELPER_ZONE", sg_zoneHelper);

    bool sg_helperZoneShowOwnerOnly = false;
    static tConfItem<bool> sg_helperZoneShowOwnerOnlyC = HelperCommand::tConfItem("HELPER_ZONE_SHOW_OWNER_ONLY", sg_helperZoneShowOwnerOnly);

    bool sg_helperZoneShow = false;
    static tConfItem<bool> sg_helperZoneShowConf = HelperCommand::tConfItem("HELPER_ZONE_SHOW", sg_helperZoneShow);

    bool sg_helperZoneInfo = false;
    static tConfItem<bool> sg_helperZoneInfoConf = HelperCommand::tConfItem("HELPER_ZONE_INFO", sg_helperZoneInfo);

    bool sg_helperZoneOnlyOwnedZones = false;
    static tConfItem<bool> sg_helperZoneOnlyOwnedZonesConf = HelperCommand::tConfItem("HELPER_ZONE_INFO_ONLY_OWNED_ZONES", sg_helperZoneOnlyOwnedZones);

    REAL sg_helperZoneShowHeight = 1;
    static tConfItem<REAL> sg_helperZoneShowHeightConf = HelperCommand::tConfItem("HELPER_ZONE_SHOW_HEIGHT", sg_helperZoneShowHeight);

    REAL sg_helperZoneShowBonusRadius = 0;
    static tConfItem<REAL> sg_helperZoneShowBonusRadiusConf = HelperCommand::tConfItem("HELPER_ZONE_SHOW_RADIUS_BONUS", sg_helperZoneShowBonusRadius);

    bool sg_helperZoneEnemyZonesColor = false;
    static tConfItem<bool> sg_helperZoneEnemyZonesColorConf = HelperCommand::tConfItem("HELPER_ZONE_SHOW_ENEMY_RED", sg_helperZoneEnemyZonesColor);

    bool sg_helperZoneTracer = false;
    static tConfItem<bool> sg_helperZoneTracerConf = HelperCommand::tConfItem("HELPER_ZONE_TRACER", sg_helperZoneTracer);

    bool sg_helperZoneTracerClosestCorner = false;
    static tConfItem<bool> sg_helperZoneTracerClosestCornerConf = HelperCommand::tConfItem("HELPER_ZONE_TRACER_CLOSEST_CORNER", sg_helperZoneTracerClosestCorner);

    REAL sg_helperZoneTracerTimeoutMult = 1;
    static tConfItem<REAL> sg_helperZoneTracerTimeoutMultConf = HelperCommand::tConfItem("HELPER_ZONE_TRACER_TIMEOUT_MULT", sg_helperZoneTracerTimeoutMult);

    REAL sg_helperZoneTracerBrightness = 1;
    static tConfItem<REAL> sg_helperZoneTracerBrightnessConf = HelperCommand::tConfItem("HELPER_ZONE_TRACER_BRIGHTNESS", sg_helperZoneTracerBrightness);

    REAL sg_helperZoneTracerHeight = 0;
    static tConfItem<REAL> sg_helperZoneTracerHeightConf = HelperCommand::tConfItem("HELPER_ZONE_TRACER_HEIGHT", sg_helperZoneTracerHeight);

    REAL sg_zoneSensorRadiusBonus = 0;
    static tConfItem<REAL> sg_zoneSensorRadiusBonusConf = HelperCommand::tConfItem("ZONE_SENSOR_RADIUS_BONUS", sg_zoneSensorRadiusBonus);

}

static gHelperHudItemRef<bool> sg_helperSmartTurningH("Status", sg_zoneHelper, "Zone Helper");
static gHelperHudItem<tColoredString> zoneDebugH("Zone Debug", tColoredString("None"), "Zone Helper");
static gHelperHudItem<tColoredString> zoneInfoH("Zone Info", tColoredString("None"), "Zone Helper");


void debugZone(gZone *zone, tColor color, REAL timeout)
{
    gHelperUtility::debugBox(color, zone->Position(), zone->GetRadius() + sg_helperZoneShowBonusRadius, timeout, sg_helperZoneShowHeight);
}

gZoneHelper::gZoneHelper(gHelper &helper, gCycle &owner)
    : helper_(helper),
      owner_(owner)
{
}

void gZoneHelper::zoneIntersects(gSensor *sensor)
{

    for (std::deque<gZone *>::const_iterator i = sg_HelperTrackedZones.begin(); i != sg_HelperTrackedZones.end(); ++i)
    {
        gZone *zone = *i;
        if (!zone)
            continue;

        eCoord sensorDirection = sensor->Direction();
        eCoord zonePos = zone->Position();
        REAL zoneRadius = zone->GetRadius() + sg_zoneSensorRadiusBonus;
        eCoord toZone = zonePos - sensor->start_;

        REAL projection = toZone.Dot(sensorDirection);

        // Check if the zone is in the direction of the sensor's movement
        if (projection < 0)
            continue;

        eCoord nearestPoint = sensor->start_ + sensorDirection.GetNormalized() * (projection / sensorDirection.Norm());
        REAL distanceToCenter = (nearestPoint - zonePos).Norm();

        if (distanceToCenter > zoneRadius)
        {
            continue;
        }

        // Calculate the intersection points between the sensor's movement path and the zone's circle
        REAL length = sqrt(zoneRadius * zoneRadius - distanceToCenter * distanceToCenter);
        eCoord directionToIntercept = sensorDirection.GetNormalized();
        eCoord intercept1 = nearestPoint + directionToIntercept * length;
        eCoord intercept2 = nearestPoint - directionToIntercept * length;

        // Choose the intercept point that is closer to the sensor's starting point
        eCoord intercept = (intercept1 - sensor->start_).Norm() < (intercept2 - sensor->start_).Norm() ? intercept1 : intercept2;

        REAL zoneHitDistance = (intercept - sensor->start_).Norm();

        // Update data if zone hit distance is less than initial hit
        if (zoneHitDistance < sensor->hit && zoneHitDistance >= 0)
        {
            sensor->before_hit = intercept;
            sensor->hit = zoneHitDistance;
            sensor->type = gSENSOR_ZONE;
        }
    }
}

// Returns the closest corner of the given center and radius to the owner's position
// center: the center of the corners
// radius: the distance from the center to the corners
// Returns: the closest corner
eCoord gZoneHelper::closestCorner(gZone *zone)
{
    eCoord ourPos = owner_.Position();
    eCoord center = zone->Position();
    REAL radius = zone->GetRadius();

    // Define the 4 corners
    eCoord corner[4];
           corner[0] = eCoord(center.x - (radius), center.y + (radius));
           corner[1] = eCoord(center.x + (radius), center.y + (radius));
           corner[2] = eCoord(center.x + (radius), center.y - (radius));
           corner[3] = eCoord(center.x - (radius), center.y - (radius));

    // Calculate the difference between each corner and the owner's position
    REAL positionDifference[4];
         positionDifference[0] = st_GetDifference(corner[0], ourPos);
         positionDifference[1] = st_GetDifference(corner[1], ourPos);
         positionDifference[2] = st_GetDifference(corner[2], ourPos);
         positionDifference[3] = st_GetDifference(corner[3], ourPos);

    // Find the index of the minimum difference
    int minIndex = std::min_element(positionDifference, positionDifference + 4) - positionDifference;

    // Return the closest corner
    return corner[minIndex];
}

void gZoneHelper::zoneTracer(gHelperData &data)
{
    gZone *zone = findClosestZone();
    if (!zone)
        return;

    // Draw the tracer line to the nearest corner of the zone
    gHelperUtility::debugLine(tColor(1, 0, 0), sg_helperZoneTracerHeight,
                              data.ownerData.speedFactorF() * sg_helperZoneTracerTimeoutMult,
                              owner_.Position(), closestCorner(zone), sg_helperZoneTracerBrightness);
}

gZone *gZoneHelper::findClosestZone(eGameObject *owner_, bool onlyOwnedZones)
{
    gZone *closestZone = nullptr;
    REAL closestZoneDistanceSquared = 999999999;
    for (std::deque<gZone *>::const_iterator i = sg_HelperTrackedZones.begin(); i != sg_HelperTrackedZones.end(); ++i)
    {
        gZone *zone = *i;
        if (zone && (!onlyOwnedZones || (zone->Owned() || zone->Owner() > 0)) && zone->GetRadius() > 10)
        {
            REAL positionDifference = st_GetDifference(zone->Position(), owner_->Position());
            if (positionDifference < closestZoneDistanceSquared)
            {
                closestZoneDistanceSquared = positionDifference;
                closestZone = zone;
            }
        }
    }
    return closestZone;
}

gZone *gZoneHelper::findClosestZone()
{
    return findClosestZone(&owner_);
}

void gZoneHelper::showZones(gHelperData &data)
{
    REAL zoneVelocity, timeout;
    for (std::deque<gZone *>::const_iterator i = sg_HelperTrackedZones.begin(); i != sg_HelperTrackedZones.end(); ++i)
    {
        gZone *zone = *i;

        if (!zone || zone->destroyed_)
            continue;

        bool zoneIsOurs = zone->Team() == owner_.Team();
        if (!sg_helperZoneShowOwnerOnly || sg_helperZoneShowOwnerOnly && zoneIsOurs)
        {
            tColor color(zone->GetColor().r, zone->GetColor().g, zone->GetColor().b);

            if (sg_helperZoneEnemyZonesColor && !zoneIsOurs)
            {
                color = tColor(1, 0, 0);
            }
            zoneVelocity = zone->GetVelocity().Norm();
            timeout = zoneVelocity > 0 ? 1 / zoneVelocity : .005;
            debugZone(zone, color, timeout);
        }
    }
}

void gZoneHelper::zoneData(gHelperData &data)
{
    gZone *zone = findClosestZone(&owner_,sg_helperZoneOnlyOwnedZones);
    if (!zone)
    {
        tColoredString info;
        info << "None";

        zoneInfoH << info;
        return;
    }

    // if (sg_helperDebug)
    //     gHelperUtility::Debug("zoneData","Zone found, displaying info.");

    tColoredString info;

    info << "ZONE ID:               " << zone->GetID() << "\n";
    info << "ZONE ID:               " << zone->ID() << "\n";
    info << "Position: (            " << zone->GetPosition().x << ", " << zone->GetPosition().y << ")\n";
    info << "Velocity: (            " << zone->GetVelocity().x << ", " << zone->GetVelocity().y << ")\n";
    info << "Radius:                " << zone->GetRadius() << "\n";
    info << "Expansion Speed:       " << zone->GetExpansionSpeed() << "\n";
    info << "Rotation Speed:        " << zone->GetRotationSpeed() << "\n";
    info << "Rotation Acceleration: " << zone->GetRotationAcceleration() << "\n";
    info << "Owner:                 " << (zone->GetOwner() ? zone->GetOwner()->GetName() : "None") << "\n";
    info << "Player:                " << (zone->Player() ? zone->Player()->GetName() : "None") << "\n";
    info << "Owner ID:              " << (zone->Owner()) << "\n";
    info << "Owned by us?:          " << booleanToString(zone->Owned()) << "\n";
    info << "Color:                 " << int(zone->GetColor().r*15) << " " << int(zone->GetColor().g*15) << " " << int(zone->GetColor().b*15) << "\n";
    info << "Effect:                " << zone->GetEffect() << "\n";
    info << "Zone Name:             " << zone->GetName() << "\n";
    //info << "Destroyed:           " << booleanToString(zone->destroyed_) << "\n";
    // info << "Helper Destroyed:      " << booleanToString(zone->helperDestroyed_) << "\n";
    // info << "OWNERSHIP           " << booleanToString(zone->testthishere) << "\n";
    //info << "Reference Time:      " << zone->referenceTime_ << "\n";
    //info << "Create Time:         " << zone->createTime_ << "\n";
    //info << "Actual Create Time:  " << zone->actualCreateTime_ << "\n";
    //info << "Next Update:         " << zone->nextUpdate_ << "\n";

    gSumoZoneHack *sumoZone = dynamic_cast<gSumoZoneHack *>(zone);
    if (sumoZone)
    {
        info << "SUMO ZONE\n";
        // info << "Conquered: " << fortZone->conquered_ << "\n";
        // info << "Enemies Inside: " << fortZone->enemiesInside_ << "\n";
        // info << "Owners Inside: " << fortZone->ownersInside_ << "\n";
        // info << "Only Survivor: " << booleanToString(fortZone->onlySurvivor_) << "\n";
        // info << "Last Enemy Contact: " << fortZone->lastEnemyContact_ << "\n";
        // info << "Team Distance: " << fortZone->teamDistance_ << "\n";
        // //info << "Touchy: " << booleanToStatus(fortZone->touchy_) << "\n";
        // info << "Current State: " << fortZone->currentState_ << "\n";
        //info << "Last Sync Time: " << fortZone->lastSync_ << "\n";
        //info << "Last Respawn Remind Time: " << fortZone->lastRespawnRemindTime_ << "\n";
        //info << "Last Respawn Remind Waiting: " << fortZone->lastRespawnRemindWaiting_ << "\n";
    }

    zoneInfoH << info;
}

void gZoneHelper::Activate(gHelperData &data)
{
    if (sg_HelperTrackedZones.size() <= 0)
        return;

    if (sg_helperHud)
    {
        tColoredString debug;
        debug << "Zones size " << sg_HelperTrackedZones.size() << "\n";
        zoneDebugH << debug;
    }
    if (sg_helperZoneShow)
        showZones(data);

    if (sg_helperZoneTracer)
        zoneTracer(data);

    if (sg_helperZoneInfo)
        zoneData(data);
}

gZoneHelper &gZoneHelper::Get(gHelper &helper, gCycle &owner)
{
    tASSERT(&owner);

    // create
    if (helper.zoneHelper.get() == 0)
        helper.zoneHelper.reset(new gZoneHelper(helper, owner));

    return *helper.zoneHelper;
}
