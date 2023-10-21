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

#ifndef ArmageTron_GAME_H
#define ArmageTron_GAME_H

#include "rSDL.h"

#include "nNetObject.h"

#include "gCycle.h"

#include <map>
#include "nServerInfo.h"
#define GS_CREATED 0 // newborn baby
#define GS_TRANSFER_SETTINGS 7
#define GS_CREATE_GRID 10
#define GS_CREATE_OBJECTS 20
#define GS_TRANSFER_OBJECTS 30
#define GS_CAMERA 35
#define GS_SYNCING 40
#define GS_SYNCING2 41
#define GS_SYNCING3 42
#define GS_PLAY 50
#define GS_DELETE_OBJECTS 60
#define GS_DELETE_GRID 70
#define GS_STATE_MAX 80

class eGrid;
class nServerInfo;
class nServerInfoBase;
class eTeam;
class gParser;

extern nServerInfoBase *getSeverFromStr(tString input);
extern void InitHelperItems(bool ingame = false);

typedef enum
{
    gFREESTYLE,
    gDUEL,
    gHUMAN_VS_AI
} gGameType;

extern bool tryConnectLastServer;
extern bool sg_forcePlayerUpdate, updatedThisRound;
// extern gGameType sg_gameType;      // the current game type
extern bool sg_TalkToMaster; // should this server be known on the internet?
extern bool sg_RequestedDisconnection, sg_connectToLastServerFromMenu;
extern bool sg_connectToLastServerOnStart;
extern tString sg_lastServerStr;

typedef enum
{
    gFINISH_EXPRESS,
    gFINISH_IMMEDIATELY,
    gFINISH_SPEEDUP,
    gFINISH_NORMAL
} gFinishType;

class gZone;

// extern gFinishType sg_finishType;

class gGame : public nNetObject
{
    unsigned short state;     // the gamestate we are currently in
    unsigned short stateNext; // if a state change has been requested

    tJUST_CONTROLLED_PTR<gZone> winDeathZone_; // the win zone

    bool goon;

    int rounds; // the number of rounds played

    double startTime; // time of the match start

    int warning; // timeout warnings

    tCONTROLLED_PTR(eGrid) grid; // the grid the game takes place

    bool synced_; //!< flag indicating whether the game is considered synced

    void Init();

    gParser *aParser;

public:
    gGame();
    gGame(nMessage &m);
    virtual ~gGame();
    virtual void WriteSync(nMessage &m);
    virtual void ReadSync(nMessage &m);
    virtual nDescriptor &CreatorDescriptor() const;

    static void NetSync();     // do all the network syncronisation.
    static void NetSyncIdle(); // do all the network syncronisation and wait a bit.

    virtual void Verify(); // verifies settings are OK, throws an exception if not.

    virtual void StateUpdate(); // switch to new gamestate (does all
    // the real work around here).
    virtual void SetState(unsigned short act, unsigned short next);
    virtual short GetState() { return state; }

    // make sure the clients are catching up
    void SyncState(unsigned short state);

    virtual void Timestep(REAL time, bool cam = false); // do all the world simulation
    virtual void Analysis(REAL time);                   // do we have a winner?

    virtual bool GameLoop(bool input = true); // return values: exit the game loop?

    bool GridIsReady(int c); // can we transfer gameObjects that need the grid to exist?
    eGrid *Grid() const { return grid; }

    void NoLongerGoOn();

    void StartNewMatch();

    virtual void RebuildGrid(int requestedState);

    void StartNewMatchNow();

    static tCONTROLLED_PTR(gGame) CurrentGame();
};

void update_settings(bool const *goon = 0);
void ret_to_MainMenu();

extern nServerInfoBase* CurrentServer();
static void setCurrentServer(nServerInfoBase * currentSrv);
void ConnectToServer(nServerInfoBase *server);

void sg_EnterGame(nNetState enter_state);
void sg_HostGame();
void sg_HostGameMenu();

bool ConnectToLastServer();
bool ConnectToLastServerFromStr();
// runs a single player game
void sg_SinglePlayerGame();

void MainMenu(bool ingame = false);

bool GridIsReady(int c);

bool SafeToSpawnObject();

void Activate(bool act);

void sg_DeclareWinner(eTeam *team, char const *message);
void sg_DeclareWinner(eTeam *team);

