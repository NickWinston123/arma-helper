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

#ifndef ArmageTron_AIBASE_H
#define ArmageTron_AIBASE_H

#include "rSDL.h"

#include "eTimer.h"
#include "ePath.h"
#include "ePlayer.h"
#include "eTeam.h"
#include "gCycle.h"
#include "tList.h"
#include "tRandom.h"
#include "nObserver.h"
#include "tReferenceHolder.h"
#include "gHelper/gHelper.h"

#include <vector>



#define DANGERLEVELS 4
#define LOOPLEVEL   0
#define SPACELEVEL  1
#define TRAPLEVEL   2
#define COLIDELEVEL 2
#define TEAMLEVEL   3


class gAIPlayer;
struct ThinkData;
struct ThinkDataBase;
class gSensor;
class gAILog;
class gAICharacter;

extern REAL Delay();
extern REAL Random();



// an instance of this class will prevent deterministic random lookups
class gRandomController
{
public:
    static gRandomController * random_;
    gRandomController * lastRandom_;
    tRandomizer & randomizer_;

    gRandomController( tRandomizer & randomizer = tRandomizer::GetInstance() )
            : lastRandom_( random_ ), randomizer_( randomizer )
    {
        random_ = this;
    }

    ~gRandomController()
    {
        random_ = lastRandom_;
    }
};


class gAILogEntry{
public:
    int sideDanger[DANGERLEVELS][2];
    int frontDanger[DANGERLEVELS];
    int turn;
    int tries;
    REAL time;
};

#define ENTRIES 10

class gAILog{
public:
    gAILogEntry entries[ENTRIES+1];
    int         current;
    int         del;

    gAILog():current(0), del(0){}

    void DeleteEntry()
    {
        del = 1;
        if (current > 0)
            current--;
    }

    gAILogEntry& NextEntry()
    {
        del = 0;

        if (current >= ENTRIES)
        {
            for (int i=1; i<ENTRIES; i++)
                entries[i-1] = entries[i];
        }
        else
            current++;

        gAILogEntry& ret = entries[current-1];
        ret.time = se_GameTime();
        return ret;
    }

