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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

***************************************************************************

*/

#include "gAIBase.h"
#include "gArena.h"
#include "eGrid.h"
#include "ePath.h"
#include "eGameObject.h"
#include "gWall.h"
#include "gSensor.h"
#include "gCycle.h"
#include "tConsole.h"
#include "rFont.h"
#include "rScreen.h"
#include "eFloor.h"
#include "eDebugLine.h"
#include "gAICharacter.h"
#include "gAINavigator.h"
#include "tReferenceHolder.h"
#include "tRandom.h"
#include "tRecorder.h"
#include "gZone.h"
#include "gTutorial.h"
#include <stdlib.h>
#include <cstdlib>
#include <memory>

#ifdef DEBUG
#include "tCommandLine.h"
#endif

#define AI_REACTION 0
#define AI_EMERGENCY 1
#define AI_RANGE 2
#define AI_STATE_TRACE 3
#define AI_STATE_CLOSECOMBAT 4
#define AI_STATE_PATH 5
#define AI_LOOP 6
#define AI_ENEMY 7
#define AI_TUNNEL 8
#define AI_DETECTTRACE 9
#define AI_STARTSTATE 10
#define AI_STARTSTRAIGHT 11
#define AI_STATECHANGE 12

static tReferenceHolder<gAIPlayer> sg_AIReferences;

static tCONTROLLED_PTR(gAITeam) sg_AITeam = NULL;

gSimpleAIFactory *gSimpleAIFactory::factory_ = NULL;

gCycle *gAIPlayer::Object()
{
    eGameObject *go = ePlayerNetID::Object();
    if (!go)
    {
        return NULL;
    }
    else
    {
        tASSERT(dynamic_cast<gCycle *>(go));
        return static_cast<gCycle *>(go);
    }
}

gSimpleAI::~gSimpleAI()
{
    con << "simple AI destroyed.\n";
}

gSimpleAI *gSimpleAIFactory::Create(gCycle *object) const
{
    gSimpleAI *ai = DoCreate();
    if (ai)
        ai->SetObject(object);
    return ai;
}

gSimpleAIFactory *gSimpleAIFactory::Get()
{
    return factory_;
}

void gSimpleAIFactory::Set(gSimpleAIFactory *factory)
{
    factory_ = factory;
}

static gAITeam *AITeam()
{
    if (!sg_AITeam)
    {
        sg_AITeam = tNEW(gAITeam);
    }

    return sg_AITeam;
}

static void ClearAITeam()
{
    sg_AITeam = NULL;
}

// an instance of this class will prevent deterministic random lookups
class gRandomController
{
public:
    static gRandomController *random_;
    gRandomController *lastRandom_;
    tRandomizer &randomizer_;

    gRandomController(tRandomizer &randomizer = tRandomizer::GetInstance())
        : lastRandom_(random_), randomizer_(randomizer)
    {
        random_ = this;
    }

    ~gRandomController()
    {
        random_ = lastRandom_;
    }
};

gRandomController *gRandomController::random_ = 0;

REAL Random()
{
    if (gRandomController::random_)
    {
        tRandomizer &randomizer = gRandomController::random_->randomizer_;
        return randomizer.Get();
    }
    else
    {
        tRandomizer &randomizer = tRandomizer::GetInstance();
        return randomizer.Get();
    }
}

// from gGame.cpp
extern REAL sg_timestepMax;
static REAL sg_FrameTime()
{
    REAL ret = se_AverageFrameTime();
    if (ret > sg_timestepMax)
    {
        ret = sg_timestepMax;
    }

    return ret;
}

static REAL Delay()
{
    REAL delay = sg_delayCycle * .9f;

    REAL fd = sg_FrameTime() * 1.5f;
    if (fd > delay)
        delay = fd;

    return delay;
}

static gAICharacter *BestIQ(int iq)
{
    int i;

    static tArray<bool> inGame(gAICharacter::s_Characters.Len());
    for (i = gAICharacter::s_Characters.Len() - 1; i >= 0; i--)
        inGame(i) = false;

    for (i = se_PlayerNetIDs.Len() - 1; i >= 0; i--)
    {
        // count the active AIs
        ePlayerNetID *p = se_PlayerNetIDs(i);
        gAIPlayer *ai = dynamic_cast<gAIPlayer *>(p);
        if (ai && ai->Character())
        {
            int index = ai->Character() - &(gAICharacter::s_Characters(0));
            inGame(index) = true;
        }
    }

    // find the best fitting IQ that could be inserted:
    gAICharacter *bestIQ = 0;
    for (i = gAICharacter::s_Characters.Len() - 1; i >= 0; i--)
        if (!inGame(i))
            if (!bestIQ || fabs(bestIQ->iq - iq) > fabs(gAICharacter::s_Characters(i).iq - iq))
                bestIQ = &gAICharacter::s_Characters(i);

    return bestIQ;
}

gAITeam::gAITeam(nMessage &sender)
    : eTeam(sender)
{
    //	teams.Remove( this, listID );
}

gAITeam::gAITeam()
{
    //	teams.Remove( this, listID );
}

static nNOInitialisator<gAITeam> gAITeam_init(331, "gAITeam");

nDescriptor &gAITeam::CreatorDescriptor() const
{
    return gAITeam_init;
}
// fill empty team positions with AI players
void gAITeam::BalanceWithAIs(bool balanceWithAIs)
{
    // set correct team number
    EnforceConstraints();

    int numTeams = 0, numTeamsWithPlayers = 0;

    // determine the maximum number of human players on a team
    int i;
    int maxP = eTeam::minPlayers;
    for (i = teams.Len() - 1; i >= 0; --i)
    {
        eTeam *t = teams(i);

        t->UpdateProperties();

        if (t->BalanceThisTeam())
            numTeams++;

        if (t->NumHumanPlayers() > 0)
            numTeamsWithPlayers++;

        int humans = t->NumHumanPlayers();

        if (humans > maxP)
        {
            maxP = humans;
        }
    }

    // make sure all teams have equal number
    for (i = teams.Len() - 1; i >= 0; --i)
    {
        eTeam *t = teams(i);

        if (t->BalanceThisTeam())
        {
            int wishAIs = maxP - t->NumHumanPlayers();

            // don't add AI players to human team if it is not requested
            if (!balanceWithAIs && t->NumHumanPlayers() > 0)
                wishAIs = 0;

            // don't genereate AI only teams if it is not requested by the min team count
            if (numTeamsWithPlayers >= minTeams && t->NumHumanPlayers() == 0)
                wishAIs = 0;

            // add AI only teams to team count
            if (wishAIs > 0 && t->NumHumanPlayers() == 0)
                numTeamsWithPlayers++;

            int AIs = t->NumAIPlayers();

            while (AIs > wishAIs)
            {
                int j;
                gAIPlayer *throwOut = NULL;
                for (j = t->NumPlayers() - 1; j >= 0; --j)
                {
                    gAIPlayer *player = dynamic_cast<gAIPlayer *>(t->Player(j));
                    if (player && !player->IsHuman() && (!throwOut || !throwOut->Character() || !player->Character() || throwOut->Character()->iq > player->Character()->iq))
                    {
                        throwOut = player;
                    }
                }

                if (throwOut)
                {
                    throwOut->RemoveFromGame();
                    //					throwOut->ReleaseOwnership();

#ifdef DEBUG
                    if (throwOut->GetRefcount() > 1)
                    {
                        st_Breakpoint();
                    }
#endif

                    sg_AIReferences.Remove(throwOut);
                }

                --AIs;
            }

            while (AIs < wishAIs)
            {
                // add the smartest AI player you can get
                gAICharacter *best = BestIQ(1000);

                if (best)
                {
                    gAIPlayer *ai = tNEW(gAIPlayer)();
                    ai->character_ = best;
                    ai->SetName(best->name);
                    ai->SetTeam(t);
                    ai->UpdateTeam();

                    sg_AIReferences.Add(ai);
                }

                ++AIs;
            }
        }
    }

    // get rid of deleted netobjects (teams, mostly)
    nNetObject::ClearAllDeleted();
}

// may player join this team?
bool gAITeam::PlayerMayJoin(const ePlayerNetID *player) const
{
    return !player->IsHuman();
}

// this class describes a single wall close event
class gCycleTouchEvent
{
public:
    REAL dist;      // the position on this cycle's wall the touching happened
    REAL otherDist; // the position on the other cylce's wall
    int otherSide;  // the side of the other cylce this wall touches
    int winding;    // winding number to add if we cross here

    gCycleTouchEvent()
    {
        dist = 0;
        otherDist = 0;
        otherSide = -100;
    }
};

class gCycleMemoryEntry
{
public:
    gCycleMemory *memory;
    int id;
    nObserverPtr<gCycle> cycle;

    gCycleMemoryEntry(gCycleMemory *m, const gCycle *c)
        : memory(m), id(-1), cycle(c)
    {
        memory->memory.Add(this, id);

        max[0].dist = -1E+30;
        max[1].dist = -1E+30;
        min[0].dist = 1E+30;
        min[1].dist = 1E+30;
    }

    ~gCycleMemoryEntry()
    {
        memory->memory.Remove(this, id);
    }

    // usable data
    gCycleTouchEvent max[2]; // latest touch event (with the given cylce)
    gCycleTouchEvent min[2]; // earliest touch event
};

#define TOL 4

// look for a closed loop in the walls if cycle a hits cycle b's wall at
// distance bDist and on side bSide.
// Look for the loop in driving direction of b if dir is 1 or to the other side of dir is 0.
// the end of the loop is reached when the wall of cycle a is driven along in
// direction aEndDir, passing the distance aEndDist and the side's bit is set
// in aEndSides.
// Cycles that will be closed in the loop are stored in the array
// closedIn.
// return value: the number of open points this loop contains
// (if this is >0, that usually means the space is wide open)
//  or -1 if there is no loop.
static bool CheckLoop(const gCycle *a, const gCycle *b,
                      REAL bDist, int bSide, int dir,
                      tArray<const gCycle *> &closedIn, int &winding,
                      REAL aEndDist = 0, int aEndSides = 3, int aEndDir = 1)
{
    tASSERT(0 <= bSide && 1 >= bSide);
    tASSERT(0 <= dir && 1 >= dir);

    int tries = 10; // so long until we give up
    int ends = 0;

    bool bClosedIn = false;

    const gCycle *run = b; // we run along this cycle's wall
    int end = dir;         // and move towards this end its wall wall
    int side = bSide;      // we are on this side of the cycle
    REAL dist = bDist;     // and are at this distance.
    winding = 0;           // the winding number we collected

    int turn = a->Grid()->WindingNumber();
    int halfTurn = turn >> 1;

#ifdef DEBUG
    //  con << "\n";
#endif

    while (tries-- > 0 && run &&
           !(run == a &&
             end == aEndDir &&
             aEndSides & (1 << side) &&
             (end > 0 ? dist >= aEndDist : dist <= aEndDist)))
    {
#ifdef DEBUG
        //      con << "end = " << end << ", side = " << side << ", dist = " << dist
        //	  << ", winding = " << winding << "\n";
#endif
        if (end > 0)
        {

            // find the last connection
            gCycleMemoryEntry *last = run->memory.Latest(side);
            if (!last || last->max[side].dist <= dist + TOL)
            {
                // no interference. We can move directly around the cylce
                // and close it in.
#ifdef DEBUG
                //	      con << "Turning around...\n";
#endif

                winding += halfTurn *
                           (side > 0 ? -1 : 1);

                end = 0;
                side = 1 - side;
                closedIn[closedIn.Len()] = run;
                dist = run->GetDistance();

                // detect early loop
                if (run == b)
                {
                    if (bClosedIn)
                    {
                        winding = 0;
                        return false;
                    }
                    else
                        bClosedIn = true;
                }
            }
            else
            {
#ifdef DEBUG
                //	      con << "Crossing...\n";
#endif

                // find the first connection
                gCycleMemoryEntry *first = run->memory.Earliest(side);
                if (first && first->min[side].dist >= dist + TOL)
                {
                    // we cross the connection:
                    winding += first->min[side].winding;
                    run = first->cycle;
                    end = (side == first->min[side].otherSide) ? 1 : 0;

                    //		  if (end == 0)
                    // we need to turn around to follow
                    winding += halfTurn * (side > 0 ? 1 : -1);

                    dist = first->min[side].otherDist;
                    side = first->min[side].otherSide;
                }
                else
                {
                    winding = 0;
                    return false;
                }
            }
        }
        else // dir = -1, we move towards the end
        {
            // find the first connection
            gCycleMemoryEntry *first = run->memory.Earliest(side);
            if (!first || first->min[side].dist >= dist - TOL)
            {
#ifdef DEBUG
                //	      con << "Turning around...\n";
#endif

                // no interference. We can move directly around the cylce's end.
                winding += halfTurn * (side > 0 ? 1 : -1);

                end = 1;
                side = 1 - side;
                ends++;
                dist = -2 * TOL;
            }
            else
            {
#ifdef DEBUG
                //	      con << "Crossing...\n";
#endif

                // find the latest connection
                gCycleMemoryEntry *last = run->memory.Latest(side);
                if (last && last->max[side].dist <= dist - TOL)
                {
                    // we cross the connection:
                    winding += last->max[side].winding;

                    // we need to turn around to start:
                    winding += halfTurn * (side > 0 ? -1 : 1);

                    run = last->cycle;
                    end = (side == last->max[side].otherSide) ? 0 : 1;

                    //		  if (end == 1)
                    // we need to turn around to follow
                    //		  winding -= halfTurn * (side > 0 ? 1 : -1);

                    dist = last->max[side].otherDist;
                    side = last->max[side].otherSide;
                }
                else
                // uh oh. we are already closed in. No chance...
                {
                    winding = 0;
                    return false;
                }
            }
        }
    }

#ifdef DEBUG
    //      con << "end = " << end << ", side = " << side << ", dist = " << dist
    //	  << ", winding = " << winding << "\n\n";
#endif

    if (tries >= 0)
    {
        return true;
    }
    else
    {
        winding = 0;
        return false;
    }
}

