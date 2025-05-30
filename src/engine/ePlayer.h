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

#ifndef ArmageTron_PLAYER_H
#define ArmageTron_PLAYER_H

#ifndef MAX_INSTANT_CHAT
#define MAX_INSTANT_CHAT 30
#endif

#define MAX_PLAYERS 4

//For Menu Color Settings
enum PlayerColorMode {
    OFF = 0,
    RANDOM = 1,
    UNIQUE = 2,
    RAINBOW = 3,
    CROSSFADE = 4,
};

//For Menu Color Settings
enum PlayerColorNameMode {
    OFF_NAME = 0,
    GRADIENT_NAME = 1,
    RAINBOW_NAME = 2,
    SHIFT_NAME = 3,
};

#include "rSDL.h"

#include "uInput.h"
#include "tList.h"
#include "tString.h"
#include "eCamera.h"
#include "eNetGameObject.h"
#include "tCallbackString.h"
#include "nSpamProtection.h"
#include "../tron/gCycleMovement.h"

#include <set>
#include <list>
#include <utility>
#include "eChat.h"

#include <unordered_map>

class TempConfItemManager;

extern tString se_chatHistoryFileName;
extern std::deque<tString> se_chatHistory;

extern bool se_chatLog, se_chatTimeStamp;

extern bool se_playerWatchAutoRandomName, forceRandomRename;
extern REAL se_playerWatchAutoRandomNameRevertTime;
extern int se_playerWatchAutoRandomNameBanLimit;

extern bool se_avoidPlayerWatch, se_avoidPlayerWatchDisable;
extern REAL se_avoidPlayerWatchActionTime;
extern tString se_avoidPlayerWatchList;

extern tString se_disableCreateSpecific;
extern tString forcedChattingPlayers;
extern bool se_forceJoinTeam;

extern bool se_watchActiveStatus;
extern int se_watchActiveStatusTime;

extern bool sg_playerSpamProtectionWatch;

extern bool sg_playerSilencedWatch, sg_playerSilencedWatchQuit, sn_playerSilencedWatchQuitWaitForAvoidPlayers;
extern REAL sg_playerSpamProtectionWatchExtraAdd;
extern void se_UniqueColor(ePlayer *local_p );
extern void se_RandomizeColor(ePlayer *local_p);
extern void se_rainbowColor(ePlayer *local_p);
extern void se_CrossFadeColor(ePlayer *local_p);

extern bool avoidPlayerInGame(tString name = tString(""));
nMessage *se_NewChatMessage(ePlayerNetID const *player, tString const &message);

enum playerWatchStatus {
    UNSET = 0,
    INACTIVE = 1,
    ACTIVE = 2
};

extern bool se_toggleChatFlag, se_chatFlagAlways, se_BlockChatFlags;

#define PLAYER_CONFITEMS (60+MAX_INSTANT_CHAT)


extern bool se_enableChatCommands;

extern bool se_enableChatCommandsTabCompletion;

extern tString se_encryptCommand;
static int forceCreatePlayer = -1;
extern bool se_disableCreate;
// maximal length of chat message
extern int se_SpamMaxLen;

// Maximum number of chat entries to save for spam analysis
extern int se_lastSaidMaxEntries;

// time during which no repeaded chat messages are printed
extern REAL se_alreadySaidTimeout;

// minimal access level for chat
extern tAccessLevel se_chatAccessLevel;

// time between public chat requests, set to 0 to disable
extern REAL se_chatRequestTimeout;

// List the colors of other players.
static void se_listPlayerColors();

// call on commands that only work on the server; quit if it returns true
bool se_NeedsServer(char const * command, std::istream & s, bool strict = true );

class tConfItemBase;
class uAction;
class tOutput;
class eTeam;
class eVoter;
class gCycle;
class eChatCommand;

class ePlayer: public uPlayerPrototype
{
    friend class eMenuItemChat;
    static uActionPlayer s_chat;
    static uActionPlayer s_con;
    static uActionTooltip s_chatTooltip;
    TempConfItemManager confItems;

    double lastTooltip_;
public:
    bool updatedThisRound = false;

