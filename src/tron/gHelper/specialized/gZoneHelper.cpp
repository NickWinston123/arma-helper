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

bool sg_zoneHelperSensor = false;
static tConfItem<bool> sg_zoneHelperSensorC("HELPER_ZONE_SENSOR", sg_zoneHelperSensor);

bool sg_helperZoneShowOwnerOnly = false;
static tConfItem<bool> sg_helperZoneShowOwnerOnlyC("HELPER_ZONE_SHOW_OWNER_ONLY", sg_helperZoneShowOwnerOnly);

bool sg_helperZoneShow = false;
static tConfItem<bool> sg_helperZoneShowConf("HELPER_ZONE_SHOW", sg_helperZoneShow);

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

void debugZone(gZone *zone, gRealColor color, REAL timeout)
{
    gHelperUtility::debugBox(color, zone->Position(), zone->GetRadius()+sg_helperZoneShowBonusRadius, timeout);
}

gZoneHelper::gZoneHelper(gHelper &helper, gCycle &owner)
    : helper_(helper),
      owner_(owner)
{}

void gZoneHelper::renderSensorHit( gZoneHitData * zoneHit,gHelperData &data){
    if (zoneHit->hitZone) {
        zoneHitH << zoneHit->hit;
        gHelperUtility::Debug("zoneSensor","SENSOR ZONE HIT, INTERCEPT: ", zoneHit->intercept);
        gHelperUtility::debugLine(gRealColor(1,0,0), 1, data.ownerData.speedFactorF() + 1, owner_.Position(), zoneHit->intercept);
    }
}

void gZoneHelper::zoneSensor(gHelperData &data)
{
    gZone *zone = findClosestZone();

    if (!zone)
        return;

    gZoneHitData * zoneHit = zoneIntersects(LEFT, zone, data);
    renderSensorHit(zoneHit,data);

    zoneHit = zoneIntersects(FRONT, zone, data);
    renderSensorHit(zoneHit,data);

    zoneHit = zoneIntersects(RIGHT, zone, data);
    renderSensorHit(zoneHit,data);

    delete zoneHit;

}

void gZoneHelper::zoneIntersects(gHelperSensor * sensor) {
    gZone *zone = findClosestZone(sensor->owner_);
    if (!zone)
        return;

    eCoord sensorDirection = sensor->Direction();
    eCoord zonePos = zone->Position();
    REAL zoneRadius = zone->GetRadius();
    eCoord toZone = zonePos - sensor->start_;
    eCoord sensorDir = sensor->direction_;

    REAL projection = toZone.Dot(sensorDirection) / sensorDirection.Norm();
    eCoord nearestPoint = sensor->start_ + sensorDirection.GetNormalized() * projection;
    REAL distanceToCenter = (nearestPoint - zonePos).Norm();

    if (distanceToCenter > zoneRadius)
    {
        return;
    }

    // Check if the nearest point is inside the circle
    if (distanceToCenter < zoneRadius)
    {
        sensor->before_hit = nearestPoint;
        sensor->hit = (nearestPoint-sensor->start_).Norm();
        return;
    }

    // Check if the nearest point is on the circle
    eCoord toIntercept = (nearestPoint - zonePos).GetNormalized() * sqrt(zoneRadius * zoneRadius - distanceToCenter * distanceToCenter);
    eCoord intercept = nearestPoint + toIntercept;
    sensor->before_hit = intercept;
    sensor->hit = (nearestPoint-sensor->start_).Norm();

}

gZoneHitData *gZoneHelper::zoneIntersects(int dir, gZone *zone,gHelperData &data)
{

    gHelperSensor* sensor  = data.sensors.getSensor(dir,true);
    eCoord sensorDirection = sensor->Direction();
    eCoord zonePos = zone->Position();
    REAL zoneRadius = zone->GetRadius();
    eCoord toZone = zonePos - sensor->start_;
    eCoord sensorDir = sensor->direction_;

    REAL projection = toZone.Dot(sensorDirection) / sensorDirection.Norm();
    eCoord nearestPoint = sensor->start_ + sensorDirection.GetNormalized() * projection;
    REAL distanceToCenter = (nearestPoint - zonePos).Norm();

    if (distanceToCenter > zoneRadius)
    {
        return new gZoneHitData();
    }

    // Check if the nearest point is inside the circle
    if (distanceToCenter < zoneRadius)
    {
        return new gZoneHitData(nearestPoint, (nearestPoint-sensor->start_).Norm());
    }

    // Check if the nearest point is on the circle
    eCoord toIntercept = (nearestPoint - zonePos).GetNormalized() * sqrt(zoneRadius * zoneRadius - distanceToCenter * distanceToCenter);
    eCoord intercept = nearestPoint + toIntercept;
    return new gZoneHitData(intercept, (nearestPoint-sensor->start_).Norm());
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
    gHelperUtility::debugLine(gRealColor(1, 0, 0), sg_helperZoneTracerHeight,
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
            gRealColor color(zone->GetColor().r, zone->GetColor().g, zone->GetColor().b);

            if (sg_helperZoneEnemyZonesColor && !zoneIsOurs) {
                color = gRealColor(1,0,0);
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

    if (sg_zoneHelperSensor)
        zoneSensor(data);

    zoneData(data);
}

gZoneHelper &gZoneHelper::Get(gHelper &helper, gCycle &owner)
{
    tASSERT(owner);

    // create
    if (helper.zoneHelper.get() == 0)
        helper.zoneHelper.reset(new gZoneHelper(helper, owner));

    return *helper.zoneHelper;
}