// see if the given Cycle is trapped currently
static bool IsTrapped(const gCycle *trapped, const gCycle *other)
{
    tArray<const gCycle *> closedIn;
    int winding = 0;
    if (CheckLoop(trapped, trapped, trapped->GetDistance(), 1, 0, closedIn, winding, trapped->GetDistance() - 1))
    {
        if (winding + 2 < 0)
        {
            // see if the other cylce is trapped with him
            for (int i = closedIn.Len() - 1; i >= 0; i--)
                if (other == closedIn(i))
                    return false; // we can get him!

            // no. trapped is trapped allone.
            return true;
        }
    }

    return false;
}

// see if the given Cycle is trapped currently
bool IsTrapped(const eGameObject *trapped, const gCycle *other)
{
    gCycle const *cycle = dynamic_cast<gCycle const *>(trapped);
    if (cycle)
        return IsTrapped(cycle, other);
    return false;
}

// data about a loop
class gLoopData
{
public:
    bool loop;                       // is there a loop?
    int winding;                     // in what direction does it go?
    tArray<const gCycle *> closedIn; // which cycles are closed in?

    gLoopData() : loop(false), winding(0) {}
    void AddCycle(const gCycle *c) { closedIn[closedIn.Len()] = c; }
};

// hit data
class gHitData
{
public:
    const eHalfEdge *edge;    // edge we hit
    gSensorWallType wallType; // type of the wall hit
    int lr;                   // does the wall go left or right?
    REAL distance;            // distance to the wall

    // additional info if the wall that got hit is a cycle wall
    gCycle *otherCycle; // the cylce that the hit wall belongs to
    REAL driveDistance; // the distance it had travelled when it was at the place we hit
    int windingNumber;  // the winding number at the place hit

    gHitData() : edge(NULL), wallType(gSENSOR_NONE), otherCycle(NULL) {}

    bool Hit() const { return edge; }

    void AddHit(const eCoord &origin, const eCoord &dir, const gSensor &sensor, int winding)
    {
        if (!sensor.ehit)
            return; // no hit, nothing to do

        /*
        REAL otherDist = eCoord::F(*sensor.ehit->Point(), dir)
                         - eCoord::F(origin, dir);

        {
            REAL otherDist2 = eCoord::F(*sensor.ehit->Other()->Point(), dir)
                              - eCoord::F(origin, dir);
            if (otherDist2 < otherDist)
                otherDist = otherDist2;
        }
        */
        REAL otherDist = eCoord::F(dir, sensor.before_hit - origin);

        if (otherDist < EPS)
            return; // the new hit is a wall we apparently drive along; nothing to do

        if (Hit() && distance < otherDist)
            return; // the hit that is already stored is more relevant. Ignore the new hit.

        // copy the relevant data
        edge = sensor.ehit;
        wallType = sensor.type;
        lr = sensor.lr;
        distance = otherDist;

        REAL alpha = edge->Ratio(sensor.before_hit);

        // get the extra information from the wall we hit:
        gPlayerWall *w = dynamic_cast<gPlayerWall *>(edge->GetWall());
        if (!w)
        {
            alpha = 1 - alpha;
            w = dynamic_cast<gPlayerWall *>(edge->Other()->GetWall());
        }

        if (w)
        {
            otherCycle = w->Cycle();
            driveDistance = w->Pos(alpha);
            windingNumber = w->WindingNumber() - winding;
        }
    }
};

// special sensor that scans a broader area (not just a raycast)
class gAISensor
{
public:
    // the raw data we collect:

    const gCycle *cycle; // the cycle that sent out this sensor

    bool hit; // whether a dangerous spot is hit

    gHitData front;    // what happens in our front
    gHitData sides[2]; // and on our sides

    // what we make of it:
    gLoopData frontLoop[2];   // does the front wall we hit cause us to be trapped if we turn left or right?
    gLoopData sideLoop[2][2]; // do the two side walls cause us to be trapped if we turn/drive straight on?

    REAL distance; // distance to the closest wall

    bool Hit() const
    {
        return hit;
    }

    void DetectLoop(const gHitData &hit, gLoopData loopData[2])
    {
        // avoid loops:
        gCycle *other = hit.otherCycle;

        if (other)
        {
            //	  if (other!= Object())
            {
                REAL dist = hit.driveDistance;
                int otherSide = hit.lr < 0 ? 0 : 1;
                for (int i = 1; i >= 0; i--)
                {
                    loopData[i].closedIn.SetLen(0);
                    loopData[i].loop = false;

                    int lr = i + i - 1;
                    int dir = hit.lr * lr > 0 ? 1 : 0;
                    int winding = 0;
                    bool loop = CheckLoop(cycle, other,
                                          dist, otherSide, dir,
                                          loopData[i].closedIn, winding);

                    if (loop)
                    {
                        // complete the winding calculation: the target winding
                        // is the one of this cycle:
                        winding += cycle->WindingNumber();
                        // and the source is the winding of the wall we hit
                        winding -= hit.windingNumber;

                        //		      if (dir == 0)
                        //			winding -= lr * (Object()->Grid()->WindingNumber() >> 1);

                        winding += lr;

#ifdef DEBUG_X
                        if (winding != 4 && winding != -4)
                        {
                            gRandomController noRandom;
                            if (winding != 0)
                                //			st_Breakpoint();

                                winding = 0;
                            CheckLoop(cycle, other,
                                      dist, otherSide, dir,
                                      loopData[i].closedIn, winding);

                            winding += cycle->WindingNumber();
                            winding -= hit.windingNumber;
                            winding += lr;
                        }
#endif

#ifdef DEBUG
                        //		      con << "winding = " << winding << " ,direction = " << lr << "\n";
#endif
                        // if the winding continues the direction we would turn in,
                        // we're trapped.
                        if (winding * lr > 0)
                            loopData[i].loop = true;

                        loopData[i].winding = winding;
                    }
                }
            }
        }
    }

    gAISensor(const gCycle *c,
              const eCoord &start, const eCoord &dir,
              REAL sideScan,     // the amout of space we should scan left and right
              REAL frontScan,    // the total front scanning range
              REAL corridorScan, // the corridor scanning range
              int winding        // direction relative to the cycle's driving direction
              )
        : cycle(c), distance(frontScan * 2)
    {
        gAIPlayer *ai = dynamic_cast<gAIPlayer *>(c->Player());
        tASSERT(ai);
        gAICharacter *character = ai->Character();
        tASSERT(character);

        hit = false;
        
        //eDebugLine::SetTimeout(.5);

        gCycle *cycle = const_cast<gCycle *>(this->cycle);

        // detect straight ahead
        gSensor ahead(cycle, start, dir);

        int count = 0;

        do
        {
            ahead.detect(frontScan);
            front.AddHit(start, dir, ahead, winding);
            if (front.Hit())
            {
                hit = true;
                distance = front.distance;
                if (character->properties[AI_LOOP] > 3 + fabsf(winding) * 3)
                    DetectLoop(front, frontLoop);
            }
        } while (!front.Hit() && count++ < character->properties[AI_RANGE]);

        // adapt the corridor distance so the corridor is not looked for too far away
        if (distance * .99f < corridorScan)
            corridorScan = distance * .99f;
        corridorScan -= sideScan * .02;
        if (corridorScan < 0.1f)
            corridorScan = 0.1f;

        if (Random() * 10 < character->properties[AI_TUNNEL])
        {
            // check the sides
            eCoord lookTunnel = start + dir * corridorScan;

            int i;

            for (i = 1; i >= 0; i--)
            {
                int lr = i + i - 1;
                eCoord lrDir = dir.Turn(eCoord(.01f, -lr));

                gSensor side(cycle, start, lrDir);
                side.detect(sideScan * 1.01f);
                REAL thisSideScan = side.hit * .99f;

                gSensor tunnel(cycle, lookTunnel, lrDir);
                tunnel.detect(thisSideScan);
                sides[i].AddHit(start, dir, tunnel, winding + lr);

                gSensor parallel(cycle, start + lrDir * thisSideScan, dir);
                parallel.detect(corridorScan);
                sides[i].AddHit(start, dir, parallel, winding);

                if (sides[i].Hit())
                {
                    if (character->properties[AI_LOOP] > 6 + fabsf(winding) * 3)
                        DetectLoop(sides[i], sideLoop[i]);

                    if (sideLoop[i][1 - i].loop ||
                        (character->properties[AI_TUNNEL] >= 10 &&
                         sides[i].otherCycle &&
                         sides[i].otherCycle->Team() != cycle->Team() &&
                         sides[i].lr * (i + i - 1) < 0 &&
                         sides[i].otherCycle->GetDistance() < sides[i].driveDistance + sides[i].otherCycle->Speed() * 20))
                    {
                        if (sides[i].distance < distance)
                            distance = sides[i].distance;

                        hit = true;
                    }
                }

#ifdef DEBUG_X
                {
                    gRandomController noRandom;
                    gSensor ahead(cycle, start, dir);
                    ahead.detect(frontScan);

                    gSensor side(cycle, start, lrDir);
                    side.detect(sideScan * 1.01f);
                    REAL thisSideScan = side.hit * .99f;

                    gSensor tunnel(cycle, lookTunnel, lrDir);
                    tunnel.detect(thisSideScan);
                    //	  sides[i].AddHit(start, dir, tunnel, winding + lr);

                    gSensor parallel(cycle, start + lrDir * thisSideScan, dir);
                    parallel.detect(corridorScan);
                    // sides[i].AddHit(start, dir, parallel, winding);
                }
#endif
            }
        }
        if (sides[1].otherCycle == sides[0].otherCycle && sides[0].otherCycle)
            hit = true;

#ifdef DEBUG
        if (Hit())
        {
            eDebugLine::SetColor(1, 1, 1);
            eDebugLine::SetTimeout(1);
            eDebugLine::Draw(start, .5, start + dir * distance, .5);

            eDebugLine::SetColor(1, .5, 1);
            eDebugLine::Draw(start + dir * distance, .5, start + dir * distance, 1.5);
        }
#endif

        //eDebugLine::SetTimeout(0);
    }
};

