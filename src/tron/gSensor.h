/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)

**************************************************************************

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

***************************************************************************

*/

#ifndef ArmageTron_gSENSOR_H
#define ArmageTron_gSENSOR_H

#include "eSensor.h"
#include "eGrid.h"

class eEdge;

// sensor sent out to detect near eWalls
class gSensor : public eSensor
{
public:
    gSensorWallType type;

    gCycle *wallOwner;

    gCycle *hitWallOwner() { return wallOwner; }

    gSensor(eGameObject *o, const eCoord &start, const eCoord &d)
        : eSensor(o, start, d), type(gSENSOR_NONE), owner_(o), gameObjects(owner_->Grid()->GameObjects()), start_(start), direction_(d), wallOwner(NULL)
    {
    }

    eCoord start_;
    eCoord direction_;
    eGameObject *owner_;
    const tList<eGameObject> &gameObjects;
    virtual void PassEdge(const eWall *w, REAL time, REAL, int = 1);
    void detect(REAL range, const eCoord &newPos, const eCoord &newDir, bool render = false);
    void detect(REAL range, bool render = true);
};
#endif