    static void resetPlayerUpdateStatuses()
    {
        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            ePlayer *local_p = ePlayer::PlayerConfig(i);
            local_p->updatedThisRound = false;
        }
    }

    int updateIteration;
    tString    name;                 // the player's screen name
    tString    globalID;             // the global ID of the player in user@authority form
    // REAL	   rubberstatus;
    tString     teamname;
    bool       centerIncamOnTurn;
    bool       wobbleIncam;
    bool       autoSwitchIncam;

    bool       spectate;              // shall this player always spectate?
    bool       stealth;               // does this player wish to hide his/her identity?
    bool       autoLogin;             // should the player always request authentication on servers?

    bool 	   nameTeamAfterMe;       // player prefers to call his team after his name
    int		   favoriteNumberOfPlayersPerTeam;

    ePlayerNetID * watchPlayer; // the player we want to watch

    //Smarter bot
    bool sg_smarterBotThink;
    REAL sg_smarterBotRange;
    REAL sg_smarterBotRandomScale;
    REAL sg_smarterBotRubberEval;
    REAL sg_smarterBotSurviveEval;
    REAL sg_smarterBotTrapScale;
    REAL sg_smarterBotFollowScale;
    bool sg_smarterBotFollowBlockLogic;
    bool sg_smarterBotFollowCheckLogic;
    REAL sg_smarterBotFollowPredictionTime;
    bool sg_smarterBotFollowTryLogic;
    bool sg_smarterBotFollowTryLogicOppositeTurn;
    bool sg_smarterBotFollowFindTarget;
    bool sg_smarterBotFollowFindZone;
    bool sg_smarterBotFollowTail;
    tString sg_smarterBotFollowTarget;
    tString sg_smarterBotFollowTargetTeamList;
    REAL sg_smarterBotFollowAlignedThresh;
    REAL sg_smarterBotPlanScale;
    REAL sg_smarterBotTailScale;
    REAL sg_smarterBotSpaceScale;
    REAL sg_smarterBotCowardScale;
    REAL sg_smarterBotTunnelScale;
    REAL sg_smarterBotSpeedScale;
    REAL sg_smarterBotNextThinkMult;
    REAL sg_smarterBotTurnRandMult;
    int sg_smarterBotState;

    tString sg_smarterBotContributionStr;



    eCamMode startCamera;
    bool     allowCam[10];
    int      FOV;
    bool     smartCustomGlance; //!< flag making the smart camera use the custom settings for glancing

    tCHECKED_PTR(eCamera)           cam;
    tCONTROLLED_PTR(ePlayerNetID) netPlayer;

    int rgb[3]; // our color

    int colorMode; // The players color randomization
    int colorNameMode; // The players name color randomization
    int playerRandomColorNameStartMode;

    int lastplayerRandomColorNameStartMode;
    tString instantChatString[MAX_INSTANT_CHAT];
    // instant chat macros

    static uActionPlayer *se_instantChatAction[MAX_INSTANT_CHAT];

    ePlayer();
    virtual ~ePlayer();

    virtual const char *Name() const{return name;}
    virtual const char *Teamname() const{return teamname;}

    virtual bool Act(uAction *act,REAL x);

    int ID() const {return id;}
#ifndef DEDICATED
    void Render();
#endif

    static ePlayer * PlayerConfig(int p);

    static ePlayer *NetToLocalPlayer(ePlayerNetID *player);
    static ePlayer  *gCycleToLocalPlayer(gCycle *owner);

    static bool PlayerIsInGame(int p);
    static bool PlayerIsInGame(ePlayerNetID *p);

    // veto function for tooltips that require a controllable game object
    static bool VetoActiveTooltip(int player);

    static rViewport * PlayerViewport(int p);

    static void LogIn();          //!< sends authentication login messages for all local players
    static void SendAuthNames();  //!< sends authentication names and authentication wishes for all local players

    static void Init();
    static void Exit();
};

//! class managing access levels.
class eAccessLevelHolder
{
public:
    eAccessLevelHolder();

    tAccessLevel GetAccessLevel() const { return accessLevel; }
    void SetAccessLevel( tAccessLevel level );

private:
    tAccessLevel     accessLevel;    //!< admin access level of the current user
};

//! detector for turn timing assist bots
class eUncannyTimingDetector
{
public:
    //! settings for a single analyzer
    struct eUncannyTimingSettings
    {
        REAL timescale; //!< the timescale. Events are divided in two buckets, one between 0 and timescale/2, the other from timescale/2 to timescale.
        REAL maxGoodRatio; //!< the maximal allowed recent ratio of events to land in the 'good' bucket
        REAL goodHumanRatio; //!< the maximal observed ratio for a human
        int  averageOverEvents; //!< number of events to average over

        mutable REAL bestRatio; //!< best ratio achieved by players during this session

        eUncannyTimingSettings( REAL ts, REAL human, REAL max )
        : timescale( ts ), maxGoodRatio( max ), goodHumanRatio(human), averageOverEvents(40)
        , bestRatio(0)
        {}

        ~eUncannyTimingSettings();
    };

    //! single analyzer with single timescale
    class eUncannyTimingAnalysis
    {
    public:
        //! analyze a single timing event
        REAL Analyze( REAL timing, eUncannyTimingSettings const & settings );
        eUncannyTimingAnalysis();
    private:
        REAL accurateRatio; //!< ratio of events in the more accurate half
        int turnsSoFar;     //!< number of turns accounted for so far
    };

    //! detection level of timing aid hacks
    enum DangerLevel
    {
        DangerLevel_Low,    //!< about 25% of the tolerance reached
        DangerLevel_Medium, //!< about 50% of the tolerance reached
        DangerLevel_High,   //!< about 75% of the tolerance reached
        DangerLevel_Max     //!< 100% of the tolerance reached, worst action triggered
    };

    eUncannyTimingDetector();

    //! analzye a timing event
    void Analyze( REAL timing, ePlayerNetID * player );
private:
    //! three analyzers for varying timescales
    eUncannyTimingAnalysis fast, medium, slow;

