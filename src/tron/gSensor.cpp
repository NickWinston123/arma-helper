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

extern REAL sg_delayCycle;

bool gSensor::HitIntersceptsZone(gZone *zone)
{
    eCoord zoneCenter = zone->GetPosition();
    REAL zoneRadius = zone->GetRadius();

    eCoord p3((start_.x - zoneCenter.x), (start_.y - zoneCenter.y)); // shifted line points
    eCoord p4((before_hit.x - zoneCenter.x), (before_hit.y - zoneCenter.y));

    REAL slope = (p4.y - p3.y) / (p4.x - p3.x); // slope of the line
    REAL yIntercept = p3.y - slope * p3.x;      // y-intercept of line

    REAL underRadical = pow(zoneRadius, 2) * pow(slope, 2) + pow(zoneRadius, 2) - pow(yIntercept, 2); // the value under the square root sign

    if (underRadical < 0)
    {
        // line completely missed
        return false;
    }
    else
    {
        REAL intercept1Dis = (-slope * yIntercept + sqrt(underRadical)) / (pow(slope, 2) + 1); // one of the intercept x's
        REAL intercept2Dis = (-slope * yIntercept - sqrt(underRadical)) / (pow(slope, 2) + 1); // other intercept's x
        eCoord intercept(intercept1Dis + zoneCenter.x, slope * intercept1Dis + yIntercept + zoneCenter.y);
        eCoord intercept2(intercept2Dis + zoneCenter.x, slope * intercept2Dis + yIntercept + zoneCenter.y);
        REAL distance = eCoord::F(direction, intercept - start_);
        REAL distance2 = eCoord::F(direction, intercept2 - start_);
        bool exist = intercept == intercept && intercept2 == intercept2;
        con << "EXIST ? " << exist << "\n";
        if (!exist) { return false;}
        distance = (distance);
        distance2 = (distance2);
        
        if (distance2 <= distance)
        {
            intercept = intercept2;
            distance = distance2;
        }

        before_hit = intercept;
        hit = distance;
        type = gSENSOR_RIM;
        con << hit << " " << type << "\n";
        return true;

        // con << "INT1 " << intercept1 << "\n";
        // con << "INT2 " << intercept2 << "\n";
    }
    return false;
}

bool gSensor::calculateZoneHit() {
    //const tList<eGameObject>& gameObjects = Grid()->GameObjects();
    gZone *zone = NULL;
    for (int i=gameObjects.Len()-1;i>=0;i--)
    {
        bool hit = false;
        zone = dynamic_cast<gZone *>(gameObjects(i));

        if (zone)
        {
            hit = HitIntersceptsZone(zone);
        }
        if (hit) {return true;}
    }
    return false;
}
// void gSensor::HitZone(eGameObject * zone ,REAL time){
//     try{
//         eSensor::HitZone(zone,time);
//     }   catch( eSensorFinished & e )
//     {

//     }
// }
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
            gCycle *owner=w->Cycle();
            if (owner && owner->IsMe( owned ) )
            {
                type=gSENSOR_SELF;
            }
            else if ( owner && owned && owner->Team() == owned->Team() )
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