gCycleMemoryEntry *gCycleMemory::Latest(int side) const
{
    side = (side > 0 ? 1 : 0);
    gCycleMemoryEntry *ret = NULL;
    for (int i = memory.Len() - 1; i >= 0; i--)
    {
        gCycleMemoryEntry *m = memory(i);
        if ((!ret || ((m->max[side].dist > ret->max[side].dist) && bool(m->cycle) && m->cycle->Alive())))
            ret = memory(i);
    }

    return ret;
}

gCycleMemoryEntry *gCycleMemory::Earliest(int side) const
{
    side = (side > 0 ? 1 : 0);
    gCycleMemoryEntry *ret = NULL;
    for (int i = memory.Len() - 1; i >= 0; i--)
    {
        gCycleMemoryEntry *m = memory(i);
        if ((!ret || ((m->min[side].dist < ret->min[side].dist) && bool(m->cycle) && m->cycle->Alive())))
            ret = memory(i);
    }
    return ret;
}

gCycleMemoryEntry *gCycleMemory::Remember(const gCycle *cycle)
{
    for (int i = memory.Len() - 1; i >= 0; i--)
        if (memory(i)->cycle == cycle)
            return memory(i);

    return tNEW(gCycleMemoryEntry)(this, cycle);
}

gCycleMemory::gCycleMemory()
{
}

gCycleMemory::~gCycleMemory()
{
    Clear();
}

void gCycleMemory::Clear()
{
    for (int i = memory.Len() - 1; i >= 0; i--)
        delete memory(i);
}

gCycleMemoryEntry *gCycleMemory::operator()(int i) const
{
    tASSERT(0 <= i && i < Len());
    return memory(i);
}

// deeper analysis functions:

// helper function for gAIPlayer::CycleBlocksWay()
static void CycleBlocksWayHelper(const gCycle *a, const gCycle *b,
                                 int aDir, int bDir, REAL aDist, REAL bDist, int winding)
{
    gCycleMemoryEntry *aEntry = (const_cast<gCycleMemory &>(a->memory)).Remember(b);

    if (aDist > aEntry->max[aDir].dist)
    {
        aEntry->max[aDir].dist = aDist;
        aEntry->max[aDir].otherSide = bDir;
        aEntry->max[aDir].otherDist = bDist;
        aEntry->max[aDir].winding = winding;
    }

    if (aDist < aEntry->min[aDir].dist)
    {
        aEntry->min[aDir].dist = aDist;
        aEntry->min[aDir].otherSide = bDir;
        aEntry->min[aDir].otherDist = bDist;
        aEntry->min[aDir].winding = winding;
    }
}

// *******************
// * Tracing state   *
// *******************

class gStateTrace : public gAIPlayer::State
{
public:
    gStateTrace(gAIPlayer &ai, int dir) : gAIPlayer::State(ai), dir_(dir) {}

    // evaluator for tracing
    class Evaluator : public gAINavigator::PathEvaluator
    {
    public:
        Evaluator(gCycle &cycle, int dir) : cycle_(cycle), dir_(dir) {}

        virtual void Evaluate(gAINavigator::Path const &path, gAINavigator::PathEvaluation &evaluation) const
        {
            gAINavigator::WallHug const *hug = 0, *noHug = 0;
            if (dir_ > 0)
            {
                hug = &path.left;
                noHug = &path.right;
            }
            else
            {
                hug = &path.right;
                noHug = &path.left;
            }

            bool good = hug->owner && hug->owner->Team() != cycle_.Team();
            if (good)
            {
                if (hug->owner != noHug->owner)
                {
                    evaluation.score = 50;
                }
                else if ((path.shortTermDirection - path.longTermDirection).NormSquared() < .01)
                {
                    evaluation.score = 50 + .5 * Adjust(path.immediateDistance / (cycle_.Speed() * cycle_.GetTurnDelay()));
                }
            }
        }

    private:
        gCycle &cycle_;
        int dir_;
    };

    virtual REAL Think(REAL maxStep)
    {
        Navigator().UpdatePaths();
        gAINavigator::EvaluationManager manager(Navigator().GetPaths());
        manager.Evaluate(gAINavigator::SuicideEvaluator(*Parent().Object()), 1);
        manager.Reset();
        manager.Evaluate(Evaluator(*Parent().Object(), dir_), 1);
        manager.Evaluate(gAINavigator::SpaceEvaluator(*Parent().Object()), .5);
        manager.Evaluate(gAINavigator::PlanEvaluator(), .1);

        gAINavigator::CycleControllerBasic controller;
        return manager.Finish(controller, *Parent().Object(), maxStep);
    }

private:
    int dir_;
};

// ********************
// * tailchase helper *
// ********************

// *******************
// * Survival state  *
// *******************

class gStateSurvive : public gAIPlayer::State
{
public:
    gStateSurvive(gAIPlayer &ai) : gAIPlayer::State(ai) {}

    virtual REAL Think(REAL maxStep)
    {
        gCycle &cycle = *Parent().Object();
        Navigator().UpdatePaths();
        gAINavigator::EvaluationManager manager(Navigator().GetPaths());
        manager.Evaluate(gAINavigator::SuicideEvaluator(cycle), 1);
        manager.Evaluate(gAINavigator::SuicideEvaluator(cycle, maxStep), 1);
        manager.Evaluate(gAINavigator::TrapEvaluator(cycle), 1);
        manager.Reset();
        manager.Evaluate(gAINavigator::CowardEvaluator(cycle), 5);
        manager.Evaluate(gAINavigator::SpaceEvaluator(cycle), 1);
        manager.Evaluate(gAINavigator::RandomEvaluator(cycle), .01);
        manager.Evaluate(gAINavigator::PlanEvaluator(), .1);

        gAINavigator::CycleControllerBasic controller;
        return manager.Finish(controller, *Parent().Object(), maxStep);
    }
};

void gAIPlayer::SwitchToSurvival()
{
    SwitchToState(tNEW(gStateSurvive)(*this));
}

void gAIPlayer::SetTarget(eNetGameObject *target)
{
    this->target_ = target;

    // let's see how well this works:
    // state = AI_CLOSECOMBAT;

    // start with it right now
    nextTime_ = -1;
}

// called whenever cylce a drives close to the wall of cylce b.
// directions: aDir tells whether the wall is to the left (-1) or right(1)
// of a
// bDir tells the direction the wall of b is going (-1: to the left, 1:...)
// bDist is the distance of b's wall to its start.
void gAIPlayer::CycleBlocksWay(const gCycleMovement *aa, const gCycleMovement *bb,
                               int aDir, int bDir, REAL bDist, int winding)
{

    return;

    tASSERT(aa && bb);
    gCycle *a = dynamic_cast<gCycle *>(const_cast<gCycleMovement *>(aa));
    gCycle *b = dynamic_cast<gCycle *>(const_cast<gCycleMovement *>(bb));

    REAL aDist = a->GetDistance();
    aDir = (aDir > 0 ? 1 : 0);
    bDir = (bDir > 0 ? 1 : 0);

    int w = winding + aDir * 2 + bDir * 2;
    while (w < 0)
        w += 400;
    if (w % 4 != 2)
        return;

    CycleBlocksWayHelper(a, b, aDir, bDir, aDist, bDist, winding);
    CycleBlocksWayHelper(b, a, bDir, aDir, bDist, aDist, -winding);

    // what to do if cylce a tries to trace cylce b?
    if (a->Team() != b->Team())
    {
        gAIPlayer *ai = dynamic_cast<gAIPlayer *>(b->Player());
        if (ai && dynamic_cast<gZone const *>(ai->GetTarget()))
        {
            // ai is attacking or defending a zone, don't disturb it
            return;
        }

        if (ai && ai->Character() && ai->Character()->properties[AI_DETECTTRACE] > 5)
            if (aDir != bDir)
            {
                REAL behind = b->GetDistance() - bDist;
                if (a->Speed() > b->Speed() * 1.2f && behind < (a->Speed() - b->Speed()) * 10)
                { // a is faster. Try to escape.
                    ai->SwitchToState(tNEW(gStateTrace)(*ai, aDir > 0 ? 1 : -1));
                    ai->target_ = const_cast<gCycle *>(a);
                }
                else // if (a->Speed() < b->Speed() * 1.1f)
                {    // b is faster. Attack.
                    ai->SwitchToState(tNEW(gStateTrace)(*ai, aDir > 0 ? -1 : 1));
                    ai->target_ = const_cast<gCycle *>(a);
                }
            }

        // what to do if the AI player traces his opponent by accident? Trace On!
        ai = dynamic_cast<gAIPlayer *>(a->Player());

        if (ai && dynamic_cast<gZone const *>(ai->GetTarget()))
        {
            // ai is attacking or defending a zone, don't disturb it
            return;
        }

        if (ai && ai->Character() && ai->Character()->properties[AI_DETECTTRACE] > 0)
            if (aDir != bDir)
            {
                ai->SwitchToState(tNEW(gStateTrace)(*ai, aDir > 0 ? 1 : -1));
                ai->target_ = const_cast<gCycle *>(b);
            }
    }
}

// called whenever a cylce blocks the rim wall.
void gAIPlayer::CycleBlocksRim(const gCycleMovement *a, int aDir)
{
}

// called whenever a hole is ripped in a's wall at distance aDist.
void gAIPlayer::BreakWall(const gCycleMovement *a, REAL aDist)
{
}

// #define DEBUG

// which eWall is detected?

#define MAXAI_COLOR 13

static int current_ai;
static REAL rgb_ai[MAXAI_COLOR][3] = {
    {1, .2, .2},
    {.2, 1, .2},
    {.2, .2, 1},
    {1, 1, .2},
    {1, .2, 1},
    {.2, 1, 1},
    {1, .6, .2},
    {1, .2, .6},
    {.6, .2, 1},
    {.2, .6, 1},
    {1, 1, 1},
    {.2, .2, .2},
    {.5, .5, .5}};

static nNOInitialisator<gAIPlayer> gAIPlayer_init(330, "gAIPlayer");

nDescriptor &gAIPlayer::CreatorDescriptor() const
{
    return gAIPlayer_init;
}

//! creates a netobject form sync data
gAIPlayer::gAIPlayer(nMessage &sender) : ePlayerNetID(sender),
                                         character_(NULL),
                                         simpleAI_(NULL),
                                         target(NULL),
                                         target_(NULL),
                                         lastPath(se_GameTime() - 100),
                                         lastTime(se_GameTime()),
                                         nextTime(0),
                                         concentration(1),
                                         log(NULL),
                                         state(AI_SURVIVE)
{
}