    DangerLevel dangerLevel;
};

// the class that identifies players across the network
class ePlayerNetID: public nNetObject, public eAccessLevelHolder{
    friend class ePlayer;
    friend class eTeam;
    friend class eNetGameObject;
    friend class tControlledPTR< ePlayerNetID >;
    // access level. lower numeric values are better.
public:

    struct SyncData
    {
        REAL score;
        bool spectating, chatting;
        SyncData()
            : score(0), spectating(false), chatting(false)
        {
        }
    };
    bool removedFromGame = false;
    SyncData lastSyncMessage_;
    static tArray<tString> nameSpeakWords;
    static int nameSpeakIndex;
    static bool nameSpeakForceUpdate;
    static bool nameSpeakCheck;
    static int nameSpeakPlayerID;
    static int playerUpdateIteration;

    static double nextSpeakTimePrefix;
    static tString nextSpeakTimePrefixCommonPrefix;
    static double nextSpeakTime;

    tString lastSentName;
    
    static bool canChat();
    static bool canChatCommonPrefix(tString message);
    static bool canChatWithMsg(tString message);
    static void setNextSpeakTime(REAL seconds);
    static void setNextSpeakTimeCommonPrefix(tString commonPrefix, REAL seconds);

    tString lastName = tString("");
    bool isLocal() { return pID != -1; }
    ePlayerNetID * lastKilledPlayer;
    ePlayerNetID * lastDiedByPlayer = nullptr;
    ePlayerNetID * lastDiedByPlayerBanFunc = nullptr;
    REAL lastDiedByTime = 0;
    REAL lastBannedPlayerTime = 0;
    REAL lastTurnTime = -999;
    REAL lastTeamCreateTime = -999;

    ePlayerNetID * lastMessagedPlayer;
    ePlayerNetID * lastMessagedByPlayer;
    std::string lastMsgType = "normal";
    REAL lastCycleDeathTime = 0;
    typedef std::set< eTeam * > eTeamSet;
    bool respawnedLocally;
    playerWatchStatus lastWatchStatus;
    static void watchPlayerStatus();
    static ePlayerNetID  *gCycleToNetPlayer(gCycle *owner);
    static gCycle *NetPlayerToCycle(ePlayerNetID *player);
    gCycle *NetPlayerToCycle();
    static ePlayerNetID *GetPlayerByName(tString name, bool exact = true);
    int lastsyncedColor[3] = {0, 0, 0};
    tColoredString lastColoredName;
    int currentShift = 0;
    int shiftIter = 0;
    tArray<tString> lastSearch;
    tArray<tString> nameHistory;
    bool nameFirstSync  = true;
    bool teamFirstSync  = true;
    nTimeAbsolute joinedTeamTime() { return timeJoinedTeam; }
    nTimeAbsolute createdTime() { return timeCreated_; }

    bool encryptVerified = false;
    bool departedByChatBot = false;
    bool greetedByChatBot = false;
    bool acknowledgeLeftSpectatorByChatbot = false;
    bool acknowledgeEnterSpectatorByChatbot = false;
private:

    int listID;                          // ID in the list of all players
    int teamListID;                      // ID in the list of the team

    bool							silenced_;		    // flag indicating whether the player has been silenced
    int                             suspended_;         //! number of rounds the player is currently suspended from playing
    tString                         suspendReason_;     // what was the reason for suspending this person...?

    nTimeAbsolute                   timeCreated_;   // the time the player was created
    nTimeAbsolute					timeJoinedTeam; // the time the player joined the team he is in now
    tCONTROLLED_PTR(eTeam)			nextTeam;		// the team we're in ( logically )
    tCONTROLLED_PTR(eTeam)			currentTeam;	// the team we currently are spawned for
    eTeamSet                        invitations_;   // teams this player is invited to

    tCHECKED_PTR(eNetGameObject) object; // the object this player is
    // controlling

    int score; // points made so far
    int lastScore_; // last saved score

    int favoriteNumberOfPlayersPerTeam;		// join team if number of players on it is less than this; create new team otherwise
    bool nameTeamAfterMe; 					// player prefers to call his team after his name
public:
    bool greeted;        					// did the server already greet him?
    bool disconnected;   					// did he disconnect from the game?
private:
    static void SwapPlayersNo(int a,int b); // swaps the players a and b

    ePlayerNetID& operator= (const ePlayerNetID&); // forbid copy constructor

    bool			spectating_; //!< are we currently spectating? Spectators don't get assigned to teams.
    bool			stealth_; //!< does this player want to hide his/her identity?
    bool			chatting_;   //!< are we currently chatting?
public:
    struct tm       createTime_;
private:
    REAL            timeSinceLastChat_;
    int				chatFlags_;  //!< different types of reasons for beeing chatting
    bool			allowTeamChange_; //!< allow team changes even if ALLOW_TEAM_CHANGE is disabled?

    //For improved remoteadmin
    tAccessLevel     lastAccessLevel;//!< access level at the time of the last name update

    eUncannyTimingDetector uncannyTimingDetector_; //!< detector for timingbots

