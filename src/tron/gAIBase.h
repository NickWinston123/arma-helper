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
class gAINavigator;

class gSensor;
class gAISensor;
class gAILog;
class gAICharacter;

namespace Game
{
    class AIPlayerSync;
    class AITeamSync;
}

typedef enum
{
    AI_SURVIVE = 0,  // just try to stay alive
    AI_TRACE,        // trace a wall
    AI_PATH,         // follow a path to a target
    AI_CLOSECOMBAT,  // try to frag a nearby opponent
    AI_PATH_GIVEN,   // follow a predetermined path to a target, abort if danger looms
    AI_PATH_MINDLESS // follow a path to a target mindlessly without caring whether it's suicide
} gAI_STATE;

class gSimpleAI
{
public:
    gSimpleAI()
    {
    }

    // do the thinking
    inline REAL Think(REAL maxStep)
    {
        return DoThink(maxStep);
    }

    virtual ~gSimpleAI();

    gCycle *Object() { return object_; }
    void SetObject(gCycle *cycle) { object_ = cycle; }

protected:
    virtual REAL DoThink(REAL maxStep) = 0;

private:
    tJUST_CONTROLLED_PTR<gCycle> object_;
};

class gSimpleAIFactory : public tListItem<gSimpleAIFactory>
{
public:
    gSimpleAI *Create(gCycle *object) const;
    static gSimpleAIFactory *Get();
    static void Set(gSimpleAIFactory *factory);

protected:
    virtual gSimpleAI *DoCreate() const = 0;

private:
    static gSimpleAIFactory *factory_;
};

class gAIPlayer : public ePlayerNetID
{
    friend class gAITeam;

protected:
    // for pathfinding mode:
    ePath path;    // last found path to the victim
    REAL lastPath; // when was the last time we did a pathsearch?

    // for trace mode:
    int traceSide;
    REAL lastChangeAttempt;
    REAL lazySideChange;

    // state management:
    gAI_STATE state;      // the current mode of operation
    REAL nextStateChange; // when is the operation mode allowed to change?

    bool emergency; // tell if an emergency is present
    int triesLeft;  // number of tries left before we die

    REAL freeSide; // number that tells which side is probably free for evasive actions

    // basic thinking:
    REAL lastTime;
    REAL nextTime;

    REAL concentration;

    // log
    gAILog *log;

    //  gCycle * Cycle(){return object;}

    // set trace side:
    void SetTraceSide(int side);

    // state change:
    void SwitchToState(gAI_STATE nextState, REAL minTime = 10);

    // data structure common to thinking functions
public:
    //! set sight on target (side effects: switch state accordingly)
    void SetTarget(eNetGameObject *target);

    eNetGameObject const *GetTarget() const
    {
        return target_;
    }

    gAI_STATE GetState() const { return state; } // the current mode of operation

    struct ThinkDataBase
    {
        int turn;        // direction to turn to
        REAL thinkAgain; // when to think again

        ThinkDataBase()
            : turn(0), thinkAgain(0)
        {
        }
    };

protected:
    struct ThinkData : public ThinkDataBase
    {
        gAISensor const &front; // sensors cast by upper level function
        gAISensor const &left;
        gAISensor const &right;

        ThinkData(gAISensor const &a_front, gAISensor const &a_left, gAISensor const &a_right)
            : front(a_front), left(a_left), right(a_right)
        {
        }
    };

    // state update functions:
    virtual void ThinkSurvive(ThinkData &data);
    virtual void ThinkTrace(ThinkData &data);
    virtual void ThinkPath(ThinkData &data);
    virtual void ThinkPathGiven(ThinkData &data, bool emergency = false);
    virtual void ThinkCloseCombat(ThinkData &data);

    // emergency functions:
    virtual bool EmergencySurvive(ThinkData &data, int enemyEvade = -1, int preferedSide = 0);
    virtual void EmergencyTrace(ThinkData &data);
    virtual void EmergencyPath(ThinkData &data);
    virtual void EmergencyCloseCombat(ThinkData &data);

    // acting on gathered data
    virtual void ActOnData(ThinkData &data);
    virtual void ActOnData(ThinkDataBase &data);

public:
    gAICharacter *Character() const { return character_; }