gAIPlayer::gAIPlayer() : simpleAI_(NULL),
                         character_(NULL),
                         target(NULL),
                         target_(NULL),
                         lastPath(se_GameTime() - 100),
                         lastTime(se_GameTime()),
                         nextTime(0),
                         concentration(1),
                         log(NULL),
                         state(AI_SURVIVE)
{
    ClearTarget();

    // find a good color
    current_ai = (current_ai + 1) % MAXAI_COLOR;
    int take_ai = current_ai;
    int try_ai = current_ai;

    REAL maxmindist = -10000;

    for (int i = MAXAI_COLOR - 1; i >= 0; i--)
    {
        REAL mindist = 4;
        REAL score = 0;
        for (int j = se_PlayerNetIDs.Len() - 1; j >= -1; j--)
        {
            REAL R, G, B; // the color we want to avoid

            if (j >= 0)
            {
                ePlayerNetID *p = se_PlayerNetIDs(j);
                if (p && p != this)
                {
                    R = p->r / 15.0;
                    G = p->g / 15.0;
                    B = p->b / 15.0;
                }
                else
                    continue;
            }
            else // last case: j = -1. Test against floor color
            {
                se_FloorColor(R, G, B);
            }
            REAL dist =
                fabs(R - rgb_ai[try_ai][0]) +
                fabs(G - rgb_ai[try_ai][1]) +
                fabs(B - rgb_ai[try_ai][2]);

            score += exp(-dist * dist * 4);

            if (dist < mindist)
            {
                mindist = dist;
                /*	   con << c->r << ":" << rgb_ai[try_ai][0] << '\t'
                       << c->g << ":" << rgb_ai[try_ai][1] << '\t'
                       << c->b << ":" << rgb_ai[try_ai][2] << '\t' << dist << '\n'; */
            }
        }
        // con << "md=" << mindist << "\n\n";
        if (mindist > 2)
            mindist = 2;

        mindist = -score;

        if (mindist > maxmindist)
        {
            maxmindist = mindist;
            take_ai = try_ai;
        }

        try_ai = ((try_ai + 1) % MAXAI_COLOR);
    }

    r = static_cast<int>(rgb_ai[take_ai][0] * 15);
    g = static_cast<int>(rgb_ai[take_ai][1] * 15);
    b = static_cast<int>(rgb_ai[take_ai][2] * 15);

    ping = 0;
    pingCharity = 300;

    NewObject();
}

void gAIPlayer::ConfigureAIs() // ai configuration menu
{
}

// make sure this many AI players are in the game
void gAIPlayer::SetNumberOfAIs(int num, int minPlayers, int iq, int tries)
{
    // balance the human teams with AI players
    gAITeam::BalanceWithAIs();

    // remove AI players that got kicked out of their team
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; i--)
    {
        // count the active AIs
        ePlayerNetID *p = se_PlayerNetIDs(i);
        gAIPlayer *ai = dynamic_cast<gAIPlayer *>(p);
        if (ai && !bool(ai->NextTeam()))
        {
            ai->RemoveFromGame();

#ifdef DEBUG
            if (ai->GetRefcount() > 1)
            {
                st_Breakpoint();
            }
#endif

            sg_AIReferences.Remove(ai);
        }
    }

    int count = 0;

    bool iqperfect = false;

    // repeat until we run out of tries or the total amount of AIs is correct
    do
    {
        count = 0;

        int i;
        gAIPlayer *worstIQ = NULL; // worst fitting AI player that is in the game

        //		static tArray<bool> inGame(gAICharacter::s_Characters.Len());
        //		for (i = gAICharacter::s_Characters.Len()-1; i>=0; i--)
        //			inGame(i) = false;

        for (i = se_PlayerNetIDs.Len() - 1; i >= 0; i--)
        {
            // count the active AIs
            ePlayerNetID *p = se_PlayerNetIDs(i);
            gAIPlayer *ai = dynamic_cast<gAIPlayer *>(p);
            if (ai && ai->NextTeam() == sg_AITeam)
            {
                //				int index = ((int)ai->Character() - (int)&gAICharacter::s_Characters(0))/sizeof(gAICharacter);
                //				inGame(index) = true;

                if (!worstIQ || !worstIQ->character_ || fabs(worstIQ->character_->iq - iq) < fabs(ai->character_->iq - iq))
                    worstIQ = ai;

                count++;
            }
        }

        gAICharacter *bestIQ = BestIQ(iq);

        // count = numberOfAIs to add / remove(numberofAIs - requestedNumAIs)
        count -= num;

        // pcount = numberPlayers above minPlayers
        int pcount = -minPlayers;
        // check if more AIs are required (because there are less than minPlayers players)
        for (i = se_PlayerNetIDs.Len() - 1; i >= 0; i--)
        {
            ePlayerNetID *p = se_PlayerNetIDs(i);
            if (p->CurrentTeam())
                ++pcount;
        }

        if (pcount < count)
            count = pcount;

        iqperfect = true;
        if (bestIQ && worstIQ && worstIQ->Character())
            iqperfect = (fabs(bestIQ->iq - iq) > fabs(worstIQ->Character()->iq - iq) * .9f);

        // count complete. Do something!
        if (worstIQ && (count > 0 || (count >= 0 && !iqperfect)))
        {
            // too many AIs. Delete the one with the least fitting intelligence.
            worstIQ->RemoveFromGame();
            //			worstIQ->ReleaseOwnership();

#ifdef DEBUG
            if (worstIQ->GetRefcount() > 1)
            {
                st_Breakpoint();
            }
#endif

            sg_AIReferences.Remove(worstIQ);
            count--;

            // remove empty AI team
            if (0 == AITeam()->NumPlayers())
            {
                ClearAITeam();
            }
        }
        if (bestIQ && count < 0)
        {
            // too litte AIs. Create one.
            gAIPlayer *ai = tNEW(gAIPlayer)();
            ai->SetName(bestIQ->name);
            ai->character_ = bestIQ;

            sg_AIReferences.Add(ai);

            ai->SetTeam(AITeam());
            ai->UpdateTeam();

            /*
            {
                tOutput mess;
                tColoredString printname;
                printname << *(ePlayerNetID*)ai << tColoredString::ColorString(.5,1,.5);

                mess.SetTemplateParameter(1, printname);
                mess << "$player_entered_game";

                sn_ConsoleOut( mess );
            }
            */

            count++;
        }

    } while ((count != 0 ||
              !iqperfect) &&
             tries-- != 0);
}

// Possible state changes:
// Every state -> Survive for 20 seconds if the victim is dead or can be assumed dead soon, or if the situation gets too dangerous

// Survive -> CloseCombat if survival gets too boring

// Trace -> Closecombat
// Path  -> Closecombat  if the victim gets in view

// Survive -> Trace if an enemy wall is hit
// Path    -> Trace

// CloseCombat -> Path if the vicim gets out of view

void gAIPlayer::SetTraceSide(int side)
{
    REAL time = se_GameTime();
    REAL ts = time - lastChangeAttempt + 1;
    lastChangeAttempt = time;

    lazySideChange += ts * side;
    if (lazySideChange * traceSide <= 0)
    {
        // state change!
        traceSide = lazySideChange > 0 ? 1 : -1;
        lazySideChange = 10 * traceSide;
    }

    if (lazySideChange > 10)
        lazySideChange = 10;
    if (lazySideChange < -10)
        lazySideChange = -10;
}

// flag set if pathfinding is enabled. It's an expensive opeeration, so we just turn it
// off if the PC can't handle it. Doesn't doo much good, anyway.
static bool sg_pathEnabled = true;

// state change:
void gAIPlayer::SwitchToState(State *state)
{
    state_ = state;
}

void gAIPlayer::SwitchToState(gAI_STATE nextState, REAL minTime)
{
    int thisAbility = 10 - character_->properties[AI_STATE_TRACE];
    switch (state)
    {
    case AI_TRACE:
        thisAbility = character_->properties[AI_STATE_TRACE];
        break;
    case AI_CLOSECOMBAT:
        thisAbility = character_->properties[AI_STATE_CLOSECOMBAT];
        break;
    case AI_PATH:
        thisAbility = character_->properties[AI_STATE_PATH];
        break;
    case AI_PATH_GIVEN:
    case AI_PATH_MINDLESS:
        thisAbility = 1000;
        break;
    case AI_SURVIVE:
        break;
    };

    int nextAbility = 10;
    switch (nextState)
    {
    case AI_TRACE:
        nextAbility = character_->properties[AI_STATE_TRACE];
        break;
    case AI_CLOSECOMBAT:
        nextAbility = character_->properties[AI_STATE_CLOSECOMBAT];
        break;
    case AI_PATH:
    case AI_PATH_GIVEN:
    case AI_PATH_MINDLESS:
        nextAbility = character_->properties[AI_STATE_PATH];
        if (!sg_pathEnabled)
        {
            nextAbility = 0;
        }
        break;
    case AI_SURVIVE:
        break;
    };

    if (nextAbility > thisAbility && Random() * 10 > nextAbility)
        return;

#ifdef DEBUG
    if (state != nextState)
        con << "Switching to state " << nextState << "\n";
#endif

    state = nextState;
    nextStateChange = se_GameTime() + minTime;
}

// add a coordinate to the path, switch to path follow mode
void gAIPlayer::AddToPath(eCoord const &target, bool mindless)
{
    gAI_STATE desiredState = mindless ? AI_PATH_MINDLESS : AI_PATH_GIVEN;
    if (state != desiredState)
    {
        if( state != AI_PATH_GIVEN && state != AI_PATH_MINDLESS )
        {
            path.Clear();
        }

        // hard state change, avoid random changes back from it
        state = desiredState;
        lastPath = 10000;
        nextStateChange = 10000;
        nextTime = 0;
    }
    path.Add(target);
}

// state update functions:
void gAIPlayer::ThinkSurvive(ThinkData &data)
{
    if (!character_)
    {
        st_Breakpoint();
        return;
    }

    REAL random = 0;
    // do nothing much. Rely on the emergency program.
    /*
      random=10*(Random()/float(1));
      if (random < .2)
      EmergencySurvive(front, left, right, -1, 1);
      else if (random > 9.8)
      EmergencySurvive(front, left, right, -1, -1);
      else
      if (front.front.wallType == gSENSOR_RIM && front.distance < 10)
      st_Breakpoint();


    */

    if (data.left.front.wallType == gSENSOR_RIM)
        EmergencySurvive(data, 1);
    else if (data.right.front.wallType == gSENSOR_RIM)
        EmergencySurvive(data, -1);
    else
        EmergencySurvive(data);

    if (nextStateChange > se_GameTime())
    {
        data.thinkAgain = .5f;
        return;
    }

    // switch from Survival to close combat if surviving is too boring
    random = 10 * Random();
    if (random < 5)
    {
        // find a new victim:
        eCoord enemypos = eCoord(1000, 100);

        const tList<eGameObject> &gameObjects = Object()->Grid()->GameObjects();
        gCycle *secondbest = NULL;

        // find the closest enemy
        for (int i = gameObjects.Len() - 1; i >= 0; i--)
        {
            gCycle *other = dynamic_cast<gCycle *>(gameObjects(i));

            if (other && other->Team() != Object()->Team() &&
                !IsTrapped(other, Object()))
            {
                // then, enemy is realy an enemy
                eCoord otherpos = other->Position() - Object()->Position();
                if (otherpos.NormSquared() < enemypos.NormSquared())
                {
                    // check if the path is clear
                    gSensor p(Object(), Object()->Position(), otherpos);
                    p.detect(REAL(.98));
                    secondbest = dynamic_cast<gCycle *>(other);
                    if (p.hit >= .98)
                    {
                        enemypos = otherpos;
                        target = secondbest;
                    }
                }
            }
        }

        if (!target)
            target = secondbest;

        if (target)
            SwitchToState(AI_CLOSECOMBAT, 1);
    }

    data.thinkAgain = 1;
}

