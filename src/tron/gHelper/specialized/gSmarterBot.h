// #include "defs.h"
// #include "../../gAIBase.h"
// #include "../../gCycle.h"
// #include "../gHelperUtilities.h"
// #include "tString.h"
// #include "ePath.h"
// #include "eTimer.h"
// #include "tRandom.h"


// #ifndef ArmageTron_GHELPER_SMARTERBOT
// #define ArmageTron_GHELPER_SMARTERBOT
// class gAILog;
// struct ThinkData;
// struct ThinkDataBase;

// #define DANGERLEVELS 4
// #define LOOPLEVEL   0
// #define SPACELEVEL  1
// #define TRAPLEVEL   2
// #define COLIDELEVEL 2
// #define TEAMLEVEL   3
// #define TOL 4

// typedef enum
// { SURVIVE = 0,   // just try to stay alive
//   TRACE,         // trace a wall
//   PATH,          // follow a path to a target
//   CLOSECOMBAT,   // try to frag a nearby opponent
// }
// gSTATE;

// typedef enum
// {
//     KILLED = 0,   // just try to stay alive
//     TRAPPED,      // trace a wall
// } gCHAT_ACTION;

// class gAIChat
// {
// public:

//     gAIChat(){};
//     static const int NUM_KILLED_MESSAGES = 3;
//     static const int NUM_TRAPPED_MESSAGES = 2;

//     static const tString killedMessages[NUM_KILLED_MESSAGES];
//     static const tString trappedMessages[NUM_TRAPPED_MESSAGES];

//     static int lastKilledMessage;
//     static int lastTrappedMessage;
//     static tString lastMessage;
//     static REAL lastMessageTime;
    
//     static void Chat(gCHAT_ACTION type,ePlayerNetID* player_);
//     static void Chat(tString message,ePlayerNetID* player_);

// };


// class gSmarterBot
// {
// public:
//     // Constructor
//     gSmarterBot(gCycle *owner);
//     void Activate();
//     static gSmarterBot &Get( gCycle *owner);
//     bool EmergencySurvive( ThinkData & data, int enemyEvade = -1, int preferedSide = 0);
//     void ThinkCloseCombat( ThinkData & data );
//     void SetTraceSide(int side);
//     void ThinkSurvive( ThinkData & data );
//     void ThinkTrace( ThinkData & data );
//     void ThinkPath( ThinkData & data );
//     void SwitchToState(gSTATE nextState, REAL minTime=10);
//     void ActOnData( ThinkData & data );
//     void ActOnData( ThinkDataBase & data );
//     void Chat(gCHAT_ACTION type);
//     void Chat(tString message);
//     void ClearTarget(){target=NULL;}
//     REAL Think();
//     gCycle* Object() { return owner_;}

// tReproducibleRandomizer randomizer_;
// private:
//     bool emergency;
//     gCycle *owner_; // Pointer to the owner cycle
//     gHelper *helper_;
//     gSTATE state;
//     gSTATE lastState;
//     nObserverPtr< gCycle > target; 
//     int triesLeft;
//     REAL nextStateChange;
//     REAL lastStateChange;
//     REAL nextTime;
//     REAL lastChangeAttempt;
//     REAL lazySideChange;
//     REAL traceSide;
//     REAL freeSide;
//     ePath path;    // last found path to the victim
//     REAL lastPath;
//     REAL lastTime;
//     // gAILog * log;
//     gAIChat chat;
//     REAL concentration;
// };
// #endif