    nMachine *      registeredMachine_; //!< the machine the player is registered with
    void RegisterWithMachine();         //!< registers with a machine
    void UnregisterWithMachine();       //!< un registers with a machine

    tJUST_CONTROLLED_PTR< ePlayerNetID > substitute; // the player who will replace this one next round

public:
    bool SetSubstitute(ePlayerNetID *p); //!< set substitute for this player
    bool ApplySubstitution();            //!< perform substitution ie remove this player from the game and replace him by his substitute at the same position
    static void ClearSubstitutes();      //!< remove all substitutes
    static void ApplySubstitutions();    //!< perform substitutions for all players

    enum			ChatFlags
    {
        ChatFlags_Chat = 1,
        ChatFlags_Away = 2,
        ChatFlags_Menu = 4,
        ChatFlags_Console = 8
    };

    void RespawnPlayer( bool local = false);
    bool flagOverrideChat;
    bool flagChatState;

    int    pID;
    // REAL	rubberstatus;
    tString teamname;
    tString teamnameNew, teamnameSync;
    REAL lastTeamName, lastTeamNameSync;
    tArray<tString> lastSaid;
    tArray<nTimeRolling> lastSaidTimes;
    //	void SetLastSaid(tString ls);
    unsigned short r,g,b; // our color
    int syncIteration;
    bool overrideColor;
    gRealColor colorOverride;

    unsigned short pingCharity; // max ping you are willing to take over

    REAL ping;

    double lastSync;         //!< time of the last sync request
    double lastActivity_;    //!< time of the last activity

    bool loginWanted;        //!< flag indicating whether this player currently wants to log on

    bool renameAllowed_;     //!< specifies if the player is allowed to rename or not, does not know about votes.

    nSpamProtection & GetChatSpam();       //!< chat volume spam
    eChatLastSaid & GetLastSaid();         //!< last said information
    eShuffleSpamTester & GetShuffleSpam(); //!< shuffle message spam

    ePlayerNetID(int p=-1, int owner = -1);
    ePlayerNetID(nMessage &m);
    virtual ~ePlayerNetID();

    virtual bool ActionOnQuit();
    virtual void ActionOnDelete();

    // Check if a player can be respawned. Relaying on team alone is not enough.
    // If a player enters as spectator, they are still assumed to be on a team.
    // When a player is suspeded they are also on a team until the end of the round.
    bool CanRespawn() const { return currentTeam && suspended_ == 0 && ! spectating_; }

    // chatting
    bool IsChatting() const { return chatting_; }
    void SetChatting ( ChatFlags flag, bool chatting );

    // spectating
    bool IsSpectating() const { return spectating_; }

    bool StealthMode() const { return stealth_; }

    // team management
    bool TeamChangeAllowed( bool informPlayer = false ) const; //!< is this player allowed to change teams?
    void SetTeamChangeAllowed(bool allowed) {allowTeamChange_ = allowed;} //!< set if this player should always be allowed to change teams
    eTeam *NextTeam()    const { return nextTeam; }				// return the team I will be next round
    eTeam *CurrentTeam() const { return currentTeam; }		// return the team I am in
    int  TeamListID() const { return teamListID; }		// return my position in the team
    int  ListID() const { return listID; }              // return my position in the player's list
    void SetShuffleWish( int pos ); 	                 //!< sets a desired team position

    eTeam * FindDefaultTeam();                                   // find a good default team for us
    void SetDefaultTeam();                                              // register me in a good default team
    void SetTeamForce(eTeam* team );            // register me in the given team without checks
    void SetTeam(eTeam* team);                  // register me in the given team (callable on the server)
    void SetTeamWish(eTeam* team); 				// express the wish to be part of the given team (always callable)
    void SetTeamname(const char *);             // set teamname to be used for my own team
    void UpdateTeamForce();						// update team membership without checks
    void UpdateTeam();							// update team membership

    eTeamSet const & GetInvitations() const ;   //!< teams this player is invited to

    void CreateNewTeam(); 	    				// create a new team and join it (on the server)
    void CreateNewTeamWish();	 				// express the wish to create a new team and join it
    virtual void ReceiveControlNet(nMessage &m);// receive the team control wish

    static bool Enemies( ePlayerNetID const * a, ePlayerNetID const * b ); //!< determines whether two players are opponents and can score points against each other

    // print out an understandable name in to s
    virtual void 			PrintName(tString &s) const;

    virtual bool 			AcceptClientSync() const;
    static  bool 			AcceptClientSyncStatic();
    virtual void 			WriteSync(nMessage &m);
    virtual void 			ReadSync(nMessage &m);
    virtual nDescriptor&	CreatorDescriptor() const;
    virtual void			InitAfterCreation();
    virtual bool			ClearToTransmit(int user) const;

    virtual void 			NewObject(){}        				// called when we control a new object
    virtual void 			RightBeforeDeath(int triesLeft){} 	// is called right before the vehicle gets destroyed.


    void RemoveFromGame();
    void ControlObject(eNetGameObject *c, bool local = false);
    void ClearObject();
    void ClearRespawn();

