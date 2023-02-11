#include "gHelperSensor.h"
#include "../gHelperVar.h"

#include "gZoneHelper.h"
using namespace helperConfig;

void gHelperSensor::PassEdge(const eWall *ww,REAL time,REAL a,int r){
    if (!ww)
        return;

    try{
        gSensor::PassEdge(ww,time,a,r);
    }
    catch( eSensorFinished & e )
    {   
        if (sg_helperSensorsZoneDetection) {
            gZoneHelper::zoneIntersects(this);
        }
        throw;
    }
}
