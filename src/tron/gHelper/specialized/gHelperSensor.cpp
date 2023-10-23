#include "gHelperSensor.h"
#include "../gHelperVar.h"

#include "gZoneHelper.h"
using namespace helperConfig;

void gHelperSensor::PassEdge(const eWall *ww, REAL time, REAL a, int r)
{
    if (!ww)
        return;

    try
    {
        gSensor::PassEdge(ww, time, a, r);
    }
    catch (eSensorFinished &e)
    {
        if (sg_helperSensorsZoneDetection && !sg_gSensorsZoneDetection)
            gZoneHelper::zoneIntersects(this);
        throw;
    }
}

gHelperSensor::gHelperSensor(eGameObject *o, const eCoord &start, int dir)
    : gSensor(o, start, gHelperSensorsData::extractDirection(o, dir)) {}