    void Greet();

    // suspend the player from playing, forcing him to spectate
    void Suspend( int rounds = 5, tString reason = tString(""));
#ifdef KRAWALL_SERVER
    void Authenticate( tString const & authName,
                       tAccessLevel accessLevel = tAccessLevel_Authenticated,
                       ePlayerNetID const * admin = 0,
                       bool messages = true );    //!< make the authentification valid
    void DeAuthenticate( ePlayerNetID const * admin = 0 );  //!< make the authentification invalid
    bool IsAuthenticated() const;                     //!< is the authentification valid?
#endif

    static void RequestScheduledLogins();  //!< initiates login processes for all pending wishes

    bool IsActive() const { return !disconnected; }

    bool IsSilenced( void ) const { return silenced_; }
    void SetSilenced( bool silenced ); // { silenced_ = silenced; }

    // only for the menu
    bool& AccessSilenced( void ) { return silenced_; }

    bool IsSuspended ( void ) { return suspended_ > 0; }
    bool IsSuspended ( void ) const { return suspended_ > 0; }
    int RoundsSuspended() { return suspended_; }
    int RoundsSuspended() const { return suspended_; }
    tString ReasonSuspended() { return suspendReason_; }

    static void SilenceMenu();				// menu where you can silence players
    static void PoliceMenu();				// menu where you can silence and kick players

    virtual bool IsHuman() const { return true; }

    void Activity(); // call it if this player just showed some activity.
    REAL LastActivity(); //!< returns how long the last activity of this player was ago
    REAL ChattingTime() const; //!< returns how long the player has been chatting
    eNetGameObject *Object() const;

    void RequestSync(bool ack=true);
    void RequestSyncForce(bool ack=true);

    //void SetRubber(ePlayerNetID *player, REAL rubber);
    void AddScore(int points);
    void AddScore(int points, const tOutput& reasonwin, const tOutput& reasonlose, bool shouldPrint=true);
    int Score()const {return score;}
    int TotalScore() const;
    static void ResetScoreDifferences(); //<! Resets the last stored score so ScoreDifferences takes this as a reference time
    static void LogScoreDifferences();   //<! Logs accumulated scores of all players since the last call to ResetScoreDifferences() to ladderlog.txt
    static void UpdateSuspensions();     //<! Decrements the number of rounds players are suspended for
    static void UpdateShuffleSpamTesters();    //<! Reset shuffle spam checks
    void LogScoreDifference();           //<! Logs accumulated scores since the last call to ResetScoreDifferences() to ladderlog.txt
    static void LogMatchScores();         //<! Logs match scores of all players to ladderlog.txt

    void AnalyzeTiming( REAL timing );   //<! analzye a timing event for timebot detection

    static void SortByScore(); // brings the players into the right order
    static tString Ranking( int MAX=12, bool cut = true );     // returns a ranking list
    static void RankingLadderLog();     // writes a small ranking list to ladderlog
    static void GridPosLadderLog();     // writes a cycle positions to ladderlog
    static void OnlineStatsLadderLog(); //  writes the online players, teams and the numbers
    static void  ResetScore();  // resets the ranking list

    static ePlayerNetID* HighestScoringPlayer();

    static void scheduleNameChange();

    static void DisplayScores(); // display scores on the screen

    void GreetHighscores(tString &s); // tell him his positions in the
    // highscore lists (defined in game.cpp)

    static ePlayerNetID * ReadPlayer( std::istream & s ); //!< reads a player from the stream

    static void Update(int playerID);           // creates ePlayerNetIDs for new players
    static void Update(ePlayer *updatePlayer = nullptr);           // creates ePlayerNetIDs for new players
    static void ForcedUpdate(ePlayer *updatePlayer = nullptr);           // creates ePlayerNetIDs for new players
    static void ForcedUpdate(int playerID);           // creates ePlayerNetIDs for new players
    // and destroys those of players that have left

#ifdef KRAWALL_SERVER
    static tAccessLevel AccessLevelRequiredToPlay(); // is authentication required to play on this server?
#endif

    static bool WaitToLeaveChat(); //!< waits for players to leave chat state. Returns true if the caller should wait to proceed with whatever he wants to do.

    static void RemoveChatbots(); //!< removes chatbots and idling players from the game

    static void CompleteRebuild(); // same as above, but rebuilds every ePlayerNetID.
    static void ClearAll(); // deletes all ePlayerNetIDs.
    static void Clear(ePlayer *local_p); // deletes all ePlayerNetIDs.
    static void SpectateAll( bool spectate=true ); // puts all players into spectator mode.

    static void ThrowOutDisconnected(); // get rid of everyone that disconnected from the game

    void GetScoreFromDisconnectedCopy(); // get the player's data from the previous login

    void Chat(const tString &s, bool chatBotMessage = false);

    nTimeAbsolute GetTimeCreated() const { return timeCreated_; }

    virtual void Color( REAL&r, REAL&g, REAL&b ) const;
    virtual void TrailColor( REAL&r, REAL&g, REAL&b ) const;