    //	virtual void AddRef();
    //	virtual void Release();

    gAIPlayer();
    ~gAIPlayer();

    static void ClearAll(); //!< remove all AI players

    // add a coordinate to the path, switch to path follow mode IN REVERSE ORDER
    void AddToPath(eCoord const &target, bool mindless = false);

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

    static void ConfigureAIs(); // ai configuration menu

    static void SetNumberOfAIs(int num, int minPlayers, int iq, int tries = 3); // make sure this many AI players are in the game (with approximately the given IQ)

    void ClearTarget() { target = NULL; }

    virtual void ControlObject(eNetGameObject *c)
    {
        ePlayerNetID::ControlObject(c);
        simpleAI_ = NULL;
    }
    virtual void ClearObject()
    {
        ePlayerNetID::ClearObject();
        simpleAI_ = NULL;
    }

    // do some thinking. Return value: time to think again
    virtual REAL Think(REAL maxStep);

    bool Alive()
    {
        return bool(Object()) && Object()->Alive();
    }

    virtual bool IsHuman() const { return false; }

    gCycle *Object();

    void Timestep(REAL time);

    virtual void NewObject();                     // called when we control a new object
    virtual void RightBeforeDeath(int triesLeft); // is called right before the vehicle gets destroyed.

    virtual void Color(REAL &r, REAL &g, REAL &b) const;

    virtual nDescriptor &CreatorDescriptor() const;
    gAIPlayer(nMessage &m);

    //! reads incremental sync data. Returns false if sync was invalid or old.
    // bool ReadSync( Game::AIPlayerSync const & sync, nSenderInfo const & sender );
    //! writes sync data (and initialization data if flat is set)
    // void WriteSync( Game::AIPlayerSync & sync, bool init );
    //! returns the descriptor responsible for this class
    // virtual nNetObjectDescriptorBase const & DoGetDescriptor() const;

    // an AI state
    class State : public tReferencable<State>
    {
    public:
        State(gAIPlayer &player);
        virtual ~State();

        //! executes the state's work
        virtual REAL Think(REAL maxStep) = 0;

    protected:
        gAIPlayer &Parent() { return parent_; }
        gAIPlayer const &Parent() const { return parent_; }

        gAINavigator &Navigator();
        gAINavigator const &Navigator() const;

        gAICharacter const &Character() const;

    private:
        gAIPlayer &parent_;
    };

    friend class State;

    // state management
    void SwitchToState(State *newState);
    // State * GetState() const{ return state_; }

    // switch to survival state
    void SwitchToSurvival();

    // state for grinding at start
    class StateGrind : public State
    {
    public:
        enum Substate
        {
            AI_NONE = 0,
            AI_GRIND_GRIND, // currently getting closer to teammate's wall
            AI_GRIND_SPLIT_LEFT,
            AI_GRIND_SPLIT_RIGHT, // split as soon as you overtake your teammate
            AI_SUBSTATE_COUNT
        };

        StateGrind(gAIPlayer &player);
        virtual ~StateGrind();

        //! executes the state's work
        virtual REAL Think(REAL maxStep);
    };

private:
    tReproducibleRandomizer randomizer_;

    gSimpleAI *simpleAI_;
    gAICharacter *character_; // our specification of abilities

    // for all offensive modes:
    nObserverPtr<eNetGameObject> target_; // the current victim
    nObserverPtr<gCycle> target;          // the current victim

    // basic thinking resource management
    REAL lastTime_;
    REAL nextTime_;
    REAL concentration_;

    //! navigator
    std::unique_ptr<gAINavigator> navigator_;

    //! state
    tJUST_CONTROLLED_PTR<State> state_;
    void CreateNavigator();
};

// the AI team
class gAITeam : public eTeam
{
public:
    gAITeam(nMessage &m);
    gAITeam();
    virtual nDescriptor &CreatorDescriptor() const;

    static void BalanceWithAIs(bool doBalance = balanceWithAIs);  // fill empty team positions with AI players
    virtual bool PlayerMayJoin(const ePlayerNetID *player) const; // may player join this team?

    virtual bool BalanceThisTeam() const { return false; } // care about this team when balancing teams
    virtual bool IsHuman() const { return false; }         // does this team consist of humans?
};

#endif