    void Print()
    {
#ifdef DEBUG
        con << "Log:\n";
        for (int i = current + del - 1; i>=0; i--)
        {
            for (int j=0; j < DANGERLEVELS; j++)
            {
                con << entries[i].sideDanger[j][0] << ' ';
                con << entries[i].frontDanger[j]   << ' ';
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


// data about a loop
class gLoopData{
public:
    bool loop;                       // is there a loop?
    int  winding;                    // in what direction does it go?
    tArray<const gCycle*>closedIn;   // which cycles are closed in?

    gLoopData():loop(false), winding(0){}
    void AddCycle(const gCycle* c){closedIn[closedIn.Len()] = c;}
};



// hit data
class gHitData{
public:
    const eHalfEdge*  edge;      // edge we hit
    gSensorWallType   wallType;  // type of the wall hit
    int               lr;        // does the wall go left or right?
    REAL              distance;  // distance to the wall

    // additional info if the wall that got hit is a cycle wall
    gCycle         *otherCycle;    // the cylce that the hit wall belongs to
    REAL            driveDistance; // the distance it had travelled when it was at the place we hit
    int             windingNumber; // the winding number at the place hit

    gHitData():edge(NULL), wallType(gSENSOR_NONE), otherCycle(NULL){}

    bool Hit() const {return edge;}

    void AddHit(const eCoord& origin, const eCoord& dir, const gSensor& sensor, int winding);
};


class gAISensor
{
public:
    gCycle* cycle;
    bool     hit;
    gHitData front;
    gHitData sides[2];
    gLoopData frontLoop[2];
    gLoopData sideLoop[2][2];
    REAL distance;

    bool Hit() const;
    void DetectLoop(const gHitData& hit, gLoopData loopData[2]);

    gAISensor(const gAIPlayer *ai, gCycle* c,
              const eCoord& start, const eCoord& dir,
              REAL sideScan,
              REAL frontScan,
              REAL corridorScan,
              int winding
             );

    gAISensor( gCycle* c,
               const eCoord & start,  const eCoord & dir,
              REAL sideScan,
              REAL frontScan,
              REAL corridorScan,
              int winding
             );
};
extern gAISensor * sg_GetSensor( gAIPlayer const * ai, int currentDirectionNumber, gCycle const & object, int turn, REAL side, REAL range, REAL corridor, REAL & mindist );

extern gAISensor * sg_GetSensor(int currentDirectionNumber, gCycle & object, int turn, REAL side, REAL range, REAL corridor, REAL & mindist );


// data structure common to thinking functions
struct ThinkDataBase
{
    int turn;                                   // direction to turn to
    REAL thinkAgain;                            // when to think again

    ThinkDataBase()
            : turn(0), thinkAgain(0)
    {
    }
};

struct ThinkData : public ThinkDataBase
{
    gAISensor const & front;                    // sensors cast by upper level function
    gAISensor const & left;
    gAISensor const & right;

    ThinkData( gAISensor const & a_front, gAISensor const & a_left, gAISensor const & a_right )
            : front(a_front), left( a_left ), right( a_right )
    {
    }
};

typedef enum
{ AI_SURVIVE = 0,   // just try to stay alive
  AI_TRACE,         // trace a wall
  AI_PATH,          // follow a path to a target
  AI_CLOSECOMBAT,   // try to frag a nearby opponent
  AI_ROUTE          // follow a route (a set of coord)
}
gAI_STATE;



extern bool IsTrapped(const gCycle *trapped, const gCycle *other);
extern bool CheckLoop(const gCycle *a, const gCycle *b,
                      REAL bDist, int bSide, int dir,
                      tArray<const gCycle*>& closedIn, int& winding,
                      REAL aEndDist = 0, int aEndSides = 3, int aEndDir = 1 );

class gAIPlayer: public ePlayerNetID{
    friend class gAITeam;
    friend class gSmarterBot;
    tReproducibleRandomizer randomizer_;
public:
    gAIPlayer(gHelper *helper, gCycle *cycle);
    static gAIPlayer &Get(gHelper *helper, gCycle *cycle);
    gAICharacter*           character; // our specification of abilities

    // for all offensive modes:
    nObserverPtr< gCycle >    target;  // the current victim
    gCycle *owner_;
    gHelper *helper_;
    bool helperAI;
    // for pathfinding mode:
    ePath                   path;    // last found path to the victim
    REAL lastPath;                   // when was the last time we did a pathsearch?

    // for route mode:
        std::vector<eCoord> route_;
        unsigned int lastCoord_;
        tJUST_CONTROLLED_PTR<eFace> targetCurrentFace_;

    // for trace mode:
    int  traceSide;
    REAL lastChangeAttempt;
    REAL lazySideChange;

    // state management:
    gAI_STATE state;             // the current mode of operation
    REAL      nextStateChange;   // when is the operation mode allowed to change?

    bool emergency;              // tell if an emergency is present
    int  triesLeft;              // number of tries left before we die

    REAL freeSide;               // number that tells which side is probably free for evasive actions

    // basic thinking:
    REAL lastTime;
    REAL nextTime;

    REAL concentration;

    // log
    gAILog* log;

    //  gCycle * Cycle(){return object;}

    // set trace side:
    virtual void SetTraceSide(int side);

public:
    // state change:
    void SwitchToState(gAI_STATE nextState, REAL minTime=10);

    // state update functions:
    virtual void ThinkSurvive( ThinkData & data );
    virtual void ThinkTrace( ThinkData & data );
    virtual void ThinkPath( ThinkData & data );
    virtual void ThinkCloseCombat( ThinkData & data );

    virtual void ThinkRoute( ThinkData & data );

    // emergency functions:
    virtual bool EmergencySurvive( ThinkData & data, int enemyEvade = -1, int preferedSide = 0);
    virtual void EmergencyTrace( ThinkData & data );
    virtual void EmergencyPath( ThinkData & data );
    virtual void EmergencyCloseCombat( ThinkData & data );
    virtual void EmergencyRoute( ThinkData & data );

    // acting on gathered data
    virtual void ActOnData( ThinkData & data );
    virtual void ActOnData( ThinkDataBase & data );
public:
    gAICharacter* Character() const {return character;}

    gAIPlayer & AddWaypoint(eCoord const &point);
    void SetRoute(std::vector<eCoord> route);
    void UpdateRouteStep();
    void NextRouteStep();

    //	virtual void AddRef();
    //	virtual void Release();

    gAIPlayer();
    ~gAIPlayer();

    static void ClearAll(); //!< remove all AI players

    // called whenever cylce a drives close to the wall of cylce b.
    // directions: aDir tells whether the wall is to the left (-1) or right(1)
    // of a
    // bDir tells the direction the wall of b is going (-1: to the left, 1:...)
    // bDist is the distance of b's wall to its start.
    static void CycleBlocksWay(const gCycleMovement *a, const gCycleMovement *b,
                               int aDir, int bDir, REAL bDist, int winding);

    // called whenever a cylce blocks the rim wall.
    static void CycleBlocksRim(const gCycleMovement *a, int aDir);

    // called whenever a hole is ripped in a's wall at distance aDist.
    static void BreakWall(const gCycleMovement *a, REAL aDist);

    static void ConfigureAIs();  // ai configuration menu

    static void SetNumberOfAIs(int num, int minPlayers, int iq, int tries=-2); // make sure this many AI players are in the game (with approximately the given IQ)

    void ClearTarget(){target=NULL;}

    // do some thinking. Return value: time to think again
    virtual REAL Think();

    bool Alive();

    virtual bool IsHuman() const { return false; }

    gCycle *Object();

    void Timestep(REAL time);

    virtual void NewObject();                     // called when we control a new object
    virtual void RightBeforeDeath(int triesLeft); // is called right before the vehicle gets destroyed.

    virtual void Color( REAL&r, REAL&g, REAL&b ) const;

    virtual nDescriptor&	CreatorDescriptor() const;
    gAIPlayer(nMessage &m);

    // don't accept syncs from client
    virtual bool AcceptClientSync() const	{
        return false;
    }
};

static tReferenceHolder< gAIPlayer > sg_AIReferences;

// the AI team
class gAITeam: public eTeam
{
public:
    gAITeam(nMessage &m);
    gAITeam();
    virtual nDescriptor &CreatorDescriptor() const;

    static void BalanceWithAIs(bool doBalance = balanceWithAIs);	// fill empty team positions with AI players
    virtual bool PlayerMayJoin(const ePlayerNetID* player) const;	// may player join this team?

    virtual bool BalanceThisTeam() const { return false; } // care about this team when balancing teams
    virtual bool IsHuman() const { return false; } // does this team consist of humans?
};

static gAICharacter* BestIQ( int iq );

#endif