    //Remote Admin add-ins...
    bool IsLoggedIn() const { return GetAccessLevel() < tAccessLevel_Moderator; }
    void BeLoggedIn() { SetAccessLevel( tAccessLevel_Admin ); }
    void BeNotLoggedIn() { SetAccessLevel( tAccessLevel_Program ); }
    tAccessLevel GetLastAccessLevel() const { return lastAccessLevel; }

    //Login Checker
    bool HasLoggedIn() { return hasLoggedIn_; }
    void SetLoggedIn(bool newValue) { hasLoggedIn_ = newValue; }

    void DropFlag();
    void PassFlag(std::istream &s);

    static ePlayerNetID * FindPlayerByName( tString const & name, ePlayerNetID * requester = 0, bool print=true ); //!< finds a player by name using lax name matching. Reports errors to the console or to the requesting player.
    static ePlayerNetID * FindPlayerById(int owner_id);

    void UpdateName();                                           //!< update the player name from either the client's wishes, either the admin's wishes.
    static void FilterName( tString const & in, tString & out ); //!< filters a name (removes unprintables, color codes and spaces)
    static tString FilterName( tString const & in );             //!< filters a name (removes unprintables, color codes and spaces)
    bool IsAllowedToRename ( void );                             //!< tells if the user can rename or not, takes care about everything
    void AllowRename( bool allow );                              //!< Allows a player to rename (or not)

    static bool HasRenameCapability ( ePlayerNetID const *, ePlayerNetID const * admin ); //!< Checks if the admin can use the RENAME command. Used in IsAllowedToRename()

    void LogActivity(int activity_type);    //!< Log the activity of the player currently in motion
private:
    tColoredString  nameFromClient_;        //!< this player's name as the client wants it to be. Avoid using it when possilbe.
    tColoredString  nameFromServer_;        //!< this player's name as the server wants it to be. Avoid using it when possilbe.
    tColoredString  nameFromAdmin_;         //!< this player's name as the admin wants it to be. Avoid using it when possilbe.
    tString         userName_;              //!< this player's name, cleared for system logs. Use for writing to files or comparing with admin input.
public:
    tColoredString  coloredNickname;
    tString         nickname;
    tString         chatBotNickname;
    tColoredString  coloredName_;           //!< this player's name, cleared by the server. Use this for onscreen screen display.
    tString         name_;                  //!< this player's name without colors.
    bool lastNameDoesntMatch,
         nameDoesntMatch;
private:

#ifdef KRAWALL_SERVER
    tString         rawAuthenticatedName_;  //!< the raw authenticated name in user@authority form.
#endif
    //! Race Hack
    tString         authenticatedname;

    bool            hasLoggedIn_;

    REAL            wait_;                  //!< time in seconds WaitToLeaveChat() will wait for this player

    void CreateVoter();						// create our voter or find it
    void			MyInitAfterCreation();

protected:
    virtual nMachine & DoGetMachine() const;  //!< returns the machine this object belongs to

    // private:
    //	virtual void AddRef();
    //	virtual void Release();

    // accessors
public:
    inline tColoredString const & GetNameFromClient( void ) const;	//!< Gets this player's name as the client wants it to be. Avoid using it when possilbe.
    inline tColoredString const & GetNameFromServer( void ) const;	//!< Gets this player's name as the client wants it to be. Avoid using it when possilbe.
    inline tColoredString const & GetNameFromAdmin( void ) const;	//!< Gets this player's name as the client wants it to be. Avoid using it when possilbe.
    inline ePlayerNetID const & GetNameFromClient( tColoredString & nameFromClient ) const;	//!< Gets this player's name as the client wants it to be. Avoid using it when possilbe.
    inline tColoredString const & GetColoredName( void ) const;	//!< Gets this player's name, cleared by the server. Use this for onscreen screen display.
    inline tColoredString const & GetColoredNickName( void ) const;	//!< Gets this player's name, cleared by the server. Use this for onscreen screen display.
    inline ePlayerNetID const & GetColoredName( tColoredString & coloredName ) const;	//!< Gets this player's name, cleared by the server. Use this for onscreen screen display.
    inline tString const & GetName( void ) const;	//!< Gets this player's name without colors.
    inline tString const & GetNickName( void ) const;	//!< Gets this player's name without colors.
    inline ePlayerNetID const & GetName( tString & name ) const;	//!< Gets this player's name without colors.

    inline tString const & GetUserName( void ) const;	//!< Gets this player's full name. Use for writing to files or comparing with admin input.
    inline ePlayerNetID const & GetUserName( tString & userName ) const;	//!< Gets this player's name, cleared for system logs. Use for writing to files or comparing with admin input.

    tString const & GetLogName( void ) const{ return GetUserName(); }	//!< Gets this player's name, cleared for system logs (with escaped special characters). Use for writing to files.
    tString GetFilteredAuthenticatedName( void ) const;	//!< Gets the filtered, ecaped authentication name
#ifdef KRAWALL_SERVER
    tString const & GetRawAuthenticatedName( void ) const{ return rawAuthenticatedName_; }	//!< Gets the raw, unescaped authentication name
    void SetRawAuthenticatedName( tString const & name ){ if ( !IsAuthenticated()) rawAuthenticatedName_ = name; }	//!< Sets the raw, unescaped authentication name
#endif