// Race timer hack begin
class ePlayerNetID;
void sg_DeclareRaceWinner(ePlayerNetID *player);
// Race timer hack end

void sg_FullscreenMessage(tOutput const &title, tOutput const &message, REAL timeout = 60, int client = 0); //!< Displays a message on a specific client or all clients that gets displayed on the whole screen, blocking view to the game
void sg_ClientFullscreenMessage(tOutput const &title, tOutput const &message, REAL timeout = 60);           //!< Displays a message locally that gets displayed on the whole screen, blocking view to the game

void sg_RespawnAllAfter(REAL time);

class gGameSettings
{
public:
    int scoreWin; // score you get when you win a round

    int limitTime;         // match time limit. became set time limit if limitSet > 1
    int limitRounds;       // match round limit
    int limitScore;        // match score limit
    int limitScoreMinLead; // minimum lead for limitScore to take effect
    int maxBlowout;        // max score advance
    int limitSet;          // match set limit (as tennis "set", n means best of 2n-1)

    int numAIs;     // number of AI players
    int minPlayers; // minimum number of players
    int AI_IQ;      // preferred IQ of AI players

    bool autoNum; // automatically adjust number of AIs
    bool autoIQ;  // automatically adjust IQ of AIs

    REAL speedFactor; // logarithm of cycle speed multiplier
    REAL sizeFactor;  // logarithm of arena size multiplier

    int autoAIFraction; // helper variable for the autoAI functions

    REAL winZoneMinRoundTime; // minimum number of seconds a round must be going before the win zone is activated
    REAL winZoneMinLastDeath; // minimum number of seconds the last death happended before the win zone is activated

    gGameType gameType;     // what type of game is played?
    gFinishType finishType; // what happens when all humans are dead?

    // team settings
    int minTeams, maxTeams;
    int minPlayersPerTeam, maxPlayersPerTeam;
    int maxTeamImbalance;
    bool balanceTeamsWithAIs, enforceTeamRulesOnQuit;

    // game mechanics settings
    REAL wallsStayUpDelay; // the time the cycle walls stay up ( negative values: they stay up forever )
    REAL wallsLength;      // the maximum total length of the walls
    REAL explosionRadius;  // the radius of the holes blewn in by an explosion

    gGameSettings(int a_scoreWin,
                  int a_limitTime, int a_limitRounds, int a_limitScore, int a_limitSet,
                  int a_limitScoreMinLead, int a_maxBlowout,
                  int a_numAIs, int a_minPlayers, int a_AI_IQ,
                  bool a_autoNum, bool a_autoIQ,
                  REAL a_speedFactor, REAL a_sizeFactor,
                  gGameType a_gameType, gFinishType a_finishType,
                  int a_minTeams,
                  REAL a_winZoneMinRoundTime, REAL a_winZoneMinLastDeath);

    void AutoAI(bool success);
    void Menu();
};

class gGameSpawnTimer
{
public:
    static void Reset();
    static void Sync(int alive, int ai_alive, int humns);
    static bool Active() { return timerActive; }

    static REAL GetStartTimer() { return startTimer_; }
    static REAL GetLaunchTime() { return launchTime_; }
    static REAL GetTargetTime() { return targetTime_; }
    static void SetStartTimer(REAL time) { startTimer_ = time; }
    static void SetLaunchTime(REAL time) { launchTime_ = time; }
    static void SetTargetTime(REAL time) { targetTime_ = time; }

    static void SetCountdown(int countdown) { countDown_ = countdown; }
    static int GetCountdown() { return countDown_; }

    static void SetTimerActive(bool active) { timerActive = active; }

private:
    static REAL startTimer_; //!<    seconds of which the user sent
    static REAL launchTime_; //!<    time the timer hs launched at
    static REAL targetTime_; //!<    time the timer will finish at
    static bool timerActive; //!<    flag if the timer is finished
    static int countDown_;   //!<    the countdown to initiate
};

//  SHUTDOWN HACK BEGIN
class ShutDownCounter
{
public:
    ShutDownCounter();

    bool IsActive() { return isActive_; }
    void SetActive(bool value) { isActive_ = value; }

    int Timeout() { return timeout_; }
    void SetTimeout(int value) { timeout_ = value; }

    void Start();
    void Stop();