void gAIPlayer::ThinkTrace(ThinkData &data)
{
    gAISensor const &front = data.front;
    gAISensor const &left = data.left;
    gAISensor const &right = data.right;

    bool inverse = front.Hit() && front.distance < Object()->Speed() * Delay();

    if (left.front.wallType == gSENSOR_RIM)
        SetTraceSide(1);

    if (right.front.wallType == gSENSOR_RIM)
        SetTraceSide(-1);

    bool success = EmergencySurvive(data, 0, traceSide * (inverse ? -1 : 1));

    REAL &nextTurn = data.thinkAgain;
    nextTurn = 100;
    if (left.front.edge)
    {
        REAL a = eCoord::F(Object()->Direction(), *left.front.edge->Point() - Object()->Position());
        REAL b = eCoord::F(Object()->Direction(), *left.front.edge->Other()->Point() - Object()->Position());

        if (a < b)
            a = b;
        if (a > 0)
            nextTurn = a;
    }

    if (right.front.edge)
    {
        REAL a = eCoord::F(Object()->Direction(), *right.front.edge->Point() - Object()->Position());
        REAL b = eCoord::F(Object()->Direction(), *right.front.edge->Other()->Point() - Object()->Position());

        if (a < b)
            a = b;
        if ((a > 0 && a < nextTurn) || !left.front.edge)
            nextTurn = a;
    }

    nextTurn /= Object()->Speed() * .98f;

    REAL delay = Delay() * 1.5f;
    if ((!Object()->CanMakeTurn(1) || !Object()->CanMakeTurn(-1) || success) && nextTurn > delay)
        nextTurn = delay;

    if (nextTurn > .3f)
        nextTurn = .3f;

    if (nextStateChange > se_GameTime())
        return;

    // find a new victim:
    eCoord enemypos = eCoord(1000, 100);

    const tList<eGameObject> &gameObjects = Object()->Grid()->GameObjects();
    gCycle *secondbest = NULL;

    // find the closest enemy
    for (int i = gameObjects.Len() - 1; i >= 0; i--)
    {
        gCycle *other = dynamic_cast<gCycle *>(gameObjects(i));

        if (other && other->Team() != Object()->Team() &&
            !IsTrapped(other, Object()))
        {
            // then, enemy is realy an enemy
            eCoord otherpos = other->Position() - Object()->Position();
            if (otherpos.NormSquared() < enemypos.NormSquared())
            {
                // check if the path is clear
                gSensor p(Object(), Object()->Position(), otherpos);
                p.detect(REAL(.98));
                secondbest = dynamic_cast<gCycle *>(other);

                if (!target)
                    enemypos = otherpos;

                if (p.hit >= .98)
                {
                    enemypos = otherpos;
                    target = secondbest;
                }
            }
        }
    }

    eCoord relpos = enemypos.Turn(Object()->Direction().Conj()).Turn(0, 1);

    if (!target)
        target = secondbest;
    else
        SwitchToState(AI_CLOSECOMBAT, 1);

    if (target)
        SetTraceSide((relpos.x > 0 ? 10 : -10) *
                     (target->Speed() > Object()->Speed() ? -1 : 1));

    nextStateChange = se_GameTime() + 10;

    //  SwitchToState(AI_SURVIVE, 1);
    return;
}

void gAIPlayer::ThinkPath(ThinkData &data)
{

    eCoord dir = Object()->Direction();

    if (!target || !target->CurrentFace() || IsTrapped(target, Object()))
    {
        SwitchToState(AI_SURVIVE, 1);
        EmergencySurvive(data);

        data.thinkAgain = 4;
        return;
    }

    eCoord tDir = target->Position() - Object()->Position();

    if (nextStateChange < se_GameTime())
    {
        if (!sg_pathEnabled)
        {
            SwitchToState(AI_SURVIVE, 5);
            EmergencySurvive(data);

            return;
        }

        gSensor p(Object(), Object()->Position(), tDir);
        p.detect(REAL(.9999999));
        if (p.hit >= .9999999) // free line of sight to victim. Switch to close combat.
        {
            SwitchToState(AI_CLOSECOMBAT, 5);
            EmergencySurvive(data);

            return;
        }
    }

    // find a new path if the one we got is outdated:
    if (lastPath < se_GameTime() - 10)
        if (target->CurrentFace())
        {
            if (!sg_pathEnabled)
            {
                // yeah, apparently, we can't go on with this. Bail out.
                SwitchToState(AI_SURVIVE, 5);
                EmergencySurvive(data);
                return;
            }

            Object()->FindCurrentFace();
            REAL before = tRealSysTimeFloat();
            eHalfEdge::FindPath(Object()->Position(), Object()->CurrentFace(),
                                target->Position(), target->CurrentFace(),
                                Object(),
                                path);

            // calculate (and archive) time used for pathfinding
            REAL used = tRealSysTimeFloat() - before;
            static char const *section = "PATH_TIME";
            tRecorder::PlaybackStrict(section, used);
            tRecorder::Record(section, used);
            static REAL usedAverage = 0;
            const REAL decay = .1;
            usedAverage = (usedAverage + used * decay) / (1 + decay);

            // disable pathfinding if it just takes too long.
            if (used > .06 || usedAverage > .03)
            {
#ifdef DEBUG
                con << "Path finding is too expensive for this PC. Disabling it.\n";
#endif
                sg_pathEnabled = false;
            }
            lastPath = se_GameTime();
        }

    if (!path.Valid())
    {
        data.thinkAgain = 1;
        return;
    }

    path.Proceed();

    bool goon = path.Proceed();
    bool nogood = false;

    do
    {
        if (goon)
            goon = path.GoBack();
        else
            goon = true;

        eCoord pos = path.CurrentPosition() + path.CurrentOffset() * 0.1f;
        eCoord opos = Object()->Position();
        eCoord odir = pos - opos;

        REAL forward = eCoord::F(odir, dir);
        if (forward < 0)
        {
            forward = 0;
        }
        eCoord intermediate = opos + dir * forward;

        gSensor p(Object(), opos, intermediate - opos);
        p.detect(.99f);
        nogood = p.ehit;

        if (!nogood)
        {
            gSensor p(Object(), intermediate, pos - intermediate);
            p.detect(.99f);
            nogood = p.ehit;
        }

    } while (goon && nogood);

    if (goon)
    {
        ThinkPathGiven(data);
    }
    else
    {
        lastPath -= 1;
        SwitchToState(AI_SURVIVE);
    }
}

// get rubber values in effect
void sg_RubberValues(ePlayerNetID const *player, REAL speed, REAL &max, REAL &effectiveness);

// find the most advanced path point that is in our viewing range:
void gAIPlayer::ThinkPathGiven(ThinkData &data, bool emergency)
{
    if (!path.Valid())
    {
        SwitchToState(AI_SURVIVE, 1);
        ThinkSurvive(data);
        return;
    }

    eCoord dir = Object()->Direction();

    int lr = 0;
    REAL mindist = 100;

    // now we have found our next goal. Try to get there.
    eCoord pos = Object()->Position();
    eCoord t = path.CurrentPosition();

    // to which side is the target?
    REAL side = (t - pos) * dir;
    REAL delay = Object()->GetTurnDelay();
    // already aligned? Advance.
    REAL sideThresh = delay * Object()->Speed() * .5;
    if (fabs(side) < sideThresh)
    {
        if (!path.Proceed())
        {
            SwitchToState(AI_SURVIVE, 1);
            ThinkSurvive(data);
            return;
        }
        t = path.CurrentPosition();
        side = (t - pos) * dir;
    }

    // look how far ahead the target is:
    REAL ahead = eCoord::F(t - pos, dir) + eCoord::F(path.CurrentOffset(), dir);

    if (ahead > (delay * .1 + sg_FrameTime()) * Object()->Speed() && !emergency)
    { // it is still before us. just wait a while.
        mindist = ahead;
    }
    else
    { // we have passed it. Make a turn towards it.
        if (fabs(side) < mindist)
        {
            mindist = fabs(side);
        }

        // switch to survival mode if cut off, no questions asked
        if (emergency && ahead > delay * Object()->Speed() && data.front.front.wallType == gSENSOR_ENEMY && state != AI_PATH_MINDLESS)
        {
            nextStateChange = 0;
            SwitchToState(AI_SURVIVE, 1);
        }

        {
#ifdef DEBUG
            con << "Following path...\n";
#endif
            lr += (side > 0 ? 1 : -1);
            if (ahead < -sideThresh * .9)
            {
                if (!path.Proceed())
                {
                    SwitchToState(AI_SURVIVE, 1);
                }
            }
        }
    }

    if (lr && state != AI_PATH_MINDLESS)
    {
        // don't commit suicide
        gAISensor const &side = lr > 0 ? data.left : data.right;
        REAL rubber, effectiveness;
        sg_RubberValues(this, Object()->Speed(), rubber, effectiveness);
        rubber -= Object()->GetRubber();
        if (side.distance < delay * Object()->Speed() - rubber * effectiveness)
        {
            EmergencySurvive(data);
            return;
        }
    }
    data.turn = -lr;

    if (target)
    {
        eCoord tDir = target->Position() - Object()->Position();

        REAL d = sqrt(tDir.NormSquared()) * .2f;
        if (d < mindist)
            mindist = d;
    }

    data.thinkAgain = mindist / Object()->Speed();
    if (data.thinkAgain > 1)
        data.thinkAgain *= .7;
}