    //! Race Hack
    tString const & GetAuthenticatedName() const { return authenticatedname; }
    void SetAuthenticatedName(tString name)      { authenticatedname = name; }

    ePlayerNetID & SetName( tString const & name ); //!< Sets this player's name. Sets processed names (colored, username, nameFromCLient) as well.
    ePlayerNetID & SetName( char    const * name ); //!< Sets this player's name. Sets processed names (colored, username, nameFromCLient) as well.
    ePlayerNetID & SetName( tString const & name , bool force ); //!< Sets this player's name. Sets processed names (colored, username, nameFromCLient) as well.
    ePlayerNetID & ForceName( tString const & name ); //!< Forces this player's name. Forces processed names (colored, username, nameFromCLient) as well.

    inline ePlayerNetID & SetUserName( tString const & userName );  //!< Sets this player's name, cleared for system logs. Use for writing to files or comparing with admin input. The other names stay unaffected.

private:
    inline ePlayerNetID & SetNameFromClient( tColoredString const & nameFromClient );   //!< Sets this player's name as the client wants it to be. Avoid using it when possilbe.
    inline ePlayerNetID & SetColoredName( tColoredString const & coloredName ); //!< Sets this player's name, cleared by the server. Use this for onscreen screen display.
};

extern tList<ePlayerNetID> se_PlayerNetIDs;
extern int    sr_viewportBelongsToPlayer[MAX_VIEWPORTS];

void se_ChatState( ePlayerNetID::ChatFlags flag, bool cs, ePlayerNetID *player = NULL);

void se_SaveToScoreFile( tOutput const & out );  //!< writes something to scorelog.txt
void se_SaveToChatLog( tOutput const & out );  //!< writes something to chatlog.txt (if enabled) and/or ladderlog
void se_SaveToLadderLog( tOutput const & out );  //!< writes something to ladderlog.txt
void se_SaveToChatLogC( tOutput const &out ); //!< writes something to chatlog_color.txt

//! create a global instance of this to write stuff to ladderlog.txt
class eLadderLogWriter {
    static std::list<eLadderLogWriter *> &writers();
    tString id;
    bool enabled;
    tSettingItem<bool> *conf;
    tColoredString cache;
public:
    eLadderLogWriter(char const *ID, bool enabledByDefault);
    ~eLadderLogWriter();
    //! append a field to the current message. Spaces are added automatically.
    template<typename T> eLadderLogWriter &operator<<(T const &s) {
        if(enabled) {
            cache << ' ' << s;
        }
        return *this;
    }
    void write(); //!< send to ladderlog and clear message

    bool isEnabled() { return enabled; } //!< check this if you're going to make expensive calculations for ladderlog output

    static void setAll(bool enabled); //!< enable or disable all writers
};

enum
{
    ACTIVITY_LEFT,
    ACTIVITY_ENTERED_GRID,
    ACTIVITY_ENTERED_SPECTATOR,
    ACTIVITY_JOINED_SPECTATOR_FROM_GRID,
    ACTIVITY_JOINED_GAME_FROM_SPECTATOR,
    ACTIVITY_FINISHED_RACE,
    ACTIVITY_DIED
};

tColoredString & operator << (tColoredString &s,const ePlayer &p);
tColoredString & operator << (tColoredString &s,const ePlayerNetID &p);

extern int pingCharity;

void se_AutoShowScores(); // show scores based on automated decision
void se_UserShowScores(bool show); // show scores based on user input
void se_SetShowScoresAuto(bool a); // disable/enable auto show scores

//Password stuff
void se_DeletePasswords();
extern int se_PasswordStorageMode; // 0: store in memory, -1: don't store, 1: store on file

tOutput& operator << (tOutput& o, const ePlayerNetID& p);

// greeting callback
class eCallbackGreeting: public tCallbackString
{
    static tCallbackString *anchor;
    static ePlayerNetID* greeted;

public:
    static tString Greet(ePlayerNetID* player);
    static ePlayerNetID* Greeted(){return greeted;}

    eCallbackGreeting(STRINGRETFUNC* f);
};

void ForceName ( std::istream & s );

// ******************************************************************************************
// *
// *	GetNameFromClient
// *
// ******************************************************************************************
//!
//!		@return		this player's name as the client wants it to be. Avoid using it when possilbe.
//!
// ******************************************************************************************

tColoredString const & ePlayerNetID::GetNameFromClient( void ) const
{
    return this->nameFromClient_;
}

tColoredString const & ePlayerNetID::GetNameFromServer( void ) const
{
    return this->nameFromServer_;
}

tColoredString const & ePlayerNetID::GetNameFromAdmin( void ) const
{
    return this->nameFromAdmin_;
}

