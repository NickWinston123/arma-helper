#include "../../gZone.h"
#include "../../gSensor.h"
#include "gHelperSensor.h"

#include "../gHelperVar.h"
#include "../gHelperUtilities.h"
#include "../gHelperHud.h"

using namespace helperConfig;

namespace helperConfig {
bool sg_zoneHelper = false;
static tConfItem<bool> sg_zoneHelperC("HELPER_ZONE", sg_zoneHelper);

// bool sg_zoneHelperSensor = false;
// static tConfItem<bool> sg_zoneHelperSensorC("HELPER_ZONE_SENSOR", sg_zoneHelperSensor);

bool sg_helperZoneShowOwnerOnly = false;
static tConfItem<bool> sg_helperZoneShowOwnerOnlyC("HELPER_ZONE_SHOW_OWNER_ONLY", sg_helperZoneShowOwnerOnly);

bool sg_helperZoneShow = false;
static tConfItem<bool> sg_helperZoneShowConf("HELPER_ZONE_SHOW", sg_helperZoneShow);

REAL sg_helperZoneShowHeight = 1;
static tConfItem<REAL> sg_helperZoneShowHeightConf("HELPER_ZONE_SHOW_HEIGHT", sg_helperZoneShowHeight);

REAL sg_helperZoneShowBonusRadius = 0;
static tConfItem<REAL> sg_helperZoneShowBonusRadiusConf("HELPER_ZONE_SHOW_RADIUS_BONUS", sg_helperZoneShowBonusRadius);

bool sg_helperZoneEnemyZonesColor = false;
static tConfItem<bool> sg_helperZoneEnemyZonesColorConf("HELPER_ZONE_SHOW_ENEMY_RED", sg_helperZoneEnemyZonesColor);

bool sg_helperZoneTracer = false;
static tConfItem<bool> sg_helperZoneTracerConf("HELPER_ZONE_TRACER", sg_helperZoneTracer);

bool sg_helperZoneTracerClosestCorner = false;
static tConfItem<bool> sg_helperZoneTracerClosestCornerConf("HELPER_ZONE_TRACER_CLOSEST_CORNER", sg_helperZoneTracerClosestCorner);

REAL sg_helperZoneTracerTimeoutMult = 1;
static tConfItem<REAL> sg_helperZoneTracerTimeoutMultConf("HELPER_ZONE_TRACER_TIMEOUT_MULT", sg_helperZoneTracerTimeoutMult);

REAL sg_helperZoneTracerBrightness = 1;
static tConfItem<REAL> sg_helperZoneTracerBrightnessConf("HELPER_ZONE_TRACER_BRIGHTNESS", sg_helperZoneTracerBrightness);

REAL sg_helperZoneTracerHeight = 0;
static tConfItem<REAL> sg_helperZoneTracerHeightConf("HELPER_ZONE_TRACER_HEIGHT", sg_helperZoneTracerHeight);

}

gHelperHudItem<tColoredString> zoneDebugH("Zone Debug", tColoredString("None"));
gHelperHudItem<tColoredString> zoneInfoH("Zone Info", tColoredString("None"));

gHelperHudItem<REAL> zoneHitH("Zone Hit", 0);

void debugZone(gZone *zone, tColor color, REAL timeout)
{
    gHelperUtility::debugBox(color, zone->Position(), zone->GetRadius()+sg_helperZoneShowBonusRadius, timeout, sg_helperZoneShowHeight);
}

gZoneHelper::gZoneHelper(gHelper &helper, gCycle &owner)
    : helper_(helper),
      owner_(owner)
{}

void gZoneHelper::zoneIntersects(gSensor *sensor) {
    gZone *zone = findClosestZone(sensor->owner_);
    if (!zone)
        return;
    eCoord sensorDirection = sensor->Direction();
    eCoord zonePos = zone->Position();
    REAL zoneRadius = zone->GetRadius();
    eCoord toZone = zonePos - sensor->start_;

    REAL projection = toZone.Dot(sensorDirection) / sensorDirection.Norm();
    eCoord nearestPoint = sensor->start_ + sensorDirection.GetNormalized() * projection;
    REAL distanceToCenter = (nearestPoint - zonePos).Norm();

    if (distanceToCenter > zoneRadius) {
        return;
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
    if (zoneHitDistance < sensor->hit) {
        sensor->before_hit = intercept;
        sensor->hit = zoneHitDistance;
        sensor->type = gSENSOR_RIM;
    }
}

// Returns the closest corner of the given center and radius to the owner's position
// center: the center of the corners
// radius: the distance from the center to the corners
// Returns: the closest corner
eCoord gZoneHelper::closestCorner(gZone * zone)
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
    gZone * zone = findClosestZone();
    if (!zone)
        return;

    // Draw the tracer line to the nearest corner of the zone
    gHelperUtility::debugLine(tColor(1, 0, 0), sg_helperZoneTracerHeight,
                                data.ownerData.speedFactorF() *sg_helperZoneTracerTimeoutMult,
                                owner_.Position(), closestCorner(zone), sg_helperZoneTracerBrightness);
}

gZone* gZoneHelper::findClosestZone(eGameObject * owner_)
{
    gZone *closestZone = nullptr;
    REAL closestZoneDistanceSquared = 999999999;
    for (std::deque<gZone *>::const_iterator i = sg_HelperTrackedZones.begin(); i != sg_HelperTrackedZones.end(); ++i)
    {
        gZone *zone = *i;
        if (zone) {
            REAL positionDifference = st_GetDifference(zone->Position(), owner_->Position());
            if (positionDifference < closestZoneDistanceSquared) {
                closestZoneDistanceSquared = positionDifference;
                closestZone = zone;
            }
        }
    }
    return closestZone;
}

gZone* gZoneHelper::findClosestZone()
{
    gZone *closestZone = nullptr;
    REAL closestZoneDistanceSquared = 999999999;
    for (std::deque<gZone *>::const_iterator i = sg_HelperTrackedZones.begin(); i != sg_HelperTrackedZones.end(); ++i)
    {
        gZone *zone = *i;
        if (zone) {
            REAL positionDifference = st_GetDifference(zone->Position(), owner_.Position());
            if (positionDifference < closestZoneDistanceSquared) {
                closestZoneDistanceSquared = positionDifference;
                closestZone = zone;
            }
        }
    }
    return closestZone;
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

            if (sg_helperZoneEnemyZonesColor && !zoneIsOurs) {
                color = tColor(1,0,0);
            }
            zoneVelocity = zone->GetVelocity().Norm();
            timeout = zoneVelocity > 0 ? 1 / zoneVelocity : .005;
            debugZone(zone,color,timeout);
        }
    }
}


void gZoneHelper::zoneData(gHelperData &data)
{
    gZone * zone = findClosestZone();
    if (!zone)
        return;
    gBaseZoneHack * fortZone = dynamic_cast<gBaseZoneHack *>(zone);
    if (fortZone) {
    tColoredString info;

    info << "FORT ZONE " << " conquered " << fortZone->conquered_;
    info << "\n";
    zoneInfoH << info;
    }
}

void gZoneHelper::Activate(gHelperData &data)
{
    if (sg_HelperTrackedZones.size() <= 0)
        return;

    if (sg_helperHud) {
        tColoredString debug;
        debug << "Zones size " << sg_HelperTrackedZones.size() << "\n";
        zoneDebugH << debug;
    }
    if (sg_helperZoneShow)
        showZones(data);

    if (sg_helperZoneTracer)
        zoneTracer(data);

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
