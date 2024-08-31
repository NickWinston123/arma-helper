#include "defs.h"
#include "../gHelper.h"
#include "../../gCycle.h"
#include "../gHelperUtilities.h"
#ifndef ArmageTron_GHELPER_ZONEHELPER
#define ArmageTron_GHELPER_ZONEHELPER

extern bool sg_gSensorsZoneDetection;
class gZone;

struct gZoneHitData
{
    eCoord intercept;

    bool hitZone;
    REAL hit;

    gZoneHitData() : hitZone(false) {}
    gZoneHitData(eCoord intercept_, REAL hit_) : hitZone(true), intercept(intercept_), hit(hit_) {}
};

class gZoneHelper
{
public:
    // Constructor
    gZoneHelper(gHelper &helper, gCycle &owner);
    void Activate(gHelperData &data);

    eCoord closestCorner(gZone *zone);

    void zoneTracer(gHelperData &data);

    gZone *findClosestZone();
    static gZone *findClosestZone(eGameObject *owner_, bool onlyOwnedZones = false);

    void showZones(gHelperData &data);
    void zoneData(gHelperData &data);

    static void zoneIntersects(gSensor *sensor);

    void renderSensorHit(gZoneHitData *zoneHit, gHelperData &data);
    // void zoneSensor(gHelperData &data);

    static gZoneHelper &Get(gHelper &helper, gCycle &owner);

private:
    gCycle &owner_; // Pointer to the owner cycle
    // gZone &closestZone;
    gHelper &helper_;
};
#endif