void gAIPlayer::ThinkCloseCombat(ThinkData &data)
{
    int lr = 0;

    REAL nextThought = 0;

    const gAISensor *sides[2];
    sides[0] = &data.left;
    sides[1] = &data.right;

    eCoord dir = Object()->Direction();
    REAL fs = data.front.distance;
    //  REAL ls=left.hit;
    //  REAL rs=right.hit;

    gZone const *zoneTarget = dynamic_cast<gZone const *>(GetTarget());
    if (bool(target) && !zoneTarget && !IsTrapped(target, Object()) && nextStateChange < se_GameTime())
    {
        gSensor p(Object(), Object()->Position(), target->Position() - Object()->Position());
        p.detect(REAL(1));
        if (p.hit <= .999999) // no free line of sight to victim. Switch to path mode.
        {
            SwitchToState(sg_pathEnabled ? AI_PATH : AI_SURVIVE, 5);
            EmergencySurvive(data);

            return;
        }
    }

    REAL ed = 0;

    const REAL fear = REAL(.01);
    const REAL caution = .001;
    const REAL evasive = 100;
    const REAL attack = 100;
    const REAL seek = REAL(1);
    const REAL trap = REAL(.01);
    const REAL ffar = 20;
    //    const REAL close=1000;

    REAL random = 10 * Random() * Random();

    if (bool(target) && target->Alive())
    {

        eCoord enemypos = target->Position() - Object()->Position();
        eCoord enemydir = target->Direction();
        REAL enemyspeed = target->Speed();

        ed = REAL(fabs(enemypos.x) + fabs(enemypos.y));
        ed /= (enemyspeed + Object()->Speed());

        // transform coordinates relative to us:
        enemypos = enemypos.Turn(dir.Conj()).Turn(0, 1);
        enemydir = enemydir.Turn(dir.Conj()).Turn(0, 1);

        // now we are at the center of the coordinate system facing
        // in direction (0,1).

        // rules are symmetrical: exploit that.
        int side = 1;
        if (enemypos.x < 0)
        {
            side *= -1;
            enemypos.x *= -1;
            enemydir.x *= -1;

            sides[1] = &data.left;
            sides[0] = &data.right;
            //      Swap(ls,rs);
        }
        // now we can even assume the enemy is on our right side.
        /*
                if ( zoneTarget && idler.get() )
                {
                    if ( enemypos.y < 0 || ( data.front.front.wallType != gSENSOR_SELF && data.front.front.distance < Object()->Speed() * Object()->GetTurnDelay() ) )
                    {
                        gAINavigator::Wish wish( *idler );
                        wish.turn = -side;

                        // extra strong turn wish if wall is not mine
                        if( sides[1]->front.wallType != gSENSOR_SELF &&  sides[0]->front.wallType != gSENSOR_SELF )
                        {
                            wish.turn *= 2;
                        }

                        if ( enemypos.x > -enemypos.y * 2 )
                        {
                            wish.minDistance = Object()->ThisWallsLength()/2;
                        }
                        data.thinkAgain = idler->Activate( Object()->LastTime(), 0, 0, &wish );
                    }
                    else
                    {
                        data.thinkAgain = idler->Activate( Object()->LastTime(), 0, 0 );
                    }
                    return;
                }
        */
        // consider his ping and our reaction time
#define REACTION .2

        // REAL enemyspeed=target->speed;
        REAL ourspeed = Object()->Speed();

        REAL enemydist = target->Lag() * enemyspeed;

        // redo the prediction
#ifndef DEDICATED
        if (sn_GetNetState() == nCLIENT && !sr_predictObjects)
#endif
            enemypos = enemypos - enemydir * enemydist;
        enemydist += 2 * REACTION * enemyspeed;

        REAL ourdist = REACTION * ourspeed;
        ;

        // now we consider the worst case: we drive straight on,
        enemypos.y -= ourdist;
        // while the enemy cuts us: he goes in front of us
        REAL forward = -enemypos.y + .01;
        if (forward < 0) // no need to go to much ahead
            forward = 0;
        if (forward > enemydist)
            forward = enemydist;

        enemypos.y += forward;
        enemydist -= forward;

        // and then he turns left.
        enemypos.x -= enemydist;

        if (enemypos.y * enemyspeed > enemypos.x * ourspeed)
        { // he is right ahead of us.
            if (random < fear)
            { // evade him
#ifdef DEBUG
                con << "fear!\n";
#endif
                lr += side;

                nextThought += 1;
            }
            if (enemypos.y <= ffar &&
                ((enemydir.x < 0 && random < evasive) ||
                 (enemydir.y > 0 && random < caution) ||
                 (enemydir.y < 0 && random < attack)))
            {
#ifdef DEBUG
                con << "caution!\n";
#endif
                lr += side;

                nextThought += 1;

                if (enemyspeed > ourspeed)
                {
                    SetTraceSide(-side);
                    SwitchToState(AI_TRACE, 10);
                }
            }
        }
        else if (enemypos.y * ourspeed < -enemypos.x * enemyspeed)
        {

            REAL canCutIfDriveOn = enemypos.x * ourspeed - fs * (enemyspeed - ourspeed);
            canCutIfDriveOn -= enemypos.y * enemyspeed;

            REAL canCutIfAttack = -sides[1]->distance * enemyspeed - (sides[1]->distance - enemypos.x - enemypos.y * ourspeed) * ourspeed;

            if (random < attack && (!(data.front.Hit() && data.front.distance < 20) || canCutIfAttack > canCutIfDriveOn))
            {
#ifdef DEBUG
                con << "attack!\n";
#endif
                lr -= side;

                nextThought += 1;
            }
        }
        else if (enemypos.x > ffar * 4)
        {
            if (random < seek)
            {
#ifdef DEBUG
                con << "seek!\n";
#endif
                lr -= side;

                nextThought += 1;
            }
        }
        else if (enemypos.x < ffar * 2 && fabs(enemypos.y) < ffar)
        {
            if (random < trap)
            {
#ifdef DEBUG
                con << "trap!\n";
#endif
                lr += side;

                nextThought += 1;
            }
        }
    }

    if (!EmergencySurvive(data, 1, -lr))
        nextThought = 0;

    data.thinkAgain = ed + nextThought;
}

// **********
// * States *
// **********

//!@param player   the AI player the state belongs to
gAIPlayer::State::State(gAIPlayer &player)
    : parent_(player)
{
}

gAIPlayer::State::~State()
{
}

//!@return time in seconds until a new thought is needed
REAL gAIPlayer::State::Think(REAL maxStep)
{
    tASSERT(0);
    return 0;
}

//!@return the navigator to use
gAINavigator &gAIPlayer::State::Navigator()
{
    tASSERT(Parent().navigator_.get());
    return *(Parent().navigator_.get());
}

//!@return the navigator to use
gAINavigator const &gAIPlayer::State::Navigator() const
{
    tASSERT(Parent().navigator_.get());
    return *(Parent().navigator_.get());
}

//!@return the character
gAICharacter const &gAIPlayer::State::Character() const
{
    tASSERT(Parent().character_);
    return *Parent().character_;
}

gAIPlayer::StateGrind::StateGrind(gAIPlayer &player)
    : State(player)
{
}

gAIPlayer::StateGrind::~StateGrind() {}

REAL gAIPlayer::StateGrind::Think(REAL maxStep)
{
    Parent().SwitchToSurvival();
    /*

    REAL range = Delay() * Object()->Speed() * .5;

    if( data.front.front.wallType == gSENSOR_TEAMMATE )
    {
        if ( substate == AI_GRIND_GRIND )
        {
            if ( data.front.front.distance > range * .1 )
            {
                data.thinkAgain = data.front.front.distance/Object()->Speed();
                return;
            }
            else
            {
                data.turn = data.front.front.lr;
                data.thinkAgain = Object()->GetTurnDelay();
                return;
            }
        }
        else
        {
            data.turn = data.front.front.lr;
            data.thinkAgain = Object()->GetTurnDelay();
            state = AI_SURVIVE;
            return;
        }
    }

    if( data.left.front.wallType == gSENSOR_TEAMMATE )
    {
        if ( data.left.front.distance > range )
        {
            substate = AI_GRIND_GRIND;
            data.turn = -1;
            data.thinkAgain = data.left.front.distance/Object()->Speed();
            return;
        }
        else if ( state == AI_GRIND )
        {
            substate = AI_GRIND_SPLIT_RIGHT;
            return;
        }
    }

    if( data.right.front.wallType == gSENSOR_TEAMMATE )
    {
        if ( data.right.front.distance > range )
        {
            substate = AI_GRIND_GRIND;
            data.turn = 1;
            data.thinkAgain = data.right.front.distance/Object()->Speed();
            return;
        }
        else if ( state == AI_GRIND )
        {
            substate = AI_GRIND_SPLIT_LEFT;
            return;
        }
    }

    // SPLIT!
    if( substate == AI_GRIND_SPLIT_LEFT )
    {
        if ( data.right.front.wallType != gSENSOR_TEAMMATE )
        {
            data.thinkAgain = data.left.front.distance/Object()->Speed();
            data.turn = -1;
        }
        else
        {
            data.thinkAgain = Delay() * 2;
            return;
        }
    }

    if( substate == AI_GRIND_SPLIT_RIGHT )
    {
        if( data.left.front.wallType != gSENSOR_TEAMMATE )
        {
            data.thinkAgain = data.right.front.distance/Object()->Speed();
            data.turn = 1;
        }
        else
        {
            data.thinkAgain = Delay() * 2;
            return;
        }
    }

    SwitchToState( AI_SURVIVE );
    data.thinkAgain = .5;

    */

    return 0;
}

void gAIPlayer::RightBeforeDeath(int triesLeft) // is called right before the vehicle gets destroyed.
{
    // sadly, we have no energy for this
    if (triesLeft > concentration_)
    {
        return;
    }

    if (nCLIENT == sn_GetNetState())
        return;

    if (simpleAI_)
        return;

    if (!character_)
    {
        st_Breakpoint();
        return;
    }

    CreateNavigator();

    gRandomController random(randomizer_);

    // think again immediately after this
    nextTime_ = lastTime_;

#ifdef DEBUG_X
    if (log && !Object()->Alive())
    {
        log->Print();
        //      st_Breakpoint();
        delete log;
        log = NULL;
    }
#endif

    if (!Object()->Alive() || (character_ && Random() * 10 > character_->properties[AI_EMERGENCY]))
        return;

    if (triesLeft <= 0)
    {
        gAINavigator::SuicideEvaluator::SetEmergency(true);
    }

    Think(0);

    gAINavigator::SuicideEvaluator::SetEmergency(false);
}

gAIPlayer *sg_watchAI = 0;

void gAIPlayer::NewObject() // called when we control a new object
{
    lastTime_ = 0;

    navigator_.reset();

    if (character_)
    {
        nextTime_ = character_->properties[AI_STARTSTRAIGHT] * gArena::SizeMultiplier() / gCycleMovement::SpeedMultiplier();
        switch (character_->properties[AI_STARTSTATE])
        {
        default:
            state_ = tNEW(gStateSurvive)(*this);
            break;
        }
    }
    else
    {
        nextTime_ = 10;
        state_ = tNEW(gStateSurvive)(*this);
    }

    if (TeamListID() > 0)
    {
        state_ = tNEW(StateGrind)(*this);
        nextTime_ = Delay() * 2 + .1;
    }

    nextTime_ = 0;
    ClearTarget();
}

static gAISensor *sg_GetSensor(int currentDirectionNumber, gCycle const &object, int turn, REAL side, REAL range, REAL corridor, REAL &mindist)
{
    // determine the current direction
    eGrid &grid = *object.Grid();
    eCoord origDir = grid.GetDirection(currentDirectionNumber);

    // determine sensors
    gAISensor *ret = 0;

    // turn direction
    int direction = currentDirectionNumber;
    int turns = 1;
    grid.Turn(direction, turn);
    eCoord dir = grid.GetDirection(direction);
    while (!ret || (turn * (origDir * dir) > .01 && currentDirectionNumber != direction))
    {
        // cast rays
        gAISensor *sensor = tNEW(gAISensor)(&object, object.Position(), dir, side, range, corridor * .5f, turn);
        if (!ret || sensor->distance > ret->distance)
        {
            if (ret)
            {
                // calculate effective distance required to turn around in time
                REAL dist = ret->distance - 1.2 * object.Speed() * Delay() * turns;
                if (mindist > dist)
                    mindist = dist;
            }

            delete ret;
            ret = sensor;
        }
        else
        {
            delete sensor;
        }

        // go on turning
        grid.Turn(direction, turn);
        dir = grid.GetDirection(direction);
        ++turns;
    }

    return ret;
}

void gAIPlayer::CreateNavigator()
{
    gCycle *cycle = Object();
    if (cycle && !navigator_.get())
    {
        navigator_.reset(tNEW(gAINavigator(cycle)));
    }
}

#ifdef DEBUG
// static int sg_breakOnThought = 0;
// class tCommandLineBreakOnThought: public tCommandLineAnalyzer
// {
//     virtual bool DoAnalyze( tCommandLineParser & parser )    //! Analyzes the command line option
//     {
//         tString value;
//         if( parser.GetOption( value, "--breakonthought" ) )
//         {
//             sg_breakOnThought = value.toInt();

//             return true;
//         }
//         return false;
//     }

//     virtual void DoHelp( std::ostream & s )                  //! Prints option help
//     {
//         s << "--breakonthought <thought>   : sets a breakpoint on the given thought of the AI (the static variable 'count' in gAIBase::Think())\n\n";
//     }
// };
// static tCommandLineBreakOnThought sg_breakOnThoughtCommandLine;
#endif

