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

bool gSensor::HitIntersceptsZone2(gZone *zone)
{
    eCoord zoneCenter = zone->GetPosition();
    REAL zoneRadius = zone->GetRadius();

    eCoord p3((start_.x - zoneCenter.x), (start_.y - zoneCenter.y)); // shifted line points
    eCoord p4((before_hit.x - zoneCenter.x), (before_hit.y - zoneCenter.y));

    REAL slope = (p4.y - p3.y) / (p4.x - p3.x); // slope of the line
    REAL yIntercept = p3.y - slope * p3.x;      // y-intercept of line

    REAL underRadical = pow(zoneRadius, 2) * pow(slope, 2) + pow(zoneRadius, 2) - pow(yIntercept, 2); // the value under the square root sign

    //if (underRadical < 0)
    {
        // line completely missed
 //       return false;
    }
   // else
    {
        REAL intercept1Dis = (-slope * yIntercept + sqrt(underRadical)) / (pow(slope, 2) + 1); // one of the intercept x's
        REAL intercept2Dis = (-slope * yIntercept - sqrt(underRadical)) / (pow(slope, 2) + 1); // other intercept's x
        eCoord intercept(intercept1Dis + zoneCenter.x, slope * intercept1Dis + yIntercept + zoneCenter.y);
        eCoord intercept2(intercept2Dis + zoneCenter.x, slope * intercept2Dis + yIntercept + zoneCenter.y);
        REAL distance = eCoord::F(direction_, intercept - start_);
        REAL distance2 = eCoord::F(direction_, intercept2 - start_);
        bool exist = intercept == intercept && intercept2 == intercept2;
        //con << "EXIST ? " << exist << "\n";
        if (!exist) { return false;}
        distance = (distance);
        distance2 = (distance2);

        eDebugLine::SetTimeout(0.01);
        eDebugLine::SetColor  (0, 1, 1);
        eDebugLine::Draw(start_, .1, intercept, .1);
        eDebugLine::SetColor  (1, 1, 0);
        eDebugLine::Draw(start_, .1, intercept2, .1);

        if (distance2 <= distance)
        {
            intercept = intercept2;
            distance = distance2;
        }


        before_hit = intercept;
        hit = distance;
        type = gSENSOR_RIM;
       // con << hit << " " << type << "\n";
        return true;

        // con << "INT1 " << intercept1 << "\n";
        // con << "INT2 " << intercept2 << "\n";
    }
    return false;
}

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

bool gSensor::HitIntersceptsZone(gZone *zone, REAL range)
{


    eCoord zoneCenter = zone->GetPosition();
    REAL zoneRadius = zone->GetRadius();

    std::string test0 = "START \n Sensor Starting POS: " + std::to_string(start_.x) + ", " + std::to_string(start_.y) + "\n Sensor Dir:" + std::to_string(direction_.x) + ", " + std::to_string(direction_.y) + "\n Sensor Pos: " + std::to_string(zoneCenter.x) + ", " + std::to_string(zoneCenter.y) + "\nZone Radius: " + std::to_string(zoneRadius) + "\n";
    DebugLog2(test0);

    // vector from sensor start to zone center
    eCoord zoneToStart = start_ - zoneCenter;
    // vector from sensor start to end point
    eCoord zoneToStartToEnd = direction_ * range;

    std::string test6 = "zoneToStartToEnd.Dot(zoneToStartToEnd): " + std::to_string(zoneToStartToEnd.Dot(zoneToStartToEnd)) + "\n";
    DebugLog2(test6);
    // projection of zoneToStartToEnd onto zoneToStart
    REAL projection = zoneToStart.Dot(zoneToStartToEnd) / zoneToStartToEnd.Dot(zoneToStartToEnd);

    std::string test = "zoneToStart: " + std::to_string(zoneToStart.x) + ", " + std::to_string(zoneToStart.y) + "\n";
    DebugLog2(test);

    std::string test2 = "zoneToStartToEnd: " + std::to_string(zoneToStartToEnd.x) + ", " + std::to_string(zoneToStartToEnd.y) + "\n";
    DebugLog2(test2);

    std::string test3 = "projection: " + std::to_string(projection) + "\n";
    DebugLog2(test3);

    projection = fabs(projection);
   if ( projection < 0.0) {// || projection > zoneToStartToEnd.Norm() ) {
        return false;
    }

    // closest point on sensor to zone center
    eCoord closestPoint = zoneToStart + projection * zoneToStartToEnd.GetNormalized();

    std::string test4 = "closestPoint: " + std::to_string(closestPoint.x) + ", " + std::to_string(closestPoint.y) + "\n";
    DebugLog2(test4);

    // // check if closest point is within the zone
    // if ( ( closestPoint - zoneCenter ).NormSquared() > zoneRadius * zoneRadius ) {
    //     return false;
    // }

    // calculate distance to closest point
    REAL distance = (closestPoint - zoneToStart).Norm();

    std::string test5 = "distance: " + std::to_string(distance) + "\nEND";
    // check if distance is within sensor range
    if (distance > range) {
        return false;
    }

    before_hit = start_ + direction_ * distance;
    eDebugLine::SetTimeout(0.1);
    eDebugLine::SetColor(0, 1, 1);
    eDebugLine::Draw(start_, .1, before_hit, .1);
    hit = distance;
    type = gSENSOR_RIM;
    return true;
}


