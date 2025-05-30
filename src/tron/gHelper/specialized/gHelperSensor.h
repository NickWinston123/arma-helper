#ifndef ArmageTron_gHELPER_SENSOR
#define ArmageTron_gHELPER_SENSOR

#include "eSensor.h"
#include "../../gSensor.h"

// sensor sent out to detect near eWalls
class gHelperSensor : public gSensor
{
public:
    gHelperSensor(eGameObject *o, const eCoord &start, const eCoord &d)
        : gSensor(o, start, d) {}

    gHelperSensor(eGameObject *o, const eCoord &start, int dir);
    
    virtual void PassEdge(const eWall *w, REAL time, REAL, int = 1);
};
#endif