REAL gAIPlayer::Think(REAL maxStep)
{
    concentration_ -= 1;

    // con << "Think!\n";

    // if ( !simpleAI_ )
    // {
    //     gSimpleAIFactory * factory = gSimpleAIFactory::Get();
    //     if ( factory )
    //     {
    //         simpleAI_ = factory->Create( Object() );
    //     }
    // }

    // CreateNavigator();

    // if ( simpleAI_ )
    // {
    //     return simpleAI_->Think( maxStep );
         
    // }


    REAL nextThink = 10;

    if (GetTarget() && (!GetTarget()->Alive() || IsTrapped(GetTarget(), Object())))
    {
        // no longer makes sense to chase
        ClearTarget();
    }

    if (!Object() || !Object()->Alive())
        return 100;

    emergency = false;
    //  return 1;

    // first, find close eWalls and evade them.
    REAL delay = Delay();
    REAL speed = Object()->Speed();
    REAL range = speed;
    eCoord dir = Object()->Direction();
    REAL side = speed * delay;

    REAL corridor = range;
    if (corridor < side * 2)
        corridor = side * 2;

    gAISensor front(Object(), Object()->Position(), dir, side * 2, range, corridor, 0);

    // get the sensors to the left and right with the most free space
    int currentDirectionNumber = Object()->Grid()->DirectionWinding(dir);
    REAL mindistLeft = 1E+30, mindistRight = 1E+30;
    std::unique_ptr<gAISensor> left(sg_GetSensor(currentDirectionNumber, *Object(), -1, side, range, corridor, mindistLeft));
    std::unique_ptr<gAISensor> right(sg_GetSensor(currentDirectionNumber, *Object(), 1, side, range, corridor, mindistRight));

    // count intermediate walls to the left and right as if they were in front
    {
        REAL mindistFront = mindistLeft > mindistRight ? mindistLeft : mindistRight;
        if (mindistFront < front.distance)
        {
            front.distance = mindistFront;
        }
    }

    // switch to survival state if our victim died:
    if (state == AI_CLOSECOMBAT && (!target || !target->Alive()))
        SwitchToState(AI_SURVIVE, 1);
    /*
    {
        eDebugLine::SetTimeout(.5);
        eDebugLine::SetColor(0, 1, 0);
        eCoord p = Object()->Position();
        eDebugLine::Draw(p, .5, p, 5.5);
        eDebugLine::SetTimeout(0);
    }
    */

    triesLeft = 10;

    REAL ret = 1;

    if (target)
        state = AI_PATH;
    // not the best solution, but still better than segfault...
    if (left.get() != 0 && right.get() != 0)
    {
        ThinkData data(front, *left, *right);
        switch (state)
        {
        case AI_SURVIVE:
            ThinkSurvive(data);
            break;
        case AI_PATH:
            ThinkPath(data);
            break;
        case AI_PATH_MINDLESS:
        case AI_PATH_GIVEN:
            ThinkPathGiven(data);
            break;
        case AI_TRACE:
            ThinkTrace(data);
            break;
        case AI_CLOSECOMBAT:
            ThinkCloseCombat(data);
            break;
        }
        ActOnData(data);
        ret = data.thinkAgain;
        return nextThink;
    }
    else if (state_)
    {
        tJUST_CONTROLLED_PTR<State> keepalive(state_);
        return state_->Think(maxStep);
    }
    else
        return 10;
}

void gAIPlayer::Timestep(REAL time)
{

    if (!character_)
    {
        st_Breakpoint();
        con << "NO CHARACTER?\n";
        return;
    }

    const REAL relax = 3;

    // don't think if the object is not up to date
    if (Object() && Object()->LastTime() < time - EPS)
        return;

    REAL ts = time - lastTime_;
    lastTime_ = time;

    concentration_ += (character_->properties[AI_REACTION] + 1) * ts;
    concentration_ = concentration_ / (1 + ts / relax);
    if (concentration_ < 0)
    {
        nextTime_ = time - concentration_ + .01;
        //return;
    }

    // if (bool(Object()) && Object()->Alive() && nextTime_<time)
    {
        gRandomController random(randomizer_);

        REAL target = (character_->properties[AI_REACTION] + 1);
        REAL safethought = 4 / (target + 3 * concentration_);

        REAL nextthought = Think(safethought);
        //    if (nextthought>.9) nextthought=REAL(.9);

        if (nextthought < -concentration_)
        {
            nextthought = -concentration_;
        }
        nextTime_ = nextTime_ + nextthought;

        // con << concentration_ << "\t" << nextthought << '\t' << ts << '\n';
    }
}

void gAIPlayer::Color(REAL &a_r, REAL &a_g, REAL &a_b) const
{
    ePlayerNetID::Color(a_r, a_g, a_b);
}

gAIPlayer::~gAIPlayer()
{
    target_ = NULL;
    ClearObject();
    tCHECK_DEST;
}

void gAIPlayer::ClearAll()
{
    sg_AIReferences.ReleaseAll();

    // remove empty AI team
    if (0 == AITeam()->NumPlayers())
    {
        ClearAITeam();
    }
}

#define DANGERLEVELS 4
#define LOOPLEVEL 0
#define SPACELEVEL 1
#define TRAPLEVEL 2
#define COLIDELEVEL 2
#define TEAMLEVEL 3

class gAILogEntry
{
public:
    int sideDanger[DANGERLEVELS][2];
    int frontDanger[DANGERLEVELS];
    int turn;
    int tries;
    REAL time;
};

#define ENTRIES 10

class gAILog
{
public:
    gAILogEntry entries[ENTRIES + 1];
    int current;
    int del;

    gAILog() : current(0), del(0) {}

    void DeleteEntry()
    {
        del = 1;
        if (current > 0)
            current--;
    }

    gAILogEntry &NextEntry()
    {
        del = 0;

        if (current >= ENTRIES)
        {
            for (int i = 1; i < ENTRIES; i++)
                entries[i - 1] = entries[i];
        }
        else
            current++;

        gAILogEntry &ret = entries[current - 1];
        ret.time = se_GameTime();
        return ret;
    }

    void Print()
    {
#ifdef DEBUG
        con << "Log:\n";
        for (int i = current + del - 1; i >= 0; i--)
        {
            for (int j = 0; j < DANGERLEVELS; j++)
            {
                con << entries[i].sideDanger[j][0] << ' ';
                con << entries[i].frontDanger[j] << ' ';
                con << entries[i].sideDanger[j][1] << "    ";
            }
            con << entries[i].turn << ", " << entries[i].tries << "\n";
        }
#ifndef DEDICATED
        //		se_PauseGameTimer(true);
#endif
#endif
    }
};

