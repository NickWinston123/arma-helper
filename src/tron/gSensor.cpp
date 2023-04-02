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
#include "gZone.h"
#include "gCycle.h"
#include "gSensor.h"
#include "gWall.h"
#include "eDebugLine.h"
bool sg_sensorsRender = false;
static tConfItem<bool> sg_sensorsRenderC("SENSORS_GSENSOR_RENDER", sg_sensorsRender);

extern REAL sg_delayCycle;

#include "tDirectories.h"
void DebugLog2(std::string message)
{
    std::ofstream o;
    if (tDirectories::Var().Open(o, "helperdebug.txt", std::ios::app))
    {
        o << message << std::endl;
    }
    else
    {
        con << tOutput("Log Error");
    }
    o.close();
}


void gSensor::detect(REAL range, const eCoord &newPos, const eCoord &newDir,bool render){
    pos = newPos;
    dir = newDir;
    //before_hit = eCoord();
    hit = (1000);
    ehit = (NULL);
    lr = 0;

    // if (owned)
    // {
    //     currentFace=owned->currentFace;

    //     // find a better current face if our start postion is not really at the
    //     // current position of the owner
    //     if ( ( grid && !currentFace ) || ( !currentFace->IsInside( pos ) && currentFace->IsInside( owned->pos ) ) )
    //     {
    //         currentFace = owner_->grid->FindSurroundingFace( pos, currentFace );
    //     }
    // }
    // else
    //     currentFace=NULL;
    detect(range, render);
}

//void gSensor::InteractWith(eGameObject *target,REAL,int)
//{}
void gSensor::detect(REAL range, bool render){
    //  eCoord start = pos;
    //  pos=pos+dir*.01;
    //con << "Starting detection at point " << pos << " In direction: " << dir << "\n";
    //con << "Moving from " << pos << " to " << pos+dir*range << "\n";
    start_ = pos;
    direction_ = dir;
    before_hit=pos+dir*(range-.001);
    hit=range+.00001f;
    ehit = 0;

    try
    {
        //calculateZoneHit(range);
        Move(pos+dir*range,0,range);
    }
    catch( eSensorFinished & e )
    {
    }
    if (!render || !sg_sensorsRender){
        return;
    }
        if (hit < range)
    {
        eDebugLine::SetTimeout(0.01);
        eDebugLine::SetColor  (0, 1, 1);
        eDebugLine::Draw(start_, .1, before_hit, .1);

        eDebugLine::SetColor  (0, .5, 1);
        eDebugLine::Draw(before_hit, .1, before_hit, 2.0);
    }
    else
    {
        eDebugLine::SetColor  (1, 0, 0);
        eDebugLine::Draw(start_, .5, pos, .5);
    }
}

void gSensor::PassEdge(const eWall *ww,REAL time,REAL a,int r){
    if (!ww)
        return;

    try{
        eSensor::PassEdge(ww,time,a,r);
    }
    catch( eSensorFinished & e )
    {
            // if (calculateZoneHit()) {
            //      throw;
            // }
                //con << before_hit << "\n";
        const gPlayerWall *w=dynamic_cast<const gPlayerWall*>(ww);
        if (w)
        {
            wallOwner = w->Cycle();
            if (wallOwner && wallOwner->IsMe( owned ) )
            {
                type=gSENSOR_SELF;
            }
            else if ( wallOwner && owned && wallOwner->Team() == owned->Team() )
            {
                type=gSENSOR_TEAMMATE;
            }
            else
            {
                type=gSENSOR_ENEMY;
            }

            if (w->EndTime() < w->BegTime())
                lr=-lr;
        }
        else if (dynamic_cast<const gWallRim*>(ww))
            type=gSENSOR_RIM;

        throw;
    }


}