// bool gSensor::HitIntersceptsZone(gZone *zone, REAL range)
// {

//     std::string test0 = "START \n";
//     DebugLog2(test0);

//     eCoord zoneCenter = zone->GetPosition();
//     REAL zoneRadius = zone->GetRadius();

//     // vector from sensor start to zone center
//     eCoord zoneToStart = start_ - zoneCenter;
//     // vector from sensor start to end point
//     eCoord zoneToStartToEnd = direction_ * range;
//     // projection of zoneToStartToEnd onto zoneToStart
//         REAL projection = zoneToStart.Dot(zoneToStartToEnd) / zoneToStartToEnd.Dot(zoneToStartToEnd);
//     if ( projection < 0 || projection > 1 || zoneToStartToEnd.Dot(zoneToStartToEnd) == 0 )
//         return false;

//     // closest point on sensor to zone center
//     eCoord closestPoint = zoneToStart + projection * zoneToStartToEnd.GetNormalized();

//     std::string test = "zoneToStart: " + std::to_string(zoneToStart.x) + ", " + std::to_string(zoneToStart.y) + "\n";
//     DebugLog2(test);

//     std::string test2 = "zoneToStartToEnd: " + std::to_string(zoneToStartToEnd.x) + ", " + std::to_string(zoneToStartToEnd.y) + "\n";
//     DebugLog2(test2);

//     std::string test3 = "projection: " + std::to_string(projection) + "\n";
//     DebugLog2(test3);

//     std::string test4 = "closestPoint: " + std::to_string(closestPoint.x) + ", " + std::to_string(closestPoint.y) + "\n";
//     DebugLog2(test4);


//     // check if closest point is within the zone
//     if ( ( closestPoint - zoneCenter ).NormSquared() > zoneRadius * zoneRadius )
//         return false;



//     // calculate distance to closest point
//     REAL distance = (closestPoint - zoneToStart).Norm();
//     std::string test5 = "distance: " + std::to_string(distance) + "\nEND";
//     DebugLog2(test5);
//     // check if distance is within sensor range
//     if (distance > range) {
//         return false;
//     }

//     before_hit = start_ + direction_ * distance;
//         eDebugLine::SetTimeout(0.1);
//         eDebugLine::SetColor  (0, 1, 1);
//         eDebugLine::Draw(start_, .1, before_hit, .1);
//     hit = distance;
//     type = gSENSOR_RIM;
//     return true;
// }


void gSensor::calculateZoneHit(REAL range) {
    //const tList<eGameObject>& gameObjects = Grid()->GameObjects();
    gZone *zone = NULL;
    for (int i=gameObjects.Len()-1;i>=0;i--)
    {

        bool hit = false;
        zone = dynamic_cast<gZone *>(gameObjects(i));

        if (zone)
        {
            hit = HitIntersceptsZone(zone, range);
            if (hit) {
                throw eSensorFinished();
            }
        }
    }
    return;
}
// void gSensor::HitZone(eGameObject * zone ,REAL time){
//     try{
//         eSensor::HitZone(zone,time);
//     }   catch( eSensorFinished & e )
//     {

//     }
// }

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

        throw gSensorFinished();
    }


}