// emergency functions:
bool gAIPlayer::EmergencySurvive(ThinkData &data, int enemyevade, int preferedSide)
{
    if (!character_)
    {
        st_Breakpoint();
        return false;
    }

    gAISensor const &front = data.front;
    gAISensor const &left = data.left;
    gAISensor const &right = data.right;

    if (!log)
        log = tNEW(gAILog);

    triesLeft = (triesLeft * character_->properties[AI_EMERGENCY]) / 10;

    freeSide *= .95;

    int i, j;

    // don't do a thing if there may be a better way out of we drive on:
    if (triesLeft > 0 &&
        front.front.otherCycle &&
        front.front.otherCycle != Object() &&
        ((front.frontLoop[1].loop && front.front.otherCycle != left.front.otherCycle && left.front.otherCycle) ||
         (front.frontLoop[0].loop && front.front.otherCycle != right.front.otherCycle && right.front.otherCycle)))
        return false;

    // get the delay between two turns
    REAL delay = Delay();
    REAL range = Object()->Speed() * delay;

    // nothing we can do if we cannot make a turn immediately
    if (!Object()->CanMakeTurn(1) || !Object()->CanMakeTurn(-1))
        return false;

    //  bool dontCheckForLoop[2] = { false, false };

    // look out if there is anything bad going on in one of the directions:
    // [signifficance: danger level of n: You'll be (as good as) dead in [10/n delay times] if you drive that way
    int sideDanger[DANGERLEVELS][2];
    int frontDanger[DANGERLEVELS];
    for (i = DANGERLEVELS - 1; i >= 0; i--)
    {
        sideDanger[i][0] = 0;
        sideDanger[i][1] = 0;
        frontDanger[i] = 0;
    }

    bool canTrapEnemy = false;

    if (emergency)
    {
        frontDanger[SPACELEVEL] += 40;
    }

    const gAISensor *sides[2];
    sides[0] = &left;
    sides[1] = &right;

    // avoid loops:

    bool isTrapped = IsTrapped(Object(), NULL);

    /*
      if (front.front.wallType == gSENSOR_ENEMY)
      sideDanger[LOOPLEVEL][(1-front.front.lr*enemyevade) >> 1] += 5;
    */

    if (!isTrapped)
        for (i = 1; i >= 0; i--)
        {
            if (front.frontLoop[i].loop && front.distance < 5 * sides[i]->distance)
            {
                // if we would close ourself in, make the danger bigger
                if (front.front.otherCycle == Object() && i + i - 1 == front.front.lr)
                    sideDanger[LOOPLEVEL][i] += 40;

                sideDanger[LOOPLEVEL][i] += 40;
                for (j = front.frontLoop[i].closedIn.Len() - 1; j >= 0; j--)
                    if (front.frontLoop[i].closedIn(j) == target)
                        canTrapEnemy = true;
            }

            for (j = 1; j >= 0; j--)
                if (front.sideLoop[i][j].loop)
                    sideDanger[LOOPLEVEL][j]++;

            // if we would close ourselfs in by a zigzag in direction i,
            // but not by a u-turn and there is enough space for a u-turn,
            // do it.
            if (sides[i]->frontLoop[1 - i].loop &&
                !sides[i]->frontLoop[i].loop)
            {
                if (sides[i]->distance > range)
                {
                    frontDanger[LOOPLEVEL] += 20;
                    sideDanger[LOOPLEVEL][1 - i] += 10;
                }
                else // try to make some room so we can evade:
                {
                    frontDanger[LOOPLEVEL] += 20;

                    sideDanger[LOOPLEVEL][i] += 10;
                }
            }

            // if we would close ourselves in by a U-Turn, don't do it.
            //	if (sides[i]->frontLoop[i].loop && sides[i].distance < range * 2)
            //	  sideDanger[LOOPLEVEL][i] += 40;
        }

    // try to trap the enemy
    if (character_->properties[AI_LOOP] >= 10 && canTrapEnemy && !emergency)
        return false;

    /*
      // avoid closing yourself or a teammate in.
      if (front.type == gSENSOR_SELF || front.type == gSENSOR_TEAMMATE)
      {
         if (front.lr > 0)
      sideDanger[][1] +=2;
         else
      sideDanger[][0] +=2;
      }
    */

    {
        if (front.Hit() &&
            (front.distance + range < sides[0]->distance ||
             front.distance + range < sides[1]->distance))
        {
            if (front.front.wallType == gSENSOR_RIM)
                frontDanger[SPACELEVEL] += static_cast<int>(100 * range * gArena::SizeMultiplier() / (front.distance + range * .1));

            frontDanger[SPACELEVEL] += static_cast<int>(5 * range / (front.distance + range * .2));

            if (front.distance < range)
                frontDanger[SPACELEVEL] += static_cast<int>(20 * range / (front.distance + range * .2)) + 1;
        }

        // avoid close corners:
        for (i = 1; i >= 0; i--)
        {
            if (sides[i]->Hit() && // sides[i]->distance < range * 3 &&
                sides[i]->distance < front.distance + range)
            {
                if (sides[i]->front.wallType == gSENSOR_RIM)
                    sideDanger[SPACELEVEL][i] += static_cast<int>(150 * range * gArena::SizeMultiplier() / (sides[i]->distance + range * .1));

                sideDanger[SPACELEVEL][i] += static_cast<int>(range * 5 / (sides[i]->distance + range * .1));

                if (sides[i]->distance < range)
                    sideDanger[SPACELEVEL][i] += static_cast<int>(range * 20 / (sides[i]->distance + range * .1));
            }

            // give us a chance to turn around:
            if (frontDanger[SPACELEVEL] * 2 < sideDanger[SPACELEVEL][i])
                sideDanger[LOOPLEVEL][i - i] -= sideDanger[SPACELEVEL][i] * 2;
        }
    }

    // avoid close proximity to other cycles
    const gCycle *target = NULL;
    const tList<eGameObject> &gameObjects = Object()->Grid()->GameObjects();
    gCycle *secondbest = NULL;
    REAL closest = 1000000;
    eCoord dir = Object()->Direction();

    // find the closest enemy
    for (i = gameObjects.Len() - 1; i >= 0; i--)
    {
        gCycle *other = dynamic_cast<gCycle *>(gameObjects(i));

        if (other && other->Alive() && other != Object())
        {
            eCoord otherpos = other->Position() - Object()->Position();
            REAL otherNorm = otherpos.NormSquared();

            bool nothit = false;
            if (otherNorm < closest * 4)
            {
                gSensor p(other, other->Position(), -otherpos);
                p.detect(REAL(.9999));
                gSensor q(Object(), Object()->Position(), otherpos);
                q.detect(REAL(.9999));

                nothit = p.hit >= .999 && q.hit >= .999;
            }

            if (other->Team() != Object()->Team())
            {
                // then, enemy is realy an enemy
                //      REAL s = Object()->Speed() * 50;
                if (/* otherNorm < s*s && */ otherNorm < closest)
                {
                    // check if the path is clear
                    secondbest = dynamic_cast<gCycle *>(other);
                    if (nothit)
                    {
                        closest = otherNorm;
                        target = secondbest;
                    }
                }
            }
            else if (nothit)
            {
                // he is a teammate. Avoid him.

                eCoord friendpos = other->Position() - Object()->Position();

                // transform coordinates relative to us:
                friendpos = friendpos.Turn(dir.Conj()).Turn(0, 1);

                if (friendpos.y > fabs(friendpos.x) * 1.5f)
                    frontDanger[TEAMLEVEL] += 10;
                if (friendpos.x * 2 > -friendpos.y)
                    sideDanger[TEAMLEVEL][1] += 10;
                else if (-friendpos.x * 2 > -friendpos.y)
                    sideDanger[TEAMLEVEL][0] += 10;
            }
        }
    }

    //  if (!target)
    // target = secondbest;

    if (target && character_->properties[AI_ENEMY] > 0)
    {
        eCoord enemypos = target->Position() - Object()->Position();
        eCoord enemydir = target->Direction();
        REAL enemyspeed = target->Speed();

        // transform coordinates relative to us:
        enemypos = enemypos.Turn(dir.Conj()).Turn(0, 1);
        enemydir = enemydir.Turn(dir.Conj()).Turn(0, 1);

        if (character_->properties[AI_ENEMY] > 7)
        {
            // would he be able to trap us if we drive straight on?
            // bool trap[2] = {false, false};

            if (!isTrapped)
                for (i = 1; i >= 0; i--)
                {
                    // if the enemy comes racing towards us, check if he could
                    // close us in by touching our own line ON THE OPPOSITE side of i
                    tArray<const gCycle *> closedIn;
                    int winding = 0;

                    bool loop = CheckLoop(target, Object(),
                                          Object()->GetDistance() + 4 * TOL, i, 0,
                                          closedIn, winding);

                    winding -= Object()->WindingNumber();
                    winding += target->WindingNumber();

                    // yes! we shoult turn in direction 1-i to get the target
                    // to the other side.
                    if (loop)
                        if (winding * (i + i - 1) < 0)
                        {
                            // trap[i] = true;
                            REAL x = enemypos.x * (i + i - 1);
                            REAL y = enemypos.y;

                            bool canAccelerateByTurning =
                                (sides[1 - i]->Hit() &&
                                 sides[1 - i]->distance < Object()->Speed() * delay * 5 &&
                                 sides[i - i]->distance > Object()->Speed() * delay &&
                                 !sides[i - i]->frontLoop[i].loop);

                            bool ohShit = target->Speed() > Object()->Speed() + sqrt(closest);

                            if (ohShit)
                            {
                                SetTraceSide(-(i + i - 1));
                                SwitchToState(AI_TRACE, 10);
                            }

                            bool turningIsFutile =
                                front.front.otherCycle == Object() &&
                                sides[1 - i]->front.otherCycle == Object() &&
                                front.distance < sides[1 - 1]->distance * 10;

                            if (
                                x < 0 &&
                                (x * Object()->Speed() < -y * target->Speed() + 1000 ||
                                 canAccelerateByTurning || ohShit) &&
                                !turningIsFutile)
                            {
                                if (enemydir.y < -.2f && y < 0)
                                    SetTraceSide(-(i + i - 1));

                                frontDanger[TRAPLEVEL] += 10;
                            }

                            if (y > 0 || x < 0 || ohShit
                                //			   ( y * Object()->Speed() > x * target->Speed()*.9 - 200 || enemyspeed.x * (i+i-1)
                            )
                                sideDanger[TRAPLEVEL][i] += 20;
                            // sideDanger[TRAPLEVEL][i] ++;
                        }
                }
        }

        if (character_->properties[AI_ENEMY] > 0)
        {
            // imminent collision check
            REAL totalspeed = enemyspeed + Object()->Speed();

            if ((fabs(enemypos.y) < totalspeed * .3f && fabs(enemypos.x) < totalspeed * .3f))
            {
                REAL diffSpeed = -enemydir.y * enemyspeed + Object()->Speed();
                if (diffSpeed > 0 && enemydir.y <= .2)
                {
                    REAL enemyFront = enemypos.y / diffSpeed;
                    REAL enemySide = fabs(enemypos.x) / diffSpeed;
                    if (enemyFront > 0 && enemyFront < .4 + enemySide && fabs(enemypos.y) > fabs(enemypos.x))
                    {
                        frontDanger[COLIDELEVEL] += 1 + int(4 / (enemyFront + .01));
                        //		      SwitchToState( AI_SURVIVE, enemyFront * 4 + 2 );
                    }
                }

                int side = enemypos.x > 0 ? 1 : 0;

                // can we cut him instead of evade him?
                if (Object()->Team() != target->Team() &&
                    ((enemydir.y <= -.2 &&
                      enemypos.y * target->Speed() * 1.1 > fabs(enemypos.x) * Object()->Speed()) ||
                     sideDanger[COLIDELEVEL][side] > 0))
                    sideDanger[COLIDELEVEL][1 - side] += 5;
                else if (-(enemypos.y + .3f) * Object()->Speed() < fabs(enemypos.x) * target->Speed() * 1.2)
                    sideDanger[COLIDELEVEL][side] += 10;
            }
        }
    }
    /*
    eDebugLine::SetTimeout(.5);
    eDebugLine::SetColor(1, 0, 1);
    eCoord p = Object()->Position();
    eDebugLine::Draw(p, .5, p, 8.5);
    eDebugLine::SetTimeout(0);
    */
    // determine the total danger levels by taking the max of the individual experts:
    int fDanger = 0;
    int sDanger[2] = {0, 0};
    for (i = 0; i < DANGERLEVELS; i++)
    // for (i = 1; i< 2; i++)
    {
        if (!fDanger || frontDanger[i] > fDanger + 2)
            fDanger = frontDanger[i];

        for (int j = 1; j >= 0; j--)
            if (!sDanger[j] || sideDanger[i][j] > sDanger[j] + 2)
                sDanger[j] = sideDanger[i][j];
    }

    // nothing to do if we are not in immediate danger.
    if (!fDanger && !preferedSide)
        return false;

    // decide about your direction:
    int turn = 0;

    turn += sDanger[0];
    turn -= sDanger[1];

    if (!turn)
        turn = (int)freeSide;

    if (!turn && front.front.wallType != gSENSOR_RIM)
        turn = front.front.lr * enemyevade;

    if (!turn)
        turn = (sides[0]->distance > sides[1]->distance ? -1 : 1);

    if (!turn && log->current)
        turn = log->entries[log->current - 1].turn;

    // switch to survival mode if we just trapped an enemy
    if (canTrapEnemy)
    {
#ifdef DEBUG
        if (!tRecorder::IsRunning())
            Chat(tString("Hehe! Got you!"));
#endif
        SwitchToState(AI_TRACE, 10);

        if (turn)
            this->SetTraceSide(-turn);
    }

    gAILogEntry &e = log->NextEntry();
    e.turn = 0;
    e.tries = triesLeft;
    for (i = DANGERLEVELS - 1; i >= 0; i--)
    {
        e.frontDanger[i] = frontDanger[i];
        e.sideDanger[i][0] = sideDanger[i][0];
        e.sideDanger[i][1] = sideDanger[i][1];
    }

    int side = 1;
    if (preferedSide < 0)
    {
        for (i = DANGERLEVELS - 1; i >= 0; i--)
        {
            int dSwap = sideDanger[i][0];
            sideDanger[i][0] = sideDanger[i][1];
            sideDanger[i][1] = dSwap;
        }

        int dSwap = sDanger[0];
        sDanger[0] = sDanger[1];
        sDanger[1] = dSwap;

        sides[1] = &left;
        sides[0] = &right;
        side = -1;
        preferedSide = 1;
    }

    // no problem in the preferred direction. Just take it.
    if (preferedSide)
    {
        if (fDanger * 3 >= sDanger[1] * 2 - 5 &&
            sDanger[0] * 3 >= sDanger[1] * 2 - 5)
        {
            freeSide -= side * 100;
            e.turn = side;
            data.turn = side;
            return true;
        }

        if (fDanger * 2 <= sDanger[0] * 3 + 3)
        {
            log->DeleteEntry();
            return false;
        }
    }

    // it is safer driving straight on
    if (fDanger <= sDanger[0] + 3 && fDanger <= sDanger[1] + 3 && fDanger < 20)
    {
        log->DeleteEntry();
        return false;
    }

    if (turn)
    {
        freeSide -= side * 100;
        e.turn = turn;
        data.turn = turn;
    }
    else
        log->DeleteEntry();

    return turn;

    //eDebugLine::SetTimeout(0);
}

void gAIPlayer::EmergencyTrace(ThinkData &data)
{
    EmergencySurvive(data, -1, -traceSide);
}

void gAIPlayer::EmergencyPath(ThinkData &data)
{
    ThinkPathGiven(data, true);
}

void gAIPlayer::EmergencyCloseCombat(ThinkData &data)
{
    EmergencySurvive(data);

    /*
      int dir = 0;

      if (target)
      {
         eCoord enemyPos = target->Position() - Object()->Position();
         eCoord dirRel   = Object()->Direction();
         if (enemyPos * dirRel < 0)
      dir --;
         else
      dir ++;
      }


      EmergencySurvive(front, left, right, 1, dir);
    */
}

void gAIPlayer::ActOnData(ThinkData &data)
{
    // delegate
    ThinkDataBase &base = data;
    ActOnData(base);

    // sanitize next think time so it will be before we hit the next wall
    if (Object()->Speed() > 0 && data.thinkAgain > 0)
    {
        gSensor front(Object(), Object()->Position(), Object()->Direction());
        front.detect(Object()->Speed() * data.thinkAgain * 1.5);
        if (front.ehit)
        {
            REAL thinkAgain = (front.hit / Object()->Speed()) * .8;
            if (data.thinkAgain > thinkAgain)
                data.thinkAgain = thinkAgain;
        }
    }
}

void gAIPlayer::ActOnData(ThinkDataBase &data)
{
    // archive decision
    ThinkDataBase copy = data;

    // execute turn
    if (data.turn)
    {
        if (Object()->CanMakeTurn(data.turn))
        {
            Object()->Turn(data.turn);
        }
        else
        {
            data.thinkAgain = Object()->GetNextTurn(data.turn) - Object()->LastTime();
        }
    }
}

/*
void gAIPlayer::AddRef()
{
    ePlayerNetID::AddRef();
}

void gAIPlayer::Release()
{
    ePlayerNetID::Release();
}
*/