// ******************************************************************************************
// *
// *	GetNameFromClient
// *
// ******************************************************************************************
//!
//!		@param	nameFromClient	this player's name as the client wants it to be. Avoid using it when possilbe. to fill
//!		@return		A reference to this to allow chaining
//!
// ******************************************************************************************

ePlayerNetID const & ePlayerNetID::GetNameFromClient( tColoredString & nameFromClient ) const
{
    nameFromClient = this->nameFromClient_;
    return *this;
}

// ******************************************************************************************
// *
// *	SetNameFromClient
// *
// ******************************************************************************************
//!
//!		@param	nameFromClient	this player's name as the client wants it to be. Avoid using it when possilbe. to set
//!		@return		A reference to this to allow chaining
//!
// ******************************************************************************************

ePlayerNetID & ePlayerNetID::SetNameFromClient( tColoredString const & nameFromClient )
{
    this->nameFromClient_ = nameFromClient;
    return *this;
}

// ******************************************************************************************
// *
// *	GetColoredName
// *
// ******************************************************************************************
//!
//!		@return		this player's name, cleared by the server. Use this for onscreen screen display.
//!
// ******************************************************************************************

tColoredString const & ePlayerNetID::GetColoredName( void ) const
{
    return this->coloredName_;
}

tColoredString const & ePlayerNetID::GetColoredNickName( void ) const
{
    return this->coloredNickname.empty() ? this->coloredName_ : this->coloredNickname;
}


// ******************************************************************************************
// *
// *	GetColoredName
// *
// ******************************************************************************************
//!
//!		@param	coloredName	this player's name, cleared by the server. Use this for onscreen screen display. to fill
//!		@return		A reference to this to allow chaining
//!
// ******************************************************************************************

ePlayerNetID const & ePlayerNetID::GetColoredName( tColoredString & coloredName ) const
{
    coloredName = this->coloredName_;
    return *this;
}

// ******************************************************************************************
// *
// *	SetColoredName
// *
// ******************************************************************************************
//!
//!		@param	coloredName	this player's name, cleared by the server. Use this for onscreen screen display. to set
//!		@return		A reference to this to allow chaining
//!
// ******************************************************************************************

ePlayerNetID & ePlayerNetID::SetColoredName( tColoredString const & coloredName )
{
    this->coloredName_ = coloredName;
    return *this;
}

// ******************************************************************************************
// *
// *	GetName
// *
// ******************************************************************************************
//!
//!		@return		this player's name without colors.
//!
// ******************************************************************************************

tString const & ePlayerNetID::GetName( void ) const
{
    return this->name_;
}

tString const & ePlayerNetID::GetNickName( void ) const
{
    return this->nickname.empty() ? this->name_ : this->nickname;
}

// ******************************************************************************************
// *
// *	GetName
// *
// ******************************************************************************************
//!
//!		@param	name	this player's name without colors. to fill
//!		@return		A reference to this to allow chaining
//!
// ******************************************************************************************

ePlayerNetID const &ePlayerNetID::GetName(tString &name) const
{
    name = this->name_;
    return *this;
}

// ******************************************************************************************
// *
// *	GetUserName
// *
// ******************************************************************************************
//!
//!		@return		this player's name, cleared for system logs. Use for writing to files or comparing with admin input.
//!
// ******************************************************************************************

tString const & ePlayerNetID::GetUserName( void ) const
{
    return this->userName_;
}

// ******************************************************************************************
// *
// *	GetUserName
// *
// ******************************************************************************************
//!
//!		@param	userName	this player's name, cleared for system logs. Use for writing to files or comparing with admin input. to fill
//!		@return		A reference to this to allow chaining
//!
// ******************************************************************************************

ePlayerNetID const & ePlayerNetID::GetUserName( tString & userName ) const
{
    userName = this->userName_;
    return *this;
}

// ******************************************************************************************
// *
// *	SetUserName
// *
// ******************************************************************************************
//!
//!		@param	userName	this player's name, cleared for system logs. Use for writing to files or comparing with admin input. to set
//!		@return		A reference to this to allow chaining
//!
// ******************************************************************************************

ePlayerNetID & ePlayerNetID::SetUserName( tString const & userName )
{
    this->userName_ = userName;
    return *this;
}

ePlayerNetID *se_GetLocalPlayer();
ePlayerNetID *se_GetLocalPlayer(int ID);
std::vector<ePlayerNetID *> se_GetLocalPlayers();

extern bool se_highlightNames, se_tabCompletion, se_tabCompletionWithColors,se_tabCompletionColon;


static ePlayer * se_chatterPlanned=NULL;
static ePlayer * se_chatter =NULL;
static tString se_say;

class eMessageTracker
{
public:
    std::deque<tString> outgoingMessages;
    std::deque<tString> incomingMessages;
    size_t maxSize;
    std::string name;

public:
    explicit eMessageTracker(std::string name = "", size_t maxSizeVal = 5)
        : maxSize(maxSizeVal), name(name)
    {
    }

    void AddOutgoingMessage(const tString &msg);

    void AddIncomingMessage(const tString &msg);

    bool CheckIfSilenced();
    
};

extern eMessageTracker ePlayerMessages;


#endif