    void Execute();

private:
    bool isActive_;
    int timeout_;
};
//  SHUTDOWN HACK END

extern gGameSettings *sg_currentSettings;
extern bool sg_LogTurns;
extern tString mapfile;

void sg_OutputOnlinePlayers();
void LoadMap(tString mapName);

void LogPlayersCycleTurns(gCycle *cycle, tString msg);
void LogWinnerCycleTurns(gCycle *winner);

extern void sg_DetermineSpawnPoint(ePlayerNetID *p, eCoord &pos, eCoord &dir);

#include <queue>
#include <unordered_map>
#include <functional>
#include <set>
struct DelayedTask
{
    std::string id;
    REAL dueTime;
    REAL interval;
    REAL originalDelay;
    std::function<void()> task;
    DelayedTask() {}

    DelayedTask(const std::string &id, REAL dueTime, REAL interval, REAL originalDelay, std::function<void()> task)
        : id(id), dueTime(dueTime), interval(interval), originalDelay(originalDelay), task(std::move(task)) {}

    bool operator<(const DelayedTask& other) const
    {
        return dueTime > other.dueTime;
    }
};


class TaskScheduler
{
public:
    // Schedule a task
    bool schedule(std::string id, REAL delayInSeconds, std::function<void()> task, REAL interval = 0, bool useQueue = false)
    {
        REAL dueTime = tSysTimeFloat() + delayInSeconds;

        if (isTaskScheduled(id))
        {
            if (!useQueue)
            {
                return false; 
            }
            else
            {
                DelayedTask newTask(id, dueTime, interval, delayInSeconds, std::move(task));
                pendingTasks[id].push(newTask);
                return true;
            }
        }

        auto delayedTask = DelayedTask(id, dueTime, interval, delayInSeconds, std::move(task));
        tasksQueue.push(delayedTask);
        tasksMap[id] = delayedTask;

        return true;
    }


    void enqueueChain(const std::function<void()>& chain)
    {
        taskChains.push(chain);
    }

    bool isTaskScheduled(const std::string& id) const {
        return tasksMap.count(id) > 0;
    }

    // Check and execute due tasks
    void update()
    {

        if (tasksQueue.empty() && !taskChains.empty())
        {
            auto taskChain = taskChains.front();
            taskChains.pop();
            taskChain();
        }

        while (!tasksQueue.empty() && tSysTimeFloat() >= tasksQueue.top().dueTime)
        {
            DelayedTask task = tasksQueue.top();
            tasksQueue.pop();
            tasksMap.erase(task.id);

            task.task(); 

            if(pendingTasks.count(task.id) > 0 && !pendingTasks[task.id].empty())
            {
                DelayedTask nextTask = pendingTasks[task.id].front();
                pendingTasks[task.id].pop();

                nextTask.dueTime = tSysTimeFloat() + nextTask.originalDelay; 

                tasksQueue.push(nextTask);
                tasksMap[nextTask.id] = nextTask;
                
                if(pendingTasks[task.id].empty())
                    pendingTasks.erase(task.id); 
            }
            else if(task.interval > 0)
            {
                task.dueTime += task.interval;
                tasksQueue.push(task);
                tasksMap[task.id] = task;
            }
        }
    }

    // Remove a task
    void remove(const std::string& id)
    {
        if (tasksMap.count(id) > 0) {
            tasksMap.erase(id);
            rebuildQueue();
        }
    }

    // Remove all tasks
    void clear()
    {
        while (!tasksQueue.empty()) {
            tasksQueue.pop();
        }
        tasksMap.clear();
    }
    
    std::unordered_map<std::string, DelayedTask> getTasks() const
    {
        return tasksMap;
    }


private:
    std::priority_queue<DelayedTask> tasksQueue;
    std::unordered_map<std::string, DelayedTask> tasksMap;
    std::unordered_map<std::string, int> taskCounts;
    std::unordered_map<std::string, std::queue<DelayedTask>> pendingTasks;
    std::queue<std::function<void()>> taskChains;
    void rebuildQueue()
    {
        while (!tasksQueue.empty()) {
            tasksQueue.pop();
        }
        for (const auto& taskPair : tasksMap) {
            tasksQueue.push(taskPair.second);
        }
    }
};

// Global instance of the task scheduler
extern TaskScheduler gTaskScheduler;
#endif
