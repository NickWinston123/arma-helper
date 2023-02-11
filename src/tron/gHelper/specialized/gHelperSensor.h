#ifndef ArmageTron_gHELPER_SENSOR
#define ArmageTron_gHELPER_SENSOR

#include "eSensor.h"
#include "../../gSensor.h"


// sensor sent out to detect near eWalls
class gHelperSensor : public gSensor
{
public:
    gHelperSensor(eGameObject *o, const eCoord &start, const eCoord &d)
        : gSensor(o, start, d), owner_(o) {}

    virtual void PassEdge(const eWall *w,REAL time,REAL,int =1);
eGameObject *owner_;

};
#endif
