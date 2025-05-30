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

#include "tMemManager.h"
#include "tInitExit.h"
#include "nSimulatePing.h"
#include "nConfig.h"
#include "nNetwork.h"
#include "nServerInfo.h"
#include "tConsole.h"
#include "tDirectories.h"
#include "nSocket.h"
#include "nConfig.h"
#include "nKrawall.h"
#include "tSysTime.h"
#include "tRecorder.h"
#include "tRandom.h"
#include <stdlib.h>
#include <fstream>
#include "tMath.h"
#include "ePlayer.h"
#include "ePlayerStats.h"
#include "eChatBot.h"
#include <string.h>
#include "../tron/gHelper/gHelperUtilities.h"
#include "../tron/gGame.h"

#ifndef WIN32
#include  <netinet/in.h>
#else
#include  <windows.h>
#endif

#include <deque>

// my IP address. Master server/game server hopefully tell me a correct one.
static tString sn_myAddress ("*.*.*.*:*");
tString const & sn_GetMyAddress()
{
    return sn_myAddress;
}

//! checks wheter a given address is on the user's LAN (or on loopback).
bool sn_IsLANAddress( tString const & address )
{
    if ( address.StartsWith("127.") || address.StartsWith("10.") || address.StartsWith("192.168.") )
    {
        // easy LANs. Accept the client sent IP, we don't know our own LAN address.
        return true;
    }

    if( address.StartsWith( "172." ) && address[6] == '.' )
    {
        // more complicated LAN :)
        int second = address.SubStr(4,2).toInt();
        if ( 16 <= second && second < 32 )
        {
            return true;
        }
    }

    return false;
}

// debug watches
#ifdef DEBUG
nMessage* sn_WatchMessage = NULL;
unsigned int sn_WatchMessageID = 76;
#endif

#define NO_ACK

tString sn_bigBrotherString;
// tString sn_greeting[5];  //made 4 = 5 (lol i broke the laws of maths. subby),  k's bug fix


#ifdef TOP_SOURCE_DIR
#include "nTrueVersion.h"
#endif

#ifndef TRUE_ARMAGETRONAD_VERSION
#define TRUE_ARMAGETRONAD_VERSION VERSION
#endif

tString sn_programVersion (TRUE_ARMAGETRONAD_VERSION)    ;

tString sn_serverName("Unnamed Server");

const unsigned int sn_defaultPort = 4534;
unsigned int sn_serverPort = 4534;
bool sn_decorateTS = false;

tString net_hostip("ANY");

bool big_brother=true;
static tConfItem<bool> sn_bb("BIG_BROTHER",big_brother);

static tConfItemLine sn_sn("SERVER_NAME", sn_serverName);

static tConfItem<int> sn_sport("SERVER_PORT",reinterpret_cast<int&>(sn_serverPort));

static tConfItemLine sn_sbtip("SERVER_IP", net_hostip);

void sn_DisconnectUserNoWarn(int i, const tOutput& reason, nServerInfoBase * redirectTo = 0 );


tString sn_ourRealIPS = tString("");
static tConfItem<tString> sn_ourRealIPSConf("OUR_REAL_IPS", sn_ourRealIPS);

tString sn_disallowJoinWithIP = tString("");
static tConfItem<tString> sn_disallowJoinWithIPConf("DISALLOW_JOIN_WITH_IP", sn_disallowJoinWithIP);

bool se_playerMessageTriggersBannedAction = false;
static tConfItem<bool> se_playerMessageTriggersBannedActionConf("PLAYER_MESSAGE_TRIGGER_BANNED_ACTION", se_playerMessageTriggersBannedAction);

bool sn_bannedWatch = false;
static tConfItem<bool> sn_bannedWatchConf("PLAYER_WATCH_BANNED", sn_bannedWatch);
tString sn_bannedWatchStr = tString("");
static tConfItem<tString> sn_bannedWatchStrConf("PLAYER_WATCH_BANNED_STRING", sn_bannedWatchStr);
bool sn_bannedWatchQuit = false;
static tConfItem<bool> sn_bannedWatchQuitConf("PLAYER_WATCH_BANNED_QUIT", sn_bannedWatchQuit);
REAL sn_bannedWatchQuitTime = 5;
static tConfItem<REAL> sn_bannedWatchQuitTimeConf("PLAYER_WATCH_BANNED_QUIT_TIME", sn_bannedWatchQuitTime);

bool sn_loginDenalWatch = false;
static tConfItem<bool> sn_loginDenalWatchConf("PLAYER_WATCH_LOGIN_DENIAL", sn_loginDenalWatch);
bool sn_loginDenalWatchQuit = false;
static tConfItem<bool> sn_loginDenalWatchQuitConf("PLAYER_WATCH_LOGIN_DENIAL_QUIT", sn_loginDenalWatchQuit);
tString sn_loginDenalWatchStr = tString("");
static tConfItem<tString> sn_loginDenalWatchStrConf("PLAYER_WATCH_LOGIN_DENIAL_STRINGS", sn_loginDenalWatchStr);

bool sn_playerUnableToRenameWatch = false;
static tConfItem<bool> sn_playerUnableToRenameWatchConf("PLAYER_WATCH_RENAME", sn_playerUnableToRenameWatch);
bool sn_playerUnableToRenameWatchRebuild = true;
static tConfItem<bool> sn_playerUnableToRenameWatchRebuildConf("PLAYER_WATCH_RENAME_REBUILD", sn_playerUnableToRenameWatchRebuild);

bool sn_playerSuspendWatch = false;
static tConfItem<bool> sn_playerSuspendWatchConf("PLAYER_WATCH_SUSPENDED", sn_playerSuspendWatch);
bool sn_playerSuspendWatchQuit = false;
static tConfItem<bool> sn_playerSuspendWatchQuitConf("PLAYER_WATCH_SUSPENDED_QUIT", sn_playerSuspendWatchQuit);
bool sn_playerSuspendWatchQuitWaitForAvoidPlayers = false;
static tConfItem<bool> sn_playerSuspendWatchQuitWaitForAvoidPlayersConf("PLAYER_WATCH_SUSPENDED_QUIT_WAIT_FOR_AVOID_PLAYERS", sn_playerSuspendWatchQuitWaitForAvoidPlayers);

int sn_defaultDelay=10000;

//! pause a bit, abort pause on network activity
void sn_Delay()
{
    sn_BasicNetworkSystem.Select( sn_defaultDelay / 1000000.0 );
    tAdvanceFrame();
}

int sn_maxRateIn=32; // maximum data rate in kb/s
int sn_maxRateOut=16; // maximum output data rate in kb/s

static nConnectError sn_Error = nOK;

//tArray<unsigned short> send_buffer[MAXCLIENTS+2];
//REAL planned_rate_control[MAXCLIENTS+2];
//REAL rate_control[MAXCLIENTS+2];
//unsigned short  rate[MAXCLIENTS+2];

// from gGame.C
//extern unsigned short client_gamestate[MAXCLIENTS+2];

bool deb_net=false;

static REAL maxTimeout=1;  // the maximal timeout in seconds
static REAL minTimeout=.01;  // the minimal timeout in seconds
static REAL pingTimeout=1; // the normal timeout in multiples of the ping
static REAL pingVarianceTimeout=1; // the normal timeout in multiples of the ping variance
static REAL zeroTimeout=.01; // additional timeout of first packet

static REAL sn_GetTimeout( int user )
{
    tASSERT( user >= 0 && user <= MAXCLIENTS+1 );

    nPingAverager & averager = sn_Connections[ user ].ping;

    REAL timeout = pingTimeout * averager.GetPing() + pingVarianceTimeout * sqrtf( averager.GetSnailAverager().GetAverageVariance() );

    if ( timeout < minTimeout )
        timeout = minTimeout;
    if ( timeout > maxTimeout )
        timeout = maxTimeout;

    return timeout;
}

#ifndef DEBUG
static REAL killTimeout=30;
#else
static REAL killTimeout=30;
#endif

static const int kickOnDemandTimeout = 10;

static bool send_again_warn=false;

#ifdef DEBUG
static int simulate_loss=0;
#else
//static int simulate_loss=0;
#endif

int sn_maxNoAck=100; // the maximum number of not ack messages
// before more are send

//int sn_ackPending[MAXCLIENTS+2];
// int sn_ackAckPending[MAXCLIENTS+2];

//static nMessage * ack_mess[MAXCLIENTS+2];

static nNetState current_state;
//int sn_sockets[MAXCLIENTS+2];  // server mode:
// elements 1...MAXCLIENTS are the incoming connections,
// client mode: element 0 connects to the server.
// element MAXCLIENTS+1: currently logging in

nConnectionInfo sn_Connections[MAXCLIENTS+2];

static nAddress peers[MAXCLIENTS+2]; // the same logic for the peer adresses.
static nAddress lastPeers[MAXCLIENTS+2]; // the peers last connected to each slot
static int timeouts[MAXCLIENTS+2];

#define ACKBACK 1000
static unsigned short lastacks[MAXCLIENTS+2][ACKBACK];
static unsigned short lastackPos[MAXCLIENTS+2];
static unsigned short highest_ack[MAXCLIENTS+2];


//********************************************************
// Version control
//********************************************************

static int sn_MaxBackwardsCompatibility = 1000;
static tSettingItem<int> sn_mxc("BACKWARD_COMPATIBILITY",sn_MaxBackwardsCompatibility);

static int sn_minVersion = 0;
static tSettingItem<int> sn_miv("MIN_PROTOCOL_VERSION",sn_minVersion);

static int sn_maxVersion = 0;
static tSettingItem<int> sn_mav("MAX_PROTOCOL_VERSION",sn_maxVersion);

static int sn_newFeatureDelay = 0;
static tSettingItem<int> sn_nfd("NEW_FEATURE_DELAY",sn_newFeatureDelay);

// color code strictness setting from tColor.cpp
extern bool st_verifyColorCodeStrictly;
//static nSettingItemWatched< bool > stc_verifyColorCodeStrictly( "VERIFY_COLOR_STRICT", st_verifyColorCodeStrictly, nConfItemVersionWatcher::Group_Visual, 18 );
static nSettingItem<bool> stc_verifyColorCodeStrictly("VERIFY_COLOR_STRICT",st_verifyColorCodeStrictly);

// from nConfig.cpp. Adapt version string array there to bump protocol version.
int sn_GetCurrentProtocolVersion();

static const int sn_currentProtocolVersion              = sn_GetCurrentProtocolVersion(); // the current version number of the network protocol
static const int sn_backwardCompatibleProtocolVersion 	= 0;							// the smallest version of the network protocol this program is compatible with
static nVersion sn_myVersion( sn_backwardCompatibleProtocolVersion, sn_currentProtocolVersion);
static nVersion sn_currentVersion( sn_myVersion );


const nVersion& sn_MyVersion()			// the version this progam maximally supports
{
    return sn_myVersion;
}

const nVersion& sn_CurrentVersion() 	// the version currently supported
{
    return sn_currentVersion;
}

int sn_updateVersionOverrideValue = -1;
static tConfItem<int> sn_updateVersionOverrideValueConf  = HelperCommand::tConfItem("NETWORK_VERSION_OVERRIDE_VALUE", sn_updateVersionOverrideValue);

void updateVersionOverride(int version)
{
    nVersion newVersion( sn_backwardCompatibleProtocolVersion, version);
    sn_updateVersionOverrideValue = version;
    sn_currentVersion = newVersion;
    sn_myVersion      = newVersion;
}

void updateVersionOverride(std::istream &s)
{
    tString params;
    params.ReadLine(s, true);

    if (params.empty())
    {
        tOutput o;
        o.SetTemplateParameter(1, "NETWORK_VERSION_OVERRIDE");
        o.SetTemplateParameter(2, sn_updateVersionOverrideValue);
        o << "$config_message_info";
        con << o;
        return;
    }

    if (!params.isNumber()){
        tOutput o;
        o.SetTemplateParameter(1, "NETWORK_VERSION_OVERRIDE");
        o << "$config_error_read";
        con << o;
        return;
    }

    int newVersion = params.toInt();

    if (newVersion != sn_updateVersionOverrideValue)
    {
        tOutput o;
        o.SetTemplateParameter(1, "NETWORK_VERSION_OVERRIDE");
        o.SetTemplateParameter(2, sn_updateVersionOverrideValue);
        o.SetTemplateParameter(3, newVersion);
        o << "$config_value_changed";
        con << o;
    }

    updateVersionOverride(newVersion);
}

static tConfItemFunc updateVersionOverrideConf = HelperCommand::tConfItemFunc("NETWORK_VERSION_OVERRIDE", &updateVersionOverride);


nVersion::nVersion()
{
    min_=0;
    max_=0;
}

nVersion::nVersion( int min, int max )
{
    tASSERT( min <= max );
    min_=min;
    max_=max;
}

bool nVersion::Supported( int version ) const	// check if a particular version is supported
{
    tASSERT( min_ <= max_ );
    return version >= min_ && version <= max_;
}

bool nVersion::Merge( const nVersion& a,
                      const nVersion& b)	// merges two versions to one; the new version supports only features both versions understand. false is returned if no common denominator could be found
{
    int min = a.min_;
    if ( min < b.min_ )
    {
        min = b.min_;
    }

    int max = a.max_;
    if ( max > b.max_ )
    {
        max = b.max_;
    }

    if ( min <= max )
    {
        min_ = min;
        max_ = max;
        return true;
    }
    else
    {
        return false;
    }
}

bool nVersion::operator == ( const nVersion& other )
{
    return this->max_ == other.max_ && this->min_ == other.min_;
}

nMessage& operator >> ( nMessage& m, nVersion& ver )
{
    int min,max;
    m >> min;
    m >> max;

    ver = nVersion( min, max );

    return m;
}

nMessage& operator << ( nMessage& m, const nVersion& ver )
{
    m << ver.Min();
    m << ver.Max();

    return m;
}

std::istream& operator >> ( std::istream& s, nVersion& ver )
{
    int min,max;
    s >> min;
    s >> max;

    ver = nVersion( min, max );

    return s;
}

std::ostream& operator << ( std::ostream& s, const nVersion& ver )
{
    s << ver.Min() << " ";
    s << ver.Max();

    return s;
}

nVersionFeature::nVersionFeature( int min, int max ) // creates a feature that is supported from version min to max; values of -1 indicate no border
{
    tASSERT( min >= sn_MyVersion().Min() );
    tASSERT( max < 0 || max <= sn_MyVersion().Max() );

    min_ = min;
    max_ = max;
}

bool nVersionFeature::Supported() const
{
    return ( min_ < 0 || sn_CurrentVersion().Max() >= min_ ) &&  ( max_ < 0 || sn_CurrentVersion().Min() <= max_ );
}

bool nVersionFeature::Supported( int client ) const
{
    if ( client < 0 || client > MAXCLIENTS )
        return false;

    // the version to check the feature for
    const nVersion * version = &sn_CurrentVersion();

    if ( sn_GetNetState() == nCLIENT )
    {
        // clientside code: override the currently active version with the server version ( if that has been sent )
        if ( sn_Connections[0].version.Max() > 0 )
            version = &sn_Connections[0].version;
    }
    else
    {
        // serverside code: override version to use with the client's version
        version = &sn_Connections[ client ].version;
    }

    // see if the feature is supported
    return ( min_ < 0 || version->Max() >= min_ ) &&  ( max_ < 0 || version->Min() <= max_ );
}

void handle_version_control( nMessage& m )
{
    if ( sn_GetNetState() == nCLIENT )
    {
        m >> sn_currentVersion;

        // inform configuration of changes
        nConfItemVersionWatcher::OnVersionChange( sn_currentVersion );
    }
}

nDescriptor versionControl(10, handle_version_control,"version" );

void sn_UpdateCurrentVersion()
{
    // update the current version from the native version and the versions of all attached clients

    // allow maximally sn_MaxBackwardsCompatibility old versions to connect
    int min = sn_myVersion.Max() - sn_MaxBackwardsCompatibility;
    if ( min < sn_myVersion.Min() )
        min = sn_myVersion.Min();
    if( min < sn_minVersion )
        min = sn_minVersion;

    // disable features that are too new
    int max = sn_myVersion.Max() - sn_newFeatureDelay;
    if( sn_maxVersion > 0 && max > sn_maxVersion )
        max = sn_maxVersion;
    if ( max < min )
        max = min;

    nVersion version( min, max );

    // ask configuration if version is OK
    nConfItemVersionWatcher::AdaptVersion( version );

    nVersion maxVersion = version;

    if ( sn_GetNetState() == nCLIENT )
    {
        sn_currentVersion = version;
        return;
    }

    for ( int i = MAXCLIENTS; i>0; --i )
    {
        const nConnectionInfo& info = sn_Connections[i];
        if ( info.socket )
        {
            if ( ! version.Merge( version, info.version ) )
            {
                // kick user; it has gotten incompatible.
                static bool recurse = true;
                if ( recurse )
                {
                    recurse = false;
                    sn_DisconnectUser( i, "$network_kill_incompatible" );
                    recurse = true;
                }

                version = maxVersion;
            }
        }
    }

    // inform configuration of changes
    nConfItemVersionWatcher::OnVersionChange( version );

    if ( version != sn_currentVersion )
    {
        sn_currentVersion = version;

        nMessage* m = tNEW( nMessage )( versionControl );
        (*m) << version;

        m->BroadCast();
    }
}

//********************************************************

nConnectError sn_GetLastError()
{
    nConnectError ret = sn_Error;
    sn_Error = nOK;
    return ret;
}



// REAL sn_ping[MAXCLIENTS+2];

static void reset_last_acks(int i){
    for(int j=ACKBACK-1;j>=0;j--)
        lastacks[i][j]=0;
    lastackPos[i]=0;
    highest_ack[i]=0;
}


//#ifndef DEBUG
int sn_maxClients=MAXCLIENTS;

bool restrictMaxClients( int const &newValue )
{
    if (newValue > MAXCLIENTS)
    {
        tOutput o;
        o.SetTemplateParameter(1, MAXCLIENTS);
        o << "$max_clients_high";
        con << o << '\n';
        return false;
    }
    else if (newValue <= 0)
    {
        tOutput o;
        o.SetTemplateParameter(1, newValue);
        o << "$max_clients_low";
        con << o << '\n';
        return false;
    }
    return true;
}

static tSettingItem< int > sn_maxClientsConf( "MAX_CLIENTS", sn_maxClients, &restrictMaxClients );

int sn_allowSameIPCountSoft=4;
static tSettingItem< int > sn_allowSameIPCountSoftConf( "MAX_CLIENTS_SAME_IP_SOFT", sn_allowSameIPCountSoft );

int sn_allowSameIPCountHard=8;
static tSettingItem< int > sn_allowSameIPCountHardConf( "MAX_CLIENTS_SAME_IP_HARD", sn_allowSameIPCountHard );

//#else
//int maxclients=1;
//#endif

int sn_myNetID=0; // our network identification:  0: server
//                                            1..MAXCLIENTS: client

#define IDS_RESERVED 16		 // number of message IDs reserved for special purposes: id 0 is reserved for no-ack messages.
unsigned short current_id=1; // current running network number


// the classes that are responsible for the queuing of network send tEvents:
class planned_send:public tHeapElement{
protected:
    int peer;
public:
    planned_send(REAL priority,int peer);
    ~planned_send();

    virtual tHeapBase *Heap() const; // in wich heap are we?

    // change our priority:
    void add_to_priority(REAL diff);

    // what is to be done if the sceduled tEvent is executed?
    virtual void execute()=0;
};

class nMessage_planned_send:public planned_send{
    tCONTROLLED_PTR(nMessage) m;
    bool ack;

public:
    nMessage_planned_send(nMessage *m,REAL priority,bool ack,int peer);
    ~nMessage_planned_send();

    virtual void execute();
};

// *************************************************************

unsigned short nDescriptor::s_nextID(1);

static nDescriptor* nDescriptor_anchor;

std::vector<nDescriptor*>& nDescriptor::getTrackedDescriptors() {
    static std::vector<nDescriptor*> trackedDescriptors(MAXDESCRIPTORS, nullptr);
    return trackedDescriptors;
}
nDescriptor::nDescriptor(unsigned short identification,
                         nHandler *handle, const char *Name, bool awl)
        :tListItem<nDescriptor>(nDescriptor_anchor),
        id(identification), handler(handle), name(Name), acceptWithoutLogin(awl),
        lastReceivedTime(0)
{
    if (MAXDESCRIPTORS <= id || descriptors[id] != nullptr) {
        con << "Descriptor " << id << " already used!\n";
        exit(-1);
    }
    s_nextID = id + 1;

    descriptors[id] = this;
    getTrackedDescriptors()[id] = this;
}



int nCurrentSenderID::currentSenderID_ = 0;

bool sg_descriptorsShow = false;
static tConfItem<bool> sg_descriptorsShowConf = HelperCommand::tConfItem("DESCRIPTOR_HANDLED_SHOW",sg_descriptorsShow);

bool sg_descriptorsTrackLastDataAlways = false;
static tConfItem<bool> sg_descriptorsTrackLastDataAlwaysConf = HelperCommand::tConfItem("DESCRIPTOR_HANDLED_TRACK_LAST_DATA_ALWAYS",sg_descriptorsTrackLastDataAlways);

tString sg_descriptorsShowIgnoreList = tString("");
static tConfItem<tString> sg_descriptorsShowIgnoreListConf = HelperCommand::tConfItem("DESCRIPTOR_HANDLED_SHOW_IGNORE_LIST",sg_descriptorsShowIgnoreList);

void nDescriptor::HandleMessage(nMessage &message){
    // store sender ID for console
    nCurrentSenderID currentSender( message.SenderID() );
    if (sg_descriptorsShow && !tIsInList(sg_descriptorsShowIgnoreList,static_cast<int>(message.descriptor)))
        con << "Handling nDescriptor message for ID: " << message.descriptor << " (" << descriptors[message.descriptor]->name << ") :\n" << con.nMessageToString(message) << "\n";
#ifdef DEBUG_X
    if (message.descriptor>1)
        con << "RMT " << message.descriptor << "\n";
#endif

#ifndef NOEXCEPT
    try{
#endif
        nDescriptor *nd = 0;

        // z-man: security check ( thanks, Luigi Auriemma! )
        if ( message.descriptor  < MAXDESCRIPTORS )
            nd=descriptors[message.descriptor];

        if (nd){
            if ((message.SenderID() <= MAXCLIENTS) || nd->acceptWithoutLogin) {
                //nd->lastReceivedTime = tSysTimeFloat();
                if(sg_descriptorsTrackLastDataAlways || sg_descriptorsShow)
                    nd->SetLastSentData(nMessage::nMessageToDataVector(message)); // Store last sent data
                nd->handler(message);
            }
        }
#ifdef DEBUG
        else
        {
            static tArray<bool> warned;

            if (!warned[message.Descriptor()]){
                tOutput warn;
                warn.SetTemplateParameter(1, message.Descriptor());
                warn << "$network_warn_unknowndescriptor";
                con << warn;
                warned[message.Descriptor()]=true;
            }
        }
#endif
#ifndef NOEXCEPT
    }
    catch(nIgnore const &){
        // well, do nothing.
    }
    catch(nKillHim const &){
        // st_Breakpoint();
        con << tOutput("$network_error");
        sn_DisconnectUser(message.SenderID(), "$network_kill_error" );
    }

#endif
}

// *************************************************************

// random offset
static int sn_GetRandomOffset()
{
    static tReproducibleRandomizer rand;
    return rand.Get(0x7fffffff);
}

// syn cookie
static int sn_SynTimestamp()
{
    static int offset = sn_GetRandomOffset();

    return offset + int(tSysTimeFloat()/16);
}

// a cookie consists of two shorts, each transmitted as MessageID
// of two consecutive fake login accept packets. They'll be sent back
// to the server inside ONE ack message by a real, non-spoofed client.
struct nCookie
{
    unsigned short first;
    unsigned short second;

    nCookie(): first(0), second(0){}
};

static void sn_SynGenerateCookie(int stamp, nAddress const & sender, nCookie & ret)
{
    // just some random modulus.
    static const unsigned int modulo = 0x7f71fa35;
    stamp %= modulo;

    // calculate just some random checksum. Doesn't need to be any good,
    // we can change it any time should someone be able to predict it.
    int checksum = stamp;
    int mul = ( stamp & 0x7fff ) + 3;
    sockaddr const * sock = sender;
    for(int i = sender.GetAddressLength()-1; i >= 0; --i )
    {
        ++mul;
        checksum += (checksum >> 16) * mul;
        checksum += reinterpret_cast< char const * >( sock )[i];
        checksum %= modulo;
    }

    // message IDs must not be 0, so we need to add a 1 offset and can't
    // do the usual &0xffff / >>16 split.
    ret.first = ( checksum % 0xfffe ) + 1;
    ret.second = (checksum + 1 - ret.first)/0xfffe + 1;
}

static void sn_SynGenerateCookie(int stamp, nMessage const &m, nCookie & ret)
{
    sn_SynGenerateCookie( stamp, peers[m.SenderID()], ret );
}


// *************************************************************


REAL sg_ackDelay = 0;
static tConfItem<REAL> sg_ackDelayConf = HelperCommand::tConfItem("ACK_DELAY",sg_ackDelay);


void ack_handler(nMessage &m){
    if( m.SenderID() == MAXCLIENTS+1 )
    {
    // check for syn cookie response
        nCookie cookie;
        if(m.End())
        {
            return;
        }
        m.Read(cookie.first);
        if(m.End())
        {
            return;
        }
        m.Read(cookie.second);
        if(!m.End())
        {
            return;
        }

        int stamp = sn_SynTimestamp();
        for(int offset=0; offset >= -1; --offset)
        {
            nCookie correct;
            sn_SynGenerateCookie( stamp+offset , m, correct );
            if( correct.first == cookie.first && correct.second == cookie.second )
            {
                nMachine::GetMachine( m.SenderID() ).Validate();
            }
        }

        return;
    }

    while (!m.End())
    {


        //con << "Got ack:" << ack << ":" << m.SenderID() << '\n';
        if (sg_ackDelay <= 0)
        {
            sn_Connections[m.SenderID()].AckReceived();
            unsigned short ack;
            m.Read(ack);
            nWaitForAck::Ackt(ack,m.SenderID());

        }
        else
        {
            gTaskScheduler.schedule("ackDelay", sg_ackDelay, [&m]
            {
                sn_Connections[m.SenderID()].AckReceived();
                unsigned short ack;
                m.Read(ack);
                nWaitForAck::Ackt(ack,m.SenderID());
            });
        }
    }
}

static nDescriptor s_Acknowledge(1,ack_handler,"ack",true);


class nWaitForAck;
static tList<nWaitForAck> sn_pendingAcks;

//static eTimer netTimer;
static nTimeRolling netTime;

#ifdef NET_DEBUG
static int acks=0;
static int max_acks=0;
#endif

nWaitForAck::nWaitForAck(nMessage* m,int rec)
        :id(-1),message(m),receiver(rec)
{
#ifdef DEBUG
    // don't message yourself
    if ( rec == 0 && sn_GetNetState() == nSERVER )
        st_Breakpoint();
#endif

    if (!message)
        tERR_ERROR("Null ack!");

    if (message->Descriptor()!=s_Acknowledge.ID())
        sn_Connections[receiver].ackPending++;
    // else
    //     tERR_ERROR("Should not wait for ack of an ack message itself.");

    //    sn_ackAckPending[receiver]++;
#ifdef NET_DEBUG
    acks++;
#endif

    timeFirstSent=::netTime;
    timeLastSent=::netTime;

    timeouts=0;

    timeout=sn_GetTimeout( rec );

#ifdef nSIMULATE_PING
   timeSendAgain=::netTime + nSIMULATE_PING;
#ifndef WIN32
    tRandomizer & randomizer = tReproducibleRandomizer::GetInstance();
    timeSendAgain+= randomizer.Get() * nSIMULATE_PING_VARIANT;
    // timeSendAgain+=(nSIMULATE_PING_VARIANT*random())/RAND_MAX;
#endif
#else
    const REAL packetLossScale = .003; // packet loss rate that is considered big
    const REAL maxTimeoutFactor = 1.2; // maximal stretching of initial timeout value for flawless connections
    // factor mutliplied to timeout; 1 if the connection loses a lot of packets, 1.2 for a
    // flawless connection
    REAL timeoutFactor = 1 + (maxTimeoutFactor-1)*packetLossScale/(sn_Connections[receiver].PacketLoss() + packetLossScale);
    timeSendAgain=::netTime + timeout*timeoutFactor + zeroTimeout;
#endif
    sn_pendingAcks.Add(this,id);
}

nWaitForAck::~nWaitForAck(){
#ifdef NET_DEBUG
    acks--;
    if (acks>max_acks){
        max_acks=acks;
        // con << "MA=" << max_acks << '\n';
    }
#endif

    if (bool(message) && message->Descriptor()!=s_Acknowledge.ID())
    {
        sn_Connections[receiver].ackPending--;
        sn_Connections[receiver].ReliableMessageSent();
    }
    else
    {
        // tERR_ERROR( "No message." );
    }
    //    sn_ackAckPending[receiver]--;

    sn_pendingAcks.Remove(this,id);
    tCHECK_DEST;
}

void nWaitForAck::Ackt(unsigned short id,unsigned short peer){
#ifdef DEBUG_X
    int success=0;
#endif
    for(int i=sn_pendingAcks.Len()-1;i>=0;i--){
        nWaitForAck * ack = sn_pendingAcks(i);
        if (ack->message->MessageID()==id &&
                ack->receiver==peer){
#ifdef DEBUG
            //      if (sn_pendingAcks(i)->message == sn_WatchMessage)
            //	st_Breakpoint();
#endif

#ifdef DEBUG_X
            success=1;

            if (ack->message->descriptor>1)
                con << "AT  " << ack->message->descriptor << '\n';
#endif

            // calculate and average ping
            REAL thisping=netTime - ack->timeFirstSent;
            sn_Connections[peer].ping.Add( thisping, 1/(1 + 10 * REAL(ack->timeouts * ack->timeouts * ack->timeouts ) ) );

            ack->AckExtraAction();
            delete ack;
            ::timeouts[peer]=0;
            if (i<sn_pendingAcks.Len()-1) i++;
        }
    }

#ifdef DEBUG_X
    if (!success && peer!=MAXCLIENTS+1)
    {
        con << "Ack " << id << ':' << peer << " was not asked for.\n";
        if (sn_pendingAcks.Len()) con << "Expected:\n";
        for(int i=sn_pendingAcks.Len()-1;i>=0;i--){
            con << i << "\t:"
            << sn_pendingAcks[i]->message->messageIDBig_ << ":"
            << sn_pendingAcks[i]->receiver << '\n';
        }
    }
#endif
}

void nWaitForAck::AckAllPeer(unsigned short peer){
    for(int i=sn_pendingAcks.Len()-1;i>=0;i--){
        if (sn_pendingAcks(i)->receiver==peer){
            delete sn_pendingAcks(i);
            if (i<sn_pendingAcks.Len()-1) i++;
        }
    }
}

void nWaitForAck::Resend(){
    static tReproducibleRandomizer randomizer;

    for(int i=sn_pendingAcks.Len()-1;i>=0;i--){
        nWaitForAck* pendingAck = sn_pendingAcks(i);

        nConnectionInfo & connection = sn_Connections[pendingAck->receiver];

        // don't resend if you can't.
        if ( !connection.bandwidthControl_.CanSend() )
            continue;

        REAL packetLoss = connection.PacketLoss();
        REAL timeout = pendingAck->timeout;

        // should we resend the packet? Certainly it if it is overdue
        bool resend = (pendingAck->timeSendAgain + timeout * .1 <=netTime);

        // or if there is already a message waiting...
        if ( !resend && connection.sendBuffer_.Len() > 0 )
        {
            // and we are on time
            if ( pendingAck->timeSendAgain <= netTime )
                resend = true;
            // or the packet loss is so high that it is advisable to resend every message
            // multiple times if bandwidth is available ( we aim for 99% reliability )
            else if ( pendingAck->timeouts < 3 && pow( packetLoss, pendingAck->timeouts + 1 ) > .01 &&
                      connection.bandwidthControl_.Control( nBandwidthControl::Usage_Planning ) >100 )
                resend = true;

            /* + sn_GetTimeout( pendingAck->receiver ) *
                                ( 3.0 / ( pendingAck->timeouts + 1 ) )
                                ( packetLoss * ( randomizer.Get() + .5 ) ) )
            */
        }

        if ( resend ){
            //con << net_ticks-sn_pendingAcks[i]->ticks_first_sent << '\n';

            // update timeout counters
            ::timeouts[pendingAck->receiver]++;
            pendingAck->timeouts++;

            REAL timeoutTimeLimit;
            int timeoutPacketLimit;
            if(tRecorder::ProbablyDesyncedPlayback())
            {
                // allow quick timeouts in probably-broken-anyway playback mode
                timeoutTimeLimit = killTimeout/4;
                timeoutPacketLimit = 5;
            }
            else
            {
                timeoutTimeLimit = killTimeout;
                timeoutPacketLimit = 20;
            }

            if(netTime - pendingAck->timeFirstSent > timeoutTimeLimit &&
                    ::timeouts[pendingAck->receiver] > timeoutPacketLimit){
                // total timeout. Kill connection.
                if (pendingAck->receiver<=MAXCLIENTS && nWaitForAck::ExpectAcks()){
                    tOutput o;
                    o.SetTemplateParameter(1, pendingAck->receiver);
                    o << "$network_error_timeout";
                    con << o;
                    sn_DisconnectUser(pendingAck->receiver, "$network_kill_timeout" );

                    sn_Error = nTIMEOUT;

                    if (i>=sn_pendingAcks.Len())
                        i=sn_pendingAcks.Len()-1;
                }
                else // it is just in the login slot. Ignore it.
                {
                    ::timeouts[pendingAck->receiver] = 0;
                    delete pendingAck;
                }
            }
            else{
#ifdef DEBUG
                //if (pendingAck->message == sn_WatchMessage)
                //st_Breakpoint();
#endif

                if (connection.socket){
                    //	  if(sn_Connections[].rateControlPlanned[pendingAck->receiver]>-1000)
                    {
                        REAL timeoutFactor = .9 + .1 * pendingAck->timeouts + randomizer.Get() * .1;
                        pendingAck->timeSendAgain=netTime+timeout * timeoutFactor;
                        pendingAck->timeLastSent=netTime;

                        if (send_again_warn){
                            con << "sending packet again: " ;
                            deb_net=true;
                        }
                        connection.ReliableMessageSent();
                        pendingAck->message->SendImmediately
                        (pendingAck->receiver,false);
                        deb_net=false;
                    }
                }
                else
                    delete pendingAck;
            }
        }
    }
}

static bool sn_noExpectAckOnClientPlayback = false;
static tSettingItem< bool > sn_noExpectAckOnClientPlaybackConf( "EXPECT_ACK_ON_CLIENT_PLAYBACK", sn_noExpectAckOnClientPlayback );

bool nWaitForAck::ExpectAcks()
{
    bool ret = !tRecorder::IsPlayingBack() || (sn_GetNetState() != nCLIENT) || sn_noExpectAckOnClientPlayback;

    if(!ret)
        tRecorder::ActivateDesyncedPlayback();

    return ret;
}

// defined in netobjec.C
// void ClearKnows(int user);


#ifdef NET_DEBUG
static int nMessages=0;
static int max_nMessages=0;
#endif

#ifdef DEBUG
void BreakOnMessageID( unsigned int messageID )
{
    if (messageID == sn_WatchMessageID && messageID != 0 )
    {
        int x;
        x = 0;
    }
}
#endif

class nMessageIDExpander
{
    unsigned long quarters[4];
public:
    nMessageIDExpander()
    {
        for (int i=3; i>=0; --i)
            quarters[i]=i << 14;
    }

    unsigned long ExpandMessageID( unsigned short id )
    {
        // the current ID is in this quarter
        int thisQuarter = ( id >> 14 ) & 3;

        // the following quarter will be this
        int nextQuarter = ( thisQuarter + 1 ) & 3;

        // make sure the following quarter has a higher upper ID completion than this
        quarters[nextQuarter] = quarters[thisQuarter] + ( 1 << 14 );

        // replace high two bits of incoming ID with the counted up ID
        return quarters[thisQuarter] | id;
    }
};

//! expands a short message ID to a full int message ID, assuming it is from a message that was
// just received.
static unsigned long int sn_ExpandMessageID( unsigned short id, unsigned short sender )
{
#ifdef DEBUG
    BreakOnMessageID( id );
#endif

    static nMessageIDExpander expanders[MAXCLIENTS+2];

    tASSERT( sender <= MAXCLIENTS+2 )
    return expanders[sender].ExpandMessageID(id);
}

nMessage::nMessage(unsigned short*& buffer,short sender, int lenLeft )
        :descriptor(ntohs(*(buffer++))),messageIDBig_(sn_ExpandMessageID(ntohs(*(buffer++)),sender)),
senderID(sender),readOut(0){
#ifdef NET_DEBUG
    nMessages++;
#endif

    tRecorderSync< unsigned long >::Archive( "_MESSAGE_ID_IN", 3, messageIDBig_ );
    tRecorderSync< unsigned short >::Archive( "_MESSAGE_DECL_IN", 3, descriptor );

    unsigned short len=ntohs(*(buffer++));
    lenLeft-=3;
    if ( len > lenLeft )
    {
        len = lenLeft;
#ifndef NOEXCEPT
        throw nKillHim();
#endif
    }
    for(int i=0;i<len;i++)
        data[i]=ntohs(*(buffer++));

#ifdef DEBUG
    BreakOnMessageID( messageIDBig_ );
#endif
}

nMessage::nMessage(const nDescriptor &d)
        :descriptor(d.id),
senderID(::sn_myNetID), readOut(0){
#ifdef NET_DEBUG
    nMessages++;
#endif

    current_id++;
    if (current_id <= IDS_RESERVED)
        current_id = IDS_RESERVED + 1;

    messageIDBig_ = current_id;

#ifdef DEBUG_X
    con << "Message " << d.id << " " << current_id << "\n";
#endif

#ifdef DEBUG
    BreakOnMessageID( messageIDBig_ );
#endif

    tRecorderSync< unsigned long >::Archive( "_MESSAGE_ID_OUT", 3, messageIDBig_ );
    tRecorderSync< unsigned short >::Archive( "_MESSAGE_DECL_OUT", 3, descriptor );
}


nMessage::~nMessage(){
#ifdef NET_DEBUG
    nMessages--;
    if (nMessages>max_nMessages){
        max_nMessages=nMessages;
        con << "MN=" << max_nMessages <<'\n';
    }
#endif

#ifdef DEBUG_X
    if (descriptor>1)
        con << "DMT " << descriptor << "\n";
#endif

    tCHECK_DEST;
}



bool sg_descriptorsShowBroadCasted = false;
static tConfItem<bool> sg_descriptorsShowBroadCastedConf = HelperCommand::tConfItem("DESCRIPTOR_BROADCASTED_SHOW",sg_descriptorsShowBroadCasted);

tString sg_descriptorsShowBroadCastedIgnoreList = tString("");
static tConfItem<tString> sg_descriptorsShowBroadCastedIgnoreListConf = HelperCommand::tConfItem("DESCRIPTOR_BROADCASTED_SHOW_IGNORE_LIST",sg_descriptorsShowBroadCastedIgnoreList);

void nMessage::BroadCast(bool ack){

if (sg_descriptorsShowBroadCasted && !tIsInList(sg_descriptorsShowBroadCastedIgnoreList,(*this).descriptor))
    con << "BroadCast message for ID " << (*this).descriptor << " (" << descriptors[(*this).Descriptor()]->name << ") :\n" << con.nMessageToString(*this) << "\n";

    tControlledPTR< nMessage > keep( this );
    if (sn_GetNetState()==nCLIENT)
        Send(0,ack);

    if (sn_GetNetState()==nSERVER){
        for(int i=MAXCLIENTS;i>0;i--){
            if (sn_Connections[i].socket)
                Send(i,0,ack);
        }
    }
}


static nVersionFeature sn_ZeroMessageCrashfix( 1 );

nMessage& nMessage::operator << (const tString &s){
    unsigned short len=s.Len();

    // clamp away excess zeroes
    while(len > 1 && s(len-2)==0)
    {
        --len;
    }

    // check whether all clients support zero length strings
    if ( !sn_ZeroMessageCrashfix.Supported() )
    {
        if ( len <= 0 )
        {
            static tString replacement("");
            return this->operator<<( replacement );
        }
    }
    else if ( len == 1 && s(0) == 0 )
    {
        // do away with the the trailing zero in zero length strings.
        len = 0;
    }

    Write(len);
    int i;

    // write first pairs of bytes
    for(i=0;i+1<len;i+=2)
    {
        // yep. Signed arithmetic. That gives
        // nice overflows. By the time we noticed,
        // it was too late to change :)
        signed char lo = s[i];
        signed char hi = s[i+1];

        // combine the two into a single short
        Write( short(lo) + (short(hi) << 8) );
    }

    // write last byte
    if (i<len)
        Write( static_cast< signed char >( s[i] ) );

    return *this;
}

nMessage& nMessage::operator << (const tColoredString &s){
    return *this << static_cast< const tString & >( s );
}

nMessage& nMessage::operator << ( const tOutput &o ){
    return *this << tString( static_cast< const char * >( o ) );
}

nMessage& nMessage::ReadRaw(tString &s )
{
    s.Clear();
    unsigned short w = 0,len = 0;
    Read(len);
    if ( len > 0 )
    {
        s[len-1] = 0;
        for(int i=0;i<len;i+=2){
            Read(w);

            // carefully reverse the signed
            // encoding logic
            signed char lo = w & 0xff;
            signed short hi = ((short)w) - lo;
            hi >>= 8;

            s[i] = lo;
            if (i+1<len)
                s[i+1] = hi;
        }
    }

    return *this;
}

bool sn_filterColorStrings = false;
static tConfItem<bool> sn_filterColorStringsConf("FILTER_COLOR_STRINGS",sn_filterColorStrings);
bool sn_filterDarkColorStrings = false;
static tConfItem<bool> sn_filterDarkColorStringsConf("FILTER_DARK_COLOR_STRINGS",sn_filterDarkColorStrings);


nMessage& nMessage::operator >> (tColoredString &s )
{
    // read the raw data
    ReadRaw( s );

    // filter client string messages
    if ( sn_GetNetState() == nSERVER )
    {
        s.NetFilter();
        s.RemoveTrailingColor();
    }

    //  convert colors to lower case
    if (tColoredString::HasColors(s))
    {
        s = tColoredString::LowerColors(s);
        //if(st_verifyColorCodeStrictly) s = tColoredString::EscapeBadColors(s);
    }

    // filter color codes away
    if ( sn_filterColorStrings )
        s = tColoredString::RemoveColors( s, false );
    else if ( sn_filterDarkColorStrings )
        s = tColoredString::RemoveColors( s, true );

    return *this;
}

nMessage& nMessage::operator >> (tString &s )
{
    tColoredString safe;
    operator>>( safe );
    s = safe;

    return *this;
}


#define MANT 26
#define EXP (32-MANT)
#define MS (MANT-1)


typedef struct{
int mant:MANT;
unsigned int exp:EXP;
} myfloat;


nMessage& nMessage::operator<<(const REAL &x){


#ifdef DEBUG
    // con << "write x= " << x;


    if(sizeof(myfloat)!=sizeof(int))
        tERR_ERROR_INT("floating ePoint format does not work!");
#endif
    /*
      REAL nachkomma=x-floor(x);
      Write(short(x-nachkomma));
      Write(60000*nachkomma);
    */
    // no fuss. Read and write floats in binary format.
    // will likely cause problems for systems other than i386.

    //Write(((short *)&x)[0]);
    //Write(((short *)&x)[1]);

    // right. Caused severe problems with the AIX port.

    // new way: own floating ePoint format that is not good with small numbers
    // (we do not need them anyway)
    REAL y=x;

    unsigned int negative=0;
    if (y<0){
        y=-y;
        negative=1;
    }

    unsigned int exp=0;
    while ( fabs(y)>=64 && exp < (1<<EXP)-6 )
    {
        exp +=6;
        y/=64;
    }
    while ( fabs(y)>=1 && exp < (1<<EXP)-1 )
    {
        exp++;
        y/=2;
    }
    // now x=y*2^exp
    unsigned int mant=int(y*(1<<MS));
    // now x=mant*2^exp * (1/ (1<<MANT))

    // cutoffs:
    if (mant>((1<<MS))-1)
        mant=(1<<MS)-1;

    if (exp>(1<<EXP)-1){
        exp=(1<<EXP)-1;
        if (mant>0)
            mant=(1<<MS)-1;
    }

    // put them together:

    unsigned int trans=(mant & ((1<<MS)-1)) | (negative << MS) | (exp << MANT);
    /*
      myfloat trans;
      trans.exp=exp;
      trans.mant=mant;
    */

    operator<<(reinterpret_cast<int &>(trans));

#ifdef DEBUG
    /*
      con << "mant: " << mant
      << ", exp: " << exp
      << ", negative: " << negative;
    */

    unsigned int mant2=trans & ((1 << MS)-1);
    unsigned int negative2=(trans >> MS) & 1;
    unsigned int nt=trans-mant-(negative << MS);
    unsigned int exp2=nt >> MANT;

    if (mant2!=mant || negative2!=negative || exp2!=exp)
        tERR_ERROR_INT("Floating ePoint tranfer failure!");

    /*
      con << ", x: " << x;

      con << ", mant: " << mant
      << ", exp: " << exp
      << ", negative: " << negative;
    */

    // check:

    REAL z=REAL(mant)/(1<<MS);
    if (negative)
        z=-z;

    while (exp>=6){
        exp-=6;
        z*=64;
    }
    while (exp>0){
        exp--;
        z*=2;
    }

    if (fabs(z-x)>(fabs(x)+1)*.001)
        tERR_ERROR_INT("Floating ePoint tranfer failure!");

    //con << ", z: " << z << '\n';
#endif

    return *this;
}

nMessage& nMessage::operator>>(REAL &x){
    /*
      short vorkomma;
      unsigned short nachkomma;
      Read((unsigned short &)vorkomma);
      Read(nachkomma);
      x=vorkomma+nachkomma/60000.0;

      Read(((unsigned short *)&x)[0]);
      Read(((unsigned short *)&x)[1]);
     */

    unsigned int trans = 0;
    operator>>(reinterpret_cast<int &>(trans));

    int mant=trans & ((1 << MS)-1);
    unsigned int negative=(trans >> MS) & 1;
    unsigned int exp=(trans-mant-(negative << MS)) >> MANT;

    x=REAL(mant)/(1<<MS);
    if (negative)
        x=-x;

#ifdef DEBUG
    //  con << "read mant: " <<mant << ", exp: " << exp;
#endif

    while (exp>=6){
        exp-=6;
        x*=64;
    }
    while (exp>0){
        exp--;
        x*=2;
    }

#ifdef DEBUG
#ifndef WIN32
    if (!std::isfinite(x))
        st_Breakpoint();
    // con << " , x= " << x << '\n';
#endif
#endif
    return *this;
}

nMessage& nMessage::operator<< (const short &x){
    Write((reinterpret_cast<const short *>(&x))[0]);

    return *this;
}

nMessage& nMessage::operator>> (short &x){
    Read(reinterpret_cast<unsigned short *>(&x)[0]);

    return *this;
}

nMessage& nMessage::operator<< (const int &x){
    unsigned short a=x & (0xFFFF);
    short b=(x-a) >> 16;

    Write(a);
    operator<<(b);

    return *this;
}

nMessage& nMessage::operator>> (int &x){
    unsigned short a;
    short b;

    Read(a);
    operator>>(b);

    x=(b << 16)+a;

    return *this;
}

nMessage& nMessage::operator<< (const bool &x){
    if (x)
        Write(1);
    else
        Write(0);

    return *this;
}

nMessage& nMessage::operator>> (bool &x){
    unsigned short y;
    Read(y);
    x= (y!=0);

    return *this;
}


void nMessage::Write(const unsigned short &x)
{
    //con << x << "\n";
    data[data.Len()]=x;
}

void nMessage::Read(unsigned short &x)
{
    if (End())
    {
        tOutput o;
        st_Breakpoint();
        o.SetTemplateParameter(1, senderID);
        o << "$network_error_shortmessage";
        con << o;
        // sn_DisconnectUser(senderID, "$network_kill_error");
        nReadError(false);
    }
    else
    {
        x = data(readOut++);
        // if (readOut == data.Len())
        // {
        //     con.nMessage(*this);
        // }
    }
}

// **********************************************
//  Basic communication classes: login
// **********************************************

static bool login_failed=false;
static bool login_succeeded=false;

// salt value sent as past login tokens. They are returned by
// the server as you sent them, and make sure you only accept
// the right answer.
static nKrawall::nSalt loginSalt;

static nHandler *real_req_info_handler=NULL;

void req_info_handler(nMessage &m){
    if (real_req_info_handler)
        (*real_req_info_handler)(m);
    if (m.SenderID()==MAXCLIENTS+1)
        sn_DisconnectUser(MAXCLIENTS+1, "$network_kill_logout");
}

static nDescriptor req_info(2,req_info_handler,"req_info");

void RequestInfoHandler(nHandler *handle){
    real_req_info_handler=handle;
}

// the server we are redirected to
static std::unique_ptr< nServerInfoBase > sn_redirectTo;
std::unique_ptr< nServerInfoBase > sn_GetRedirectTo()
{
    return std::move(sn_redirectTo);
}

nServerInfoBase * sn_PeekRedirectTo()
{
    return sn_redirectTo.get();
}

void login_deny_handler(nMessage &m){
    // only the server is allowed to send this
    if(m.SenderID() != 0)
        return;


    if (se_playerStats)
        ePlayerStats::saveStatsToDB();

    if ( !m.End() )
    {
        //		tOutput output;
        //		m >> output;
        //		sn_DenyReason = output;
        m >> sn_DenyReason;
    }
    else
    {
        sn_DenyReason = tOutput( "$network_kill_unknown" );
    }

    if (sn_bannedWatch)
    {
        gHelperUtility::Debug("PLAYER_WATCH_BANNED", "Got login denial.. Reason:", sn_DenyReason);
        tString checkReason;
        checkReason << (sn_bannedWatchStr.empty()? "You are banned" : sn_bannedWatchStr);

        if (sn_DenyReason.Contains(checkReason))
        {
            gHelperUtility::Debug("PLAYER_WATCH_BANNED", "Ban detected.");
            sn_quitAction(true, sn_bannedWatchQuit, sn_DenyReason);
        }
    }

    if (sn_loginDenalWatch && !sn_loginDenalWatchStr.empty())
    {
        tString sn_DenyReasonLower = sn_DenyReason.ToLower();

        tArray<tString> denialChecks = sn_loginDenalWatchStr.ToLower().Split(",");

        for (int i = 0; i < denialChecks.Len(); i++)
        {
            if (sn_DenyReasonLower.Contains(denialChecks[i]))
            {
                gHelperUtility::Debug("PLAYER_WATCH_LOGIN_DENIAL", "Login Denial detected and found a match within '" + sn_loginDenalWatchStr.stdString() + "'.");
                sn_quitAction(true, sn_loginDenalWatchQuit, sn_DenyReason);
                break;
            }
        }
    }

    if ( !m.End() )
    {
        // read redirection data from message
        tString connectionName;
        m >> connectionName;
        int port = 0;
        m >> port;

        if ( connectionName.Len() > 1 )
        {
            // create server info and fill it with data
            sn_redirectTo.reset(new nServerInfoRedirect( connectionName, port ) );
        }
    }

    if (!login_failed)
        con << tOutput("$network_login_denial");

    if (sn_GetNetState()!=nSERVER)
    {

        login_failed=true;
        login_succeeded=false;
        sn_SetNetState(nSTANDALONE);
    }
}

static nDescriptor login_deny(3,login_deny_handler,"login_deny");

void login_handler_1( nMessage&m );
void login_handler_2( nMessage&m );
void logout_handler( nMessage&m );

nDescriptor login(6,login_handler_1,"login1", true);
nDescriptor login_2(11,login_handler_2,"login2", true);
nDescriptor logout(7,logout_handler,"logout");

tString sn_DenyReason;

void login_ignore_handler(nMessage &m){
    if (sn_GetNetState()!=nSERVER && !login_succeeded){
        /*
          login_failed=true;
          login_succeeded=false;

          // kicking the one who uses our place
          // (he is probably timing out anyway..)
          nMessage *lo=new nMessage(logout);
          lo->Write((unsigned short)sn_myNetID);
          lo->Send(0);

          sn_Sync(10);

          (new nMessage(login))->Send(0);
        */
    }


}

static nDescriptor login_ignore(4,login_ignore_handler,"login_ignore");


void first_fill_ids();

// from nServerInfo.cpp
extern bool sn_AcceptingFromMaster;


#ifndef DEDICATED
static bool sn_showOwnIP = false;
static tConfItem<bool> sn_showOwnIPConf("SHOW_OWN_IP", sn_showOwnIP);
#else
static constexpr bool sn_showOwnIP = true;
#endif

void login_accept_handler(nMessage &m){
    if (sn_GetNetState()!=nSERVER && m.SenderID() == 0){
        if(m.End())
        {
            // fake login accept sent only for cookie ack response. ignore.
            return;
        }

        unsigned short id=0;
        con << "login_accept_handler GOT ID:" << id << "\n";
        m.Read(id);

        // read or reset server version info
        if ( !m.End() )
        {
            m >> sn_Connections[0].version;

// #ifdef DEBUG
#define NOEXPIRE
// #endif

#ifndef NOEXPIRE
#ifndef DEDICATED
            // last checked to be compatible with 0.3.1_pb from trunk.
            // It's ulikely this branch will introduce more bugs/network code revisions, so we're fine accepting all
            int lastCheckedTrunkVersion = 21;

            // start of trunk as seen from this branch
            int trunkBegin = 20;

            // maximal allowed version from this branch
            int maxVersionThisBranch = sn_currentProtocolVersion + 1;

            // expiration for public beta versions
            if ( !sn_AcceptingFromMaster &&
                    ( strstr( VERSION, "rc" ) || strstr( VERSION, "alpha" ) || strstr( VERSION, "beta" ) ) &&
                 ( sn_Connections[0].version.Max() > lastCheckedTrunkVersion ||
                   ( sn_Connections[0].version.Max() > maxVersionThisBranch && sn_Connections[0].version.Max() < trunkBegin )
                     )
                )
            {
                throw tGenericException( tOutput("$testing_version_expired"), tOutput("$testing_version_expired_title" ) );
            }
#endif
#endif
        }
        else
            sn_Connections[0].version = nVersion( 0, 0);

        // read my public IP
        if ( !m.End() )
        {
            // only accept it if it is not a LAN address
            tString address;
            m >> address;
            if ( !sn_IsLANAddress( address ) )
            {
                if ( sn_myAddress != address )
                {
                    if (sn_GetNetState()!=nSERVER && !sn_disallowJoinWithIP.empty() && address.Contains(sn_disallowJoinWithIP))
                    {

                        login_failed=true;
                        login_succeeded=false;
                        sn_SetNetState(nSTANDALONE);
                        tString disallowReason;

                        disallowReason << "Not allowed to play with IP '" << sn_disallowJoinWithIP << "'.";

                        sn_DenyReason = disallowReason;

                        return;
                    }

                    if(sn_showOwnIP)
                        con << "Got address " << address << ".\n";
                    else
                        con << "Got address.\n";
                }
                sn_myAddress = address;
            }

            // read salt reply and compare it to what we sent
            nKrawall::nSalt replySalt;
            nKrawall::ReadScrambledPassword( m, replySalt );

            int compare = memcmp( &loginSalt,&replySalt, sizeof(replySalt) );

            // since we generate a different random salt on playback, record the comparison result
            static const char * section = "LOGIN_SALT";
            tRecorder::Playback( section, compare );
            tRecorder::Record( section, compare );

            if ( compare != 0 )
            {
                nReadError( false );
            }
        }

        // only now, login can be considered a success
        login_succeeded=true;
        sn_myNetID=id;
        // con << "ASSIGNED sn_myNetID to " << id << "\n";
        first_fill_ids();
    }
}

static nDescriptor login_accept(5,login_accept_handler,"login_accept", true);



//static short new_id=0;

// counts the number of logins with the same IP
int CountSameIP( int user, bool reset=false )
{
    static int sameIP[ MAXCLIENTS+2 ];
    tASSERT( user >= 0 && user <= MAXCLIENTS+1 );

    if ( reset )
    {
        int count = 0;
        for(int user2=1;user2<=MAXCLIENTS;++user2)
        {
            if(!sn_Connections[user2].socket)
                continue;

            if ( user2 != user && nAddress::Compare( peers[user], peers[user2] ) >=0 )
            {
                count++;
            }
        }

        sameIP[user] = count;
    }

    return sameIP[user];
}

// counts the number of logins with the same Connection
int CountSameConnection( int user )
{
    int count = 0;
    for(int user2=1;user2<=MAXCLIENTS;++user2)
    {
        if( NULL == sn_Connections[user2].socket )
            continue;

        if ( user2 != user && nAddress::Compare( peers[user], peers[user2] ) == 0 )
        {
            count++;
        }
    }

    return count;
}

// determine a free connection slot or at least one where the user won't be missed
int GetFreeSlot()
{
    int user;

    // level 1: look for free slot
    if ( sn_NumUsers() < sn_maxClients )
    {
        for(user=1;user<=sn_maxClients;++user)
        {
            // look for empty slot
            if(!sn_Connections[user].socket)
            {
                return user;
            }
        }
    }

    int best = -1;

    // level 2 kicked out users who were timing out and was not a good idea.

    int bestCount = sn_allowSameIPCountSoft-1;

    // level 3: look for dublicate IPs
    for(user=1;user<=MAXCLIENTS;++user)
    {
        int count = CountSameIP( user );
        if ( count > bestCount )
        {
            bestCount = count;
            best = user;
        }
    }
    if ( best > 0 )
        return best;

    return -1;
}

static REAL sn_minBan    = 120; // minimal ban time in seconds for trying to connect while you're banned
static tSettingItem< REAL > sn_minBanSetting( "NETWORK_MIN_BAN", sn_minBan );


// flag to disable 0.2.8 test version lockout
static bool sn_lockOut028tTest = true;
static tSettingItem< bool > sn_lockOut028TestConf( "NETWORK_LOCK_OUT_028_TEST", sn_lockOut028tTest );

// the network stuff planned to send:
tHeap<planned_send> send_queue[MAXCLIENTS+2];

// defined in nServerInfo.cpp
extern bool FloodProtection( nMachine & machine, REAL timeFactor=1.0 );

// time factor for incoming connections, lower makes turtle mode kick in later
static REAL sn_minConnectionTimeGlobalFactor = 0.1;
static tSettingItem< REAL > sn_minPingTimeGlobal( "CONNECTION_FLOOD_SENSITIVITY", sn_minConnectionTimeGlobalFactor );

// enforce turtle mode
static bool sn_forceTurtleMode = false;
static tSettingItem< bool > sn_forceTurtleModeConf( "FORCE_TURTLE_MODE", sn_forceTurtleMode );

// keep recording even in turtle mode
static bool sn_recordTurtleMode = false;
static tSettingItem< bool > sn_recordTurtleModeConf( "RECORD_TURTLE_MODE", sn_recordTurtleMode );

// enforce the anti-spoof login part of turtle mode at all times
static bool sn_synCookie = false;
static tSettingItem< bool > sn_synCookieConf( "ANTI_SPOOF", sn_synCookie );


// number of packets from unknown sources to process each call to rec_peer
static int sn_connectionLimit = 5;
static tSettingItem< int > sn_connectionLimitConf( "CONNECTION_LIMIT", sn_connectionLimit );

// turtle mode control
class nTurtleControl
{
    REAL lastTurtleModeTime; // last time turtle mode was activated
    bool setThisFrame;

    // true while we're turtling from a flood
    bool turtleMode;
public:
    nTurtleControl()
    : lastTurtleModeTime(-700)
    , setThisFrame(false)
    , turtleMode(false)
    {
    }

    operator bool() const
    {
        return turtleMode;
    }

    // activates turtle mode. It will persist for at least 60 seconds.
    void SetTurtleMode()
    {
        if( !setThisFrame )
        {
            if( !turtleMode )
            {
                // report
                sn_ConsoleOut( tOutput("$turtle_mode_activated") );

                // stop recording
                if( !sn_recordTurtleMode && tRecorder::IsRecording() )
                {
                    tRecorder::StopRecording();
                }
            }

            turtleMode = true;
            setThisFrame = true;
            lastTurtleModeTime = tSysTimeFloat();

        }
    }

    void Update()
    {
        setThisFrame = false;
        if( lastTurtleModeTime + 60 < tSysTimeFloat() )
        {
            if( turtleMode && !sn_forceTurtleMode )
            {
                // report
                sn_ConsoleOut( tOutput("$turtle_mode_deactivated") );
            }

            turtleMode = sn_forceTurtleMode;
        }
    }
};
static nTurtleControl sn_turtleMode;

// checks for global flood events
static inline bool GlobalConnectionFloodProtection( REAL extraFactor = 1.0f )
{
    static nMachine server;

    if( sn_minConnectionTimeGlobalFactor > 0 && FloodProtection( server, sn_minConnectionTimeGlobalFactor*extraFactor ) )
    {
        sn_turtleMode.SetTurtleMode();
    }

    return sn_turtleMode;
}

// checks for individual flood events
bool IndividualConnectionFloodProtection( nMachine * machine, int peer,  REAL extraFactor = 1.0f )
{
    // IP is not spoofed or there is no
    // current spoof heavy attack. Really look up the machine.
    if( !machine )
    {
        machine = &nMachine::GetMachine( peer );
    }

    // check individual flood protection (be lenient in turtle mode, login responses may have trouble getting through an attack)
    return FloodProtection( *machine, ( sn_turtleMode ? .2 : 1 ) * extraFactor );
}

// report login failure. Or don't if we're flooded.
int sn_ReportFailure(nMessage &m, char const * reason)
{
    if( !sn_turtleMode )
    {
        sn_DisconnectUser(m.SenderID(), reason);
    }

    return -1;
}

int login_handler( nMessage &m, unsigned short rate ){
    nCurrentSenderID senderID;

    // read version and suppored authentication methods
    nVersion version;
    tString supportedAuthenticationMethods("");
    nKrawall::nSalt salt; // it's OK that this may stay uninitialized
    if ( !m.End() )
    {
        // read version
        m >> version;

        // ok, clients that send a version do have at lesat basic authentication.
        supportedAuthenticationMethods = "bmd5";
    }
    if ( !m.End() )
    {
        // read authentication methods
        m >> supportedAuthenticationMethods;
    }
    if ( !m.End() )
    {
        // also read a login salt, the client expects to get it returned verbatim
        nKrawall::ReadScrambledPassword( m, salt );
    }

    // don't accept logins in client mode
    if (sn_GetNetState() != nSERVER)
        return -1;

    // ban users
    nMachine & machine = nMachine::GetMachine( m.SenderID() );
    REAL banned = machine.IsBanned();
    if ( banned > 0 )
    {
        // the reason for the ban
        tString const & reason = machine.GetBanReason();

        // ban user some more so he learns
        if ( banned < sn_minBan )
        {
            machine.Ban( sn_minBan );
            banned = sn_minBan;
        }
        else
            con << tOutput( "$network_ban", machine.GetIP() , int(banned/60), reason.Len() > 1 ? reason : tOutput( "$network_ban_noreason" ) );

        sn_DisconnectUser(m.SenderID(), tOutput( "$network_kill_banned", int(banned/60), reason ) );

        return -1;
    }

    // ignore multiple logins
    if( CountSameConnection( m.SenderID() ) > 0 )
        return -1;

    bool success=false;

    int new_id = -1;

    // test
    //	sn_DisconnectUser(m.SenderID(), "$network_kill_incompatible");
    //	return -1;

    nVersion mergedVersion;
    if ( !mergedVersion.Merge( version, sn_CurrentVersion() ) )
    {
        return sn_ReportFailure(m, "$network_kill_incompatible");
    }

    // expire 0.2.8 test versions, they have a security flaw
    if ( sn_lockOut028tTest && version.Max() >= 5 && version.Max() <= 10 )
    {
        return sn_ReportFailure(m, "0.2.8_beta and 0.2.8.0_rc versions have a dangerous security flaw and are obsoleted, please upgrade to 0.2.8.2.1.");
    }

    if (m.SenderID()!=MAXCLIENTS+1)
    {
        //con << "Ignoring second login from " << m.SenderID() << ".\n";
        (new nMessage(login_ignore))->Send(m.SenderID());
    }
    else if (sn_Connections[m.SenderID()].socket)
    {
        if ( sn_maxClients > MAXCLIENTS )
            sn_maxClients = MAXCLIENTS;

        // count doublicate IPs
        if ( CountSameIP( m.SenderID(), true ) < sn_allowSameIPCountHard )
        {
            // find new free ( or freeable ) ID
            new_id = GetFreeSlot();
            if ( new_id > 0 )
            {
                if(sn_Connections[new_id].socket)
                {
                    sn_DisconnectUser( new_id, "$network_kill_full" );
                }

                success = true;

                senderID.SetID( new_id );

                sn_Connections	[ new_id ].socket	= sn_Connections[MAXCLIENTS+1].socket; // the new connection has number MC+1
                peers			[ new_id ]			= peers[MAXCLIENTS+1];
                timeouts		[ new_id ]			= kickOnDemandTimeout/2;

                // sn_Connections	[ MAXCLIENTS+1 ].socket		= NULL;
                // peers			[ MAXCLIENTS+1 ].sa_family	= 0;
                //				nCallbackLoginLogout::UserLoggedIn(i);

                sn_Connections	[ new_id ].supportedAuthenticationMethods_ = supportedAuthenticationMethods;

                // recount doublicate IPs
                CountSameIP( new_id, true );
            }
        }

        // log login to console
        tOutput o;
        o.SetTemplateParameter(1, peers[m.SenderID()].ToString() );
        o.SetTemplateParameter(2, sn_Connections[m.SenderID()].socket->GetAddress().ToString() );
        o.SetTemplateParameter(3, sn_GetClientVersionString(version.Max()) );
        o.SetTemplateParameter(4, version.Max() );
        o << "$network_server_login";
        con << o;
    }
    if (success)
    {
        tOutput o;
        o.SetTemplateParameter(1, new_id);
        o << "$network_server_login_success";
        con << o;
        //      tString s;
        // s << "User " << new_id << " logged in.\n";

        sn_Connections[new_id].ping.Reset();
        sn_Connections[new_id].bandwidthControl_.Reset();
        reset_last_acks(new_id);

        if (rate>sn_maxRateOut)
            rate=sn_maxRateOut;

        sn_Connections[new_id].bandwidthControl_.SetRate( rate );
        sn_Connections[new_id].version = version;

        nWaitForAck::AckAllPeer(MAXCLIENTS+1);
        reset_last_acks(MAXCLIENTS+1);
        if (sn_Connections[MAXCLIENTS+1].ackMess)
        {
            sn_Connections[MAXCLIENTS+1].ackMess=NULL;
        }

        // clear message queue
        while (send_queue[new_id].Len())
            delete (send_queue[new_id](0));

        // send login accept message with high priority
        nMessage *rep=new nMessage(login_accept);
        rep->Write(new_id);
        (*rep) << sn_myVersion;
        (*rep) << peers[m.SenderID()].ToString();
        nKrawall::WriteScrambledPassword( salt, *rep );

        rep->Send(new_id, -killTimeout);

        nMessage::SendCollected( new_id );

        nConfItemBase::s_SendConfig(true, new_id);

        // fake activity
        nMachine & machine = nMachine::GetMachine( new_id );
        machine.AddPlayer();
        machine.RemovePlayer();

        nCallbackLoginLogout::UserLoggedIn(new_id);

        //      ANET_Listen(false);
        //      ANET_Listen(true);
    }
    else if (m.SenderID()==MAXCLIENTS+1)
    {
        return sn_ReportFailure(m, "$network_kill_full");
    }

    sn_UpdateCurrentVersion();

    return new_id;
}

void login_handler_1(nMessage& m)
{
    unsigned short rate;

    m.Read( rate );

    if ( !m.End() ){ // we get a big brother message (ignore it)
        tString rem_bb;
        m >> rem_bb;
    }

    login_handler( m, rate );
}

void login_handler_2(nMessage& m)
{
    unsigned short rate;
    unsigned short bb;

    m.Read( rate );
    m.Read( bb );
    tString rem_bb;

    if ( bb )
    { // we get a big brother message
        m >> rem_bb;
    }

    int new_ID = login_handler( m, rate );

    if ( new_ID > 0 )
    {
        nMessage* m = tNEW( nMessage )( versionControl );
        (*m) << sn_currentVersion;

        m->Send( new_ID );

        if ( bb )
        {
            std::ofstream s;
            if ( tDirectories::Var().Open(s, "big_brother",std::ios::app) )
                s << rem_bb << '\n';
        }
    }
}


void logout_handler(nMessage &m){
    unsigned short id = m.SenderID();
    //m.Read(id);

    // only the server or legal clients are allowed to send this
    // (client check comes later)
    if(sn_GetNetState() == nCLIENT && id != 0)
        return;

    if (sn_Connections[id].socket)
    {
        tOutput o;
        o.SetTemplateParameter(1, id);
        o << "$network_logout_server";
        con << o;
    }
    nWaitForAck::AckAllPeer(id);

    if (0<id && id<=MAXCLIENTS)
    {
        sn_DisconnectUser(id, "$network_kill_logout");
    }
}


#define MAX_MESS_LEN 300
#define OVERHEAD 32

static REAL sn_OrderPriority = 0;

// statistics
int sn_SentBytes        = 0;
int sn_SentPackets      = 0;
int sn_ReceivedBytes    = 0;
int sn_ReceivedPackets  = 0;
nTimeRolling sn_StatsTime		= 0;


// adds a message to the buffer
void nSendBuffer::AddMessage	( nMessage&			message, nBandwidthControl* control )
{
    unsigned long id = message.MessageID();
    unsigned short len = message.DataLen();
    tRecorderSync< unsigned long >::Archive( "_MESSAGE_ID_SEND", 5, id );

    sendBuffer_[sendBuffer_.Len()]=htons(message.Descriptor());

    sendBuffer_[sendBuffer_.Len()]=htons(message.MessageID());

    sendBuffer_[sendBuffer_.Len()]=htons(message.DataLen());
    for(int i=0;i<len;i++)
        sendBuffer_[sendBuffer_.Len()]=htons(message.Data(i));

    tRecorderSync< unsigned short >::Archive( "_MESSAGE_SEND_LEN", 5, len );

    if ( control )
    {
        control->Use( nBandwidthControl::Usage_Planning, len * 2 );
    }
}

// send the contents of the buffer to a specific socket
void nSendBuffer::Send			( nSocket const &				socket
                           , const nAddress &	peer
                           ,nBandwidthControl* control )
{
    if (sendBuffer_.Len()){
        sn_SentPackets++;
        sn_SentBytes  += sendBuffer_.Len() * 2 + OVERHEAD;

        // store our id
        sendBuffer_[sendBuffer_.Len()]=htons(::sn_myNetID);

        socket.Write( reinterpret_cast<int8 *>(&(sendBuffer_[0])),
                      2*sendBuffer_.Len(), peer);

        if ( control )
        {
            control->Use( nBandwidthControl::Usage_Execution, 2*sendBuffer_.Len() + OVERHEAD );
        }

        this->Clear();
    }
}

// broadcast the contents of the buffer
void nSendBuffer::Broadcast	( nSocket const &				socket
                              , int				port
                              , nBandwidthControl* control )
{
    if (sendBuffer_.Len()){
        sn_SentPackets++;
        sn_SentBytes  += sendBuffer_.Len() * 2 + OVERHEAD;

        // store our id
        sendBuffer_[sendBuffer_.Len()]=htons(::sn_myNetID);

        socket.Broadcast( reinterpret_cast<int8 *>(&(sendBuffer_[0])),
                          2*sendBuffer_.Len(), port);

        Clear();

        if ( control )
        {
            control->Use( nBandwidthControl::Usage_Execution, 2*sendBuffer_.Len() + OVERHEAD );
        }
    }
}

// clears the buffer
void nSendBuffer::Clear()
{
    for(int i=sendBuffer_.Len()-1;i>=0;i--)
        sendBuffer_(i)=0;

    sendBuffer_.SetLen( 0 );
}


nBandwidthControl::nBandwidthControl( nBandwidthControl* parent )
{
#ifdef DEBUG
    if ( parent )
        parent->numChildren_ ++;
    numChildren_ = 0;
#endif

    parent_ = parent;

    Reset();
}

nBandwidthControl::~nBandwidthControl()
{
#ifdef DEBUG
    if ( parent_ )
        parent_->numChildren_ --;

    tASSERT( numChildren_ == 0 );
#endif
}

void nBandwidthControl::Reset()
{
    rateControlPlanned_ = rateControl_ = 1000.0f;
    rate_ = 8;
}

void nBandwidthControl::Use( Usage planned, REAL bandwidth )
{
    tRecorderSync< REAL >::Archive( "_RATE_CONTROL_USAGE", 4, bandwidth );
    ( Usage_Planning == planned ? rateControlPlanned_ : rateControl_ ) -= bandwidth;
}

void nBandwidthControl::Update( REAL ts)
{
    tRecorderSync< REAL >::Archive( "_RATE_CONTROL", 12, rateControl_ );
    tRecorderSync< REAL >::Archive( "_RATE_CONTROL_PLANNED", 12, rateControlPlanned_ );

    rateControl_ += ( rate_ * 1000 ) * ts;

    if ( rateControl_ > 1000.0f )
    {
        rateControl_ = 1000.0f;
    }

    rateControlPlanned_ = rateControl_;
}

void nMessage::SendCollected(int peer)
{
    //if ( peer == 7 && sn_Connections[peer].sendBuffer_.Len() > 0 )
    //    con << tSysTimeFloat() << "\n";

    sn_OrderPriority = 0;

    if (peer<0 || peer > MAXCLIENTS+1 || !sn_Connections[peer].socket)
        tERR_ERROR("Invalid peer!");

    sn_Connections[peer].sendBuffer_.Send( *sn_Connections[peer].socket, peers[peer], &sn_Connections[peer].bandwidthControl_ );
}


void nMessage::BroadcastCollected(int peer, unsigned int port){
    if (peer<0 || peer > MAXCLIENTS+1 || !sn_Connections[peer].socket)
        tERR_ERROR("Invalid peer!");

    sn_Connections[peer].sendBuffer_.Broadcast( *sn_Connections[peer].socket, port, &sn_Connections[peer].bandwidthControl_ );
}


// TODO_NOACK
void nMessage::SendImmediately(int peer,bool ack){
    if (ack)
    {
#ifdef NO_ACK
        tASSERT(messageIDBig_);
#endif
        new nWaitForAck(this,peer);

#ifdef nSIMULATE_PING
        return;
#endif
    }

    // server: messages to yourself are a bit strange...
    if ( sn_GetNetState() == nSERVER && peer == 0 && ack )
    {
        st_Breakpoint();
        tJUST_CONTROLLED_PTR< nMessage > bounce(this);
        return;
    }

#ifdef DEBUG
    /*
    if (descriptor>1)
      con << "SMT " << descriptor << "\n";
    */

    /*
      #ifdef DEBUG
      if (sn_Connections[].rate_control[peer]<-2000)
         tERR_ERROR("Heavy network overflow.");
      #endif
    */

    // if (peer==0 && sn_GetNetState()==nSERVER)
    //      tERR_ERROR("talking to yourself, eh?");

    if (peer==MAXCLIENTS+1){
#ifdef DEBUG
        if(descriptor==s_Acknowledge.id)
            con << "Sending ack to login slot.\n";
#endif
        //      else if (descriptor
        //	tERR_ERROR("the last user only should receive denials or acks.");
    }
#endif

    if (sn_Connections[peer].sendBuffer_.Len()+data.Len()+3 > MAX_MESS_LEN/2){
        SendCollected(peer);
        //con << "Overflow packets sent to " << peer << '\n';
    }


    if (sn_Connections[peer].socket)
    {
        sn_Connections[peer].sendBuffer_.AddMessage( *this, &sn_Connections[peer].bandwidthControl_ );

        /*
          if (sn_Connections[].rate_control[peer]>0)
          send_collected(peer);

          unsigned short *b=new (unsigned short)[data.Len()+3];

          b[0]=htons(descriptor);
          b[1]=htons(messageID);
          b[2]=htons(data.Len());
          int len=data.Len();
          for(int i=0;i<len;i++)
          b[3+i]=htons(data(i));


          ANET_Write(sn_Connections[].socket[peer],(int8 *)b,
          2*(data.Len()+3),&peers[peer]);

          //std::cerr << "Sent " << 2*len+6 << " bytes.\n";
          sn_Connections[].rate_control[peer]-=2*(len+3)+OVERHEAD;

          delete b;
        */

        if (deb_net)
            con << "Sent " <<descriptor << ':' << messageIDBig_ << ":"
            << peer << '\n';
    }

    tControlledPTR< nMessage > bounce( this ); // delete this message if nobody is interested in it any more
}

REAL sent_per_messid[MAXDESCRIPTORS+100];

void nMessage::Send(int peer,REAL priority,bool ack){
#ifdef NO_ACK
    if (!ack)
        messageIDBig_ = 0;
#endif

    // don't send messages to unsupported peers or in non-networked mode
    if( peer > MAXCLIENTS+1 || sn_GetNetState() == nSTANDALONE )
    {
        tJUST_CONTROLLED_PTR< nMessage > bounce(this);
        return;
    }


    // messages to yourself are a bit strange...
    if ( sn_GetNetState() == nSERVER && peer == 0 && ack )
    {
        st_Breakpoint();
        tJUST_CONTROLLED_PTR< nMessage > bounce(this);
        return;
    }

#ifdef DEBUG

    if (peer==MAXCLIENTS+1){
        if(descriptor==s_Acknowledge.id)
            con << "Sending ack to login slot.\n";
        //      else if (descriptor
        //	tERR_ERROR("the last user only should receive denials or acks.");
    }
#endif

#ifdef DEBUG_X
    if (descriptor>1)
        con << "PMT " << descriptor << "\n";
#endif

    // the next line was redundant; the send buffer handles that part of accounting.
    //sn_Connections[peer].bandwidthControl_.Use( nBandwidthControl::Usage_Planning, 2*(data.Len()+3) );

    sent_per_messid[descriptor]+=2*(data.Len()+3);

    tASSERT(Descriptor()!=s_Acknowledge.ID() || !ack);

    new nMessage_planned_send(this,priority+sn_OrderPriority,ack,peer);
    sn_OrderPriority += .01; // to roughly keep the relative order of netmessages
}

// ack messages: don't get an ID
class nAckMessage: public nMessage
{
public:
    nAckMessage(): nMessage( s_Acknowledge ){ messageIDBig_ = 0; }
};

// receive and s_Acknowledge the recently reveived network messages

typedef std::deque< tJUST_CONTROLLED_PTR< nMessage > > nMessageFifo;

// from nServerInfo.cpp
extern nDescriptor RequestSmallServerInfoDescriptor, RequestBigServerInfoDescriptor;

static void rec_peer(unsigned int peer){
    tASSERT( sn_Connections[peer].socket );

    sn_turtleMode.Update();
    nMachine::Expire();

    // temporary fifo for received messages
    //static tArray< tJUST_CONTROLLED_PTR< nMessage > > receivedMessages;
    static nMessageFifo receivedMessages;

    // the growing buffer we read messages into
#ifndef DEDICATED
    const int serverMaxAcceptedSize=2000;
#endif
    static tArray< unsigned short > storage(2000);
    int maxrec = 0; maxrec = storage.Len();
    unsigned short * buff = 0; buff = &storage[0];

    // short buff[maxrec];
    if (sn_Connections[peer].socket){
        int count=0;
        int len=1;
        while (len>=0 && sn_Connections[peer].socket)
        {
            nAddress addrFrom; // the sender of the current packet
            len = sn_Connections[peer].socket->Read( reinterpret_cast<int8 *>(buff),maxrec*2, addrFrom);

            if (len>=2){
                if ( len >= maxrec*2 )
                {
#ifndef DEDICATED
                    // the message was too long to receive. What to do?
                    if ( sn_GetNetState() != nSERVER || len < serverMaxAcceptedSize )
                    {
                        // expand the buffer. The message is lost now, but the
                        // peer will certainly resend it. Hopefully, the buffer will be large enough to hold it then.
                        storage[maxrec*2-1]=0;
                        maxrec = storage.Len();
                        buff = &storage[0];

                        tERR_WARN( "Oversized network packet received. Read buffer has been enlargened to catch it the next time.");

                    }
                    else
#endif
                    {
                        // packet WAAAAY too large.
                        static float totalFatsoes = 10;  // number of oversized packages checked
                        static float clientFatsoes = 10; // number of oversized pacakges that could be attributed to clients
                        static float bother = 5;         // counter that determines whether we bother to check.
                        bother+=clientFatsoes;

                        // what follows is work, so we only do it if it payed off in the past
                        // if this block is entered not at all by error, no biggie. The clients
                        // will time out eventually.
                        bool success = false;
                        if(bother>totalFatsoes)
                        {
                            bother-=totalFatsoes;

                            // increase total stat
                            totalFatsoes++;

                            // If it's from a connected client,
                            // terminate the connection. If not, it's an attack and
                            // we should rather ignore it.
                            for( int id=MAXCLIENTS; id > 0; --id )
                            {
                                if (sn_Connections[id].socket && peers[id] == addrFrom)
                                {
                                    sn_DisconnectUser( id, "$network_kill_error" );
                                    success=true;
                                }
                            }

                            // count the successfully removed client
                            if( success )
                            {
                                clientFatsoes++;
                            }

                            // scale down the stats
                            const float factor=.99;
                            totalFatsoes*=factor;
                            clientFatsoes*=factor;
                            bother*=factor;
                        }

                        if( !success )
                        {
                            // check for global and local spam (just for reporting, the packet
                            // is going to get blocked either way)
                            REAL severity = len*.5/MAX_MESS_LEN;
                            if( !GlobalConnectionFloodProtection( severity ) )
                            {
                                peers[ MAXCLIENTS+1] = addrFrom;
                                IndividualConnectionFloodProtection( NULL, MAXCLIENTS+1, severity );
                            }
                        }
                    }

                    // no use in processing the truncated packet. Some messages may get lost,
                    // but that's better than the inevitable network error and connection
                    // termination that expects us if we go on.
                    continue;
                }

                unsigned short *b=buff;
                unsigned short *bend=buff+(len/2-1);

                sn_ReceivedPackets++;
                sn_ReceivedBytes  += len + OVERHEAD;

                unsigned short claim_id=ntohs(*bend);

                // z-man: security check ( thanks, Luigi Auriemma! )
                if ( claim_id > MAXCLIENTS+1 )
                    continue;	// drop packet, maybe it was just truncated.

                /*
                  std::cerr << "Received " << len << " bytes";
                  con << " from user " << claim_id << '\n';
                */
                count ++;

                // set if only the first message of the packet is to be processed.
                bool onlyReadFirstMessage = false;

                unsigned int id=peer;
                //	 for(unsigned int i=1;i<=(unsigned int)maxclients;i++)
                int comp=nAddress::Compare( addrFrom, peers[claim_id] );
                if ( comp == 0 ) // || claim_id == MAXCLIENTS+1 )
                {
                    // everything seems allright. accept the id.
                    id = claim_id;
                }
                else
                {
                    // check for communication from last partner
                    if( claim_id > 0 && 0 == nAddress::Compare( addrFrom, lastPeers[claim_id] ) )
                    {
                        // ignore. The peer think it's still a client, but it's wrong.
                        // new login packets, pings etc. all come with claim_id == 0.
                        continue;
                    }

                    // logged in clients should ignore packets from unknown sources
                    if(sn_GetNetState() != nSERVER && sn_myNetID != 0)
                        continue;

                    // assume it's a new connection
                    id = MAXCLIENTS+1;
                    peers[ MAXCLIENTS+1 ] = addrFrom;
                    sn_Connections[ MAXCLIENTS+1 ].socket = sn_Connections[peer].socket;

// #define NO_GLOBAL_FLOODPROTECTION
#ifndef NO_GLOBAL_FLOODPROTECTION
                    // flood check for pings, logins and other potential nasties; as early as possible
                    if( sn_turtleMode && count > sn_connectionLimit*5 )
                    {
                        continue;
                    }

                    nMachine * machine = nMachine::PeekMachine( peer );

                    if( sn_GetNetState() == nSERVER )
                    {
                        // check whether we're currently getting flooded
                        GlobalConnectionFloodProtection();

                        if( sn_turtleMode || sn_synCookie )
                        {
                            // peek at descriptor
                            unsigned short descriptor = ntohs(*b);

                            // do some extra checks
                            if( descriptor == s_Acknowledge.ID() )
                            {
                                // this must be the cookie response triggered by the code below.
                                // allow it, but be careful to only read the first message.
                                onlyReadFirstMessage = true;
                            }
                            else if( descriptor == login_accept.ID() )
                            {
                                // Hah. Nice trick. Won't work, though.
                            }
                            else if( !sn_turtleMode &&
                                     ( descriptor == RequestSmallServerInfoDescriptor.ID() ||
                                       descriptor == RequestBigServerInfoDescriptor.ID() ) )
                            {
                                // Pings. Let them in unless we're under real attack.
                                onlyReadFirstMessage = true;
                            }
                            else if( !machine || !machine->IsValidated() )
                            {
                                if( count > sn_connectionLimit )
                                {
                                    continue;
                                }

                                // send fake login accept messages; the ack response whitelists the IP
                                nCookie cookie;
                                sn_SynGenerateCookie( sn_SynTimestamp(), peers[peer], cookie );
                                tJUST_CONTROLLED_PTR<nMessage> r = tNEW(nMessage)(login_accept);
                                r->BendMessageID( cookie.first );
                                r->SendImmediately(peer,false);
                                r = tNEW(nMessage)(login_accept);
                                r->BendMessageID( cookie.second );
                                r->SendImmediately(peer,false);
                                int idback = ::sn_myNetID;
                                sn_myNetID = 1; // set a fake ID so the client doesn't consider the packet as a response from the server and messes up its ack data
                                nMessage::SendCollected(peer);
                                ::sn_myNetID = idback;

                                // and ignore for now
                                continue;
                            }
                        }

                        // IP is not spoofed or there is no
                        // current spoof heavy attack. Check closer.
                        if( IndividualConnectionFloodProtection( machine, peer ) )
                        {
                            continue;
                        }
                    }
#endif
                }


                //	 if (peer!=id)
                //  con << "Changed incoming address.\n";
                int lenleft = bend - b;

#ifndef NOEXCEPT
                try
                {
#endif
                    while( lenleft >= 3 ){
                        tJUST_CONTROLLED_PTR< nMessage > pmess;
                        pmess = tNEW( nMessage )(b,id,lenleft);
                        nMessage& mess = *pmess;

                        lenleft = bend - b;

                        bool mess_is_new=true;
                        // see if we have got this packet before
                        unsigned short mess_id=mess.MessageID();

#ifdef DEBUG
                        if ( (simulate_loss && rand()%simulate_loss==0)){
                            // simulate packet loss
                            con << "Losing packet " << mess_id << ":" << id << ".\n";
                        }else
#endif
                            if(// (id==MAXCLIENTS+1 && !nCallbackAcceptPackedWithoutConnection::Accept( mess ) ) ||
                                // do not accept normal packages from the login
                                // slot; just login and information packets are allowed.
                                ( sn_GetNetState() != nSERVER && !login_succeeded && !nCallbackAcceptPackedWithoutConnection::Accept( mess ) )
                                // if we are not yet logged in, accept only login and login_deny.
                            )
                            {
                                //							con << "Ignoring packet " << mess_id << ":" << id << ".\n";
                            }
                            else
                            {
                                if (id <= MAXCLIENTS && mess_id != 0)  // messages with ID 0 are non-ack messages and come really often. they are always new.
                                {
                                    unsigned short diff=mess_id-highest_ack[id];
                                    if ( ( diff>0 && diff<10000 ) ||
                                            ((
                                                 mess.Descriptor() == login_accept.ID() ||
                                                 mess.Descriptor() == login_deny.ID()   ||
                                                 mess.Descriptor() == login.ID()
                                             ) && highest_ack[id] == 0)
                                       ){
                                        // the message has a more recent id than anything before.
                                        // it is surely new.
                                        highest_ack[id]=mess_id;
                                    }
                                    else{
                                        // do a better check
                                        for(int i=ACKBACK-1;i>=0;i--)
                                            if (mess_id==lastacks[id][i])
                                                mess_is_new=false;
                                    }
                                }


                                // acknowledge the message, even if it was old (the sender
                                // then thinks it got lost the first time)

                                // special situation: logout. Do not sent ack any more.
                                if ((!sn_Connections[id].socket))
                                {
                                    sn_Connections[id].ackMess=NULL;
                                }
                                else if (
#ifdef NO_ACK
                                    (mess.MessageID()) &&
#endif
                                    (mess.Descriptor()!=login_ignore.ID() ||
                                     login_succeeded )){
                                    // do not ack the login_ignore packet that did not let you in.

#ifdef DEBUG
                                    if ( id > MAXCLIENTS && mess.Descriptor() != login_accept.ID() )
                                    {
                                        con << "Sending ack to login slot.\n";
                                    }
#endif

                                    if(sn_Connections[id].ackMess==NULL)
                                    {
                                        sn_Connections[id].ackMess=new nAckMessage();
                                    }

                                    sn_Connections[id].ackMess->Write(mess.MessageID());
                                    if (sn_Connections[id].ackMess->DataLen()>100){
                                        sn_Connections[id].ackMess->Send(id, 0, false);
                                        sn_Connections[id].ackMess=NULL;
                                    }
                                }

                                if (mess_is_new){
                                    // mark the message as old
                                    if (mess_id > 0)
                                    {
                                        lastacks[id][lastackPos[id]]=mess_id;
                                        if(++lastackPos[id]>=ACKBACK) lastackPos[id]=0;
                                    }

                                    /*
                                    								// special situation: login. Change peer number permanently
                                    								if (peer==MAXCLIENTS+1 && new_id>0){
                                    									id=peer=new_id;
                                    								}
                                    */

                                    if (sn_GetNetState() != nSTANDALONE)
                                    {
                                        // store the message for later processing
                                        receivedMessages.push_back( pmess );
                                    }
                                }
                                //else
                                //con << "Message " << mess_id << ":" << id << " was not new.\n";
                            }

                        // abort if we're only supoosed to process the first message
                        if( onlyReadFirstMessage )
                        {
                            break;
                        }
                    }
#ifndef NOEXCEPT
                }

                catch(nKillHim const &)
                {
                    con << "nKillHim signal caught: ";
                    sn_DisconnectUser(id, "$network_kill_error");
                }
#endif
            }

            static int recursionCount = 0;
            int lastRecursionCount = recursionCount;
	#ifndef NOEXCEPT
            try
            {
	#endif
                ++recursionCount;

                // handle messages
                while ( receivedMessages.begin() != receivedMessages.end() )
                {
                    tJUST_CONTROLLED_PTR< nMessage > mess = receivedMessages.front();
                    receivedMessages.pop_front();

                    // perhaps the connection died?
                    if ( sn_Connections[ mess->SenderID() ].socket )
                        nDescriptor::HandleMessage( *mess );
                }
	#ifndef NOEXCEPT
            }

            catch(nKillHim const &)
            {
                con << "nKillHim signal caught: ";
                sn_DisconnectUser(peer, "$network_kill_error");
            }
	#endif

            if ( 0 == (recursionCount = lastRecursionCount) )
            {
                nCallbackReceivedComplete::ReceivedComplete();
            }
        }
    }
}

// receives and processes data from control socket
void sn_ReceiveFromControlSocket()
{
    rec_peer(0);
}

// discards data from control socket
void sn_DiscardFromControlSocket()
{
    // new facts: pending incoming data on the control socket causes the idle loops
    // to use 100% CPU time, we need to fetch and discard the data instead of ignoring it.
    if ( sn_Connections[0].socket )
    {
        int8 buff[2];
        nAddress addrFrom;
        sn_Connections[0].socket->Read( reinterpret_cast<int8 *>(buff),0, addrFrom);
    }
}


nNetState sn_GetNetState(){
    return current_state;
}

void clear_owners();

// tries to open listening sockets according to specification, but falls back to increasing ports
static bool sn_Listen( unsigned int & net_hostport, const tString& net_hostip )
{
    unsigned int net_hostport_before = net_hostport;

    try
    {
        nSocketListener & listener = sn_BasicNetworkSystem.AccessListener();

        listener.SetIpList( net_hostip );

        bool reported = false;

        // try ports in a range
        while ( net_hostport < sn_serverPort + 100 )
        {
            if ( listener.SetPort( net_hostport ).Listen( true ) )
                return true;

            if ( !reported )
            {
                con << "sn_SetNetState: Unable to open accept socket on desired port " << net_hostport << ", Trying next ports...\n";
                reported = true;

                // just for safety, wait a bit. Does not do much good.
                tDelay(100000);

                continue;
            }

            net_hostport++;
        }

        con << "sn_SetNetState: Giving up setting up listening sockets for IP list " << net_hostip << ".\n";
    }
    catch( const tException & e )
    {
        con << "sn_SetNetState: can't setup listening sockets. Reason given:\n"
        << e.GetName() << ": " << e.GetDescription() << "\n";
    }

    // reset host port
    net_hostport = net_hostport_before;

    return false;
}

// save and load machine info
static void sn_SaveMachines();
static void sn_LoadMachines();

static void sn_DisconnectAll()
{
    for(int i=MAXCLIENTS+1;i>=0;i--)
    {
        if( sn_Connections[i].socket )
        {
            sn_DisconnectUser(i, "$network_kill_shutdown");
            tVERIFY( !sn_Connections[i].socket );
        }
    }
    nCallbackLoginLogout::UserLoggedOut(0);
}

// flag set as long as the network sockets should not be closed and reopened
static bool sn_noReset = false;
nSocketResetInhibitor::nSocketResetInhibitor()
{
    sn_noReset = true;
}
nSocketResetInhibitor::~nSocketResetInhibitor()
{
    sn_noReset = false;
}

void sn_SetNetState(nNetState x){
    static bool reentry=false;
    if(!reentry && x!=current_state){
        sn_UpdateCurrentVersion();

        //if (x == nSERVER)
        unsigned int net_hostport = sn_serverPort;

        // save/load machines on entering/leaving server mode
        if ( x == nSERVER )
            sn_LoadMachines();
        else if ( current_state == nSERVER )
            if ( !tRecorder::IsPlayingBack() )
                sn_SaveMachines();

        reentry=true;
        if (x!=nSTANDALONE)
        {
            if (x==nCLIENT)
            {
                // sn_Connections[MAXCLIENTS+1].socket = NULL;
                sn_DisconnectAll();
            }
            else
            {
                sn_myNetID=0;
            }

            if (!sn_Connections[0].socket)
                sn_Connections[0].socket=sn_BasicNetworkSystem.Init();
            // bool success = true;
            if (x == nSERVER)
            {
                // bool success =
                sn_Listen( net_hostport, net_hostip ) ||    // first try: do it according to user specs
                sn_Listen( net_hostport, tString( "ANY" ) ) ||         // second try: bind to generic IP
                sn_Listen( net_hostport, tString( "ALL" ) );           // last try: bind to all available IPs

#ifdef DEDICATED
                // save host port that worked, otherwise it may change from the port sent to the master server
                sn_serverPort = net_hostport;
#endif
            }
        }
        else
        {
            clear_owners();
            for(int i=MAXCLIENTS+1;i>=0;i--){
                if(sn_Connections[i].socket){
                    if (i==0 && current_state!=nSERVER)
                    { // logout: fire and forget
                        con << tOutput("$network_logout_process");
                        for(int j=3;j>=0;j--){ // just to make sure
                            nMessage *lo=new nMessage(logout);
                            lo->Write(static_cast<unsigned short>(sn_myNetID));
                            lo->ClearMessageID();
                            lo->SendImmediately(0, false);
                            nMessage::SendCollected(0);
                            tDelay(1000);
                        }
                        con << tOutput("$network_logout_done");

                        sn_myNetID=0; // MAXCLIENTS+1; // reset network id
                    }
                }
                sn_DisconnectUserNoWarn(i, "$network_kill_shutdown");
            }

            // repeat to clear out pending stuff created during
            // the last run (destruction messages, for example)
            for(int i=MAXCLIENTS+1;i>=0;i--)
            {
                sn_DisconnectUserNoWarn(i, "$network_kill_shutdown");
            }

            sn_Connections[0].socket = 0;

            // shutdown network system to get new socket
            if ( !sn_noReset )
                sn_BasicNetworkSystem.Shutdown();
        }

        current_state=x;
        reentry=false;
    }

    sn_UpdateCurrentVersion();
}



// go to client mode and connect to server


void sn_Bend( nAddress const & address )
{
    if ((sn_GetNetState() == nSTANDALONE))
        sn_SetNetState(nCLIENT);

    peers[0] = address;
}

void sn_Bend( tString const & server, unsigned int port)
{
    // fill address info
    nAddress address;
    address.SetHostname( server );
    address.SetPort( port );

    // delegate
    sn_Bend( address );
}

nConnectError sn_Connect( nAddress const & server, nLoginType loginType, nSocket const * socket ){
    sn_DenyReason = "";

    // reset redirection
    sn_redirectTo.reset();

    // pings in the beginning of the login are not really representative
    nPingAverager::SetWeight(.0001);

    // net_hostport = sn_clientPort;

    sn_SetNetState(nSTANDALONE);
    sn_SetNetState(nCLIENT);

    // set user requested socket
    if ( socket )
        sn_Connections[0].socket = socket;

    sn_Connections[0].ping.Reset();

    peers[0] = server;

    reset_last_acks(0);
    nCallbackLoginLogout::UserLoggedOut(0);
    sn_Connections[0].sendBuffer_.Clear();

    tASSERT( sn_Connections[0].socket );

    // sn_Connections[0].socket->Connect( peers[0] ); // useless
    sn_Connections[0].bandwidthControl_.SetRate( sn_maxRateOut );

    sn_myNetID=0; // MAXCLIENTS+1; // reset network id

    // first, get all pending messages
    sn_Receive();
    sn_Receive();
    sn_Receive();

    // reset version control until the true value is given by the server.
    sn_currentVersion = nVersion(0,0);

    // Login stuff.....
    tJUST_CONTROLLED_PTR< nMessage > mess;
    if ( loginType != Login_Pre0252 )
    {
        mess=new nMessage(login_2);
        mess->Write(sn_maxRateIn);

        unsigned short bb = big_brother;
        mess->Write( bb );
        if ( bb ){
            (*mess) << sn_bigBrotherString;
            big_brother=false;
        }
    }
    else
    {
        mess=new nMessage(login);
        mess->Write(sn_maxRateIn);

        // send (worthless) big brother string
        if (big_brother)
        {
            (*mess) << sn_bigBrotherString;
        }
        else
        {
            (*mess) << tString("");
        }

        big_brother=false;
    }

    // write our version
    (*mess) << sn_MyVersion();

    // write our supported authentication methods
    (*mess) << nKrawall::nMethod::SupportedMethods();

    // write a random salt
    nKrawall::RandomSalt( loginSalt );
    nKrawall::WriteScrambledPassword( loginSalt, *mess );

    mess->ClearMessageID();
    mess->SendImmediately(0,false);
    nMessage::SendCollected(0);

    con << tOutput("$network_login_process");

    login_failed=false;
    login_succeeded=false;

    nTimeRolling timeout=tSysTimeFloat()+5;

    static REAL resend = .25;
    nTimeAbsolute nextSend = tSysTimeFloat() + resend/5;
    while(sn_GetNetState()==nCLIENT && tSysTimeFloat()<timeout &&
            !login_failed && !login_succeeded){
        if ( tSysTimeFloat() > nextSend )
        {
            // con << "retrying...\n";
            nextSend = tSysTimeFloat() + resend;
            mess->SendImmediately(0,false);
            nMessage::SendCollected(0);
        }

        tAdvanceFrame(10000);
        sn_Receive();
        sn_SendPlanned();

        // check for user abort
        if ( tConsole::Idle(true) )
        {
            con << tOutput("$network_login_failed_abort");
            sn_SetNetState(nSTANDALONE);
            return nABORT;
        }
    }
    if (login_failed)
    {
        con << tOutput("$network_login_failed");
        sn_SetNetState(nSTANDALONE);
        return nDENIED;
    }
    else if (tSysTimeFloat()>=timeout || sn_GetNetState()!=nCLIENT){
        if ( loginType == Login_All )
        {
            return 	sn_Connect( server, Login_Pre0252, socket );
        }
        else
        {
            con << tOutput("$network_login_failed_timeout");
            sn_SetNetState(nSTANDALONE);
            return nTIMEOUT;
        }
    }
    else{
        nCallbackLoginLogout::UserLoggedIn(0);

        if(sn_GetNetState() != nCLIENT)
        {
            return nDENIED;
        }

        tOutput mess;
        mess.SetTemplateParameter(1, sn_myNetID);
        mess << "$network_login_success";
        con << mess;
        con << tOutput("$network_login_sync");
        sn_Sync(40);

        if(sn_GetNetState() != nCLIENT)
        {
            return nDENIED;
        }

        con << tOutput("$network_login_relabeling");
        con << tOutput("$network_login_sync2");

        sn_Sync(40,true);

        if(sn_GetNetState() != nCLIENT)
        {
            return nDENIED;
        }

        con << tOutput("$network_login_done");

        // marginalize past ping values
        nPingAverager::SetWeight(1);

        return nOK;
    }
}


void nReadError( bool critical )
{
    // st_Breakpoint();
#ifndef NOEXCEPT
    if ( critical )
        throw nKillHim();
    else
        throw nIgnore();
#else
    con << "\nI told you not to use PGCC! Now we need to leave the\n"
    << "system in an undefined state. The progam will crash now.\n"
    << "\n\n";
#endif
}

#ifdef DEDICATED
static short sn_decorateID = true;
static tConfItem< short > sn_decorateIDConf( "CONSOLE_DECORATE_ID", sn_decorateID );

static short sn_decorateIP = false;
static tConfItem< short > sn_decorateIPConf( "CONSOLE_DECORATE_IP", sn_decorateIP );

static tConfItem< bool > sn_decorateTSConf( "CONSOLE_DECORATE_TIMESTAMP", sn_decorateTS );

// console with filter for better machine readable log format
class nConsoleFilter:public tConsoleFilter{
private:
    virtual void DoFilterLine( tString &line )
    {
        if ( sn_decorateID )
        {
            tString orig = line;

            int id = nCurrentSenderID::GetID();
            bool printIP = ( id > 0 && sn_decorateIP );

            line = "";
            line << "[";
            if ( sn_decorateID )
                line << id;
            if ( sn_decorateID && sn_decorateTS )
                line << " ";
            if ( sn_decorateTS )
                line << st_GetCurrentTime("TS=%Y/%m/%d-%H:%M:%S");
            if ( (sn_decorateID || sn_decorateTS) && printIP )
                line << " ";
            if ( printIP )
            {
                // get IP from id
                tString IP;
                sn_GetAdr( id,  IP );
                line << "IP=" << IP;
            }

            line << "] " << orig;
        }
    }
};

static nConsoleFilter sn_consoleFilter;
#endif

static bool sg_playerMessageMatchWinner = false;
static tConfItem<bool> sg_playerMessageMatchWinnerConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_MATCH_WINNER", sg_playerMessageMatchWinner);

static bool sg_playerMessageMatchWinnerSelf = false;
static tConfItem<bool> sg_playerMessageMatchWinnerSelfConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_MATCH_WINNER_SELF", sg_playerMessageMatchWinnerSelf);

bool sg_playerSpamProtectionWatch = false;
static tConfItem<bool> sg_playerSpamProtectionWatchConf("CHAT_SPAM_PROTECTION_WATCH", sg_playerSpamProtectionWatch);

static tString sg_playerSpamProtectionWatchSearchString("");
static tConfItem<tString> sg_playerSpamProtectionWatchSearchStringConf("CHAT_SPAM_PROTECTION_WATCH_SEARCH_STRING", sg_playerSpamProtectionWatchSearchString);

static tString sg_playerSpamProtectionWatchCommonPrefixSearchString("");
static tConfItem<tString> sg_playerSpamProtectionWatchCommonPrefixSearchStringConf("CHAT_SPAM_PROTECTION_WATCH_COMMON_PREFIX_SEARCH_STRING", sg_playerSpamProtectionWatchCommonPrefixSearchString);

static tString sg_playerSpamProtectionWatchRepeatSearchString("");
static tConfItem<tString> sg_playerSpamProtectionWatchRepeatSearchStringConf("CHAT_SPAM_PROTECTION_WATCH_REPEAT_SEARCH_STRING", sg_playerSpamProtectionWatchRepeatSearchString);

static REAL sg_playerSpamProtectionWatchRepeatWaitTime = 0;
static tConfItem<REAL> sg_playerSpamProtectionWatchRepeatWaitTimeConf("CHAT_SPAM_PROTECTION_WATCH_REPEAT_WAIT_TIME", sg_playerSpamProtectionWatchRepeatWaitTime);

static bool currentlySuspended = false;
static bool currentlySilenced = false;

bool validateSuspended()
{
    for (auto localnetp : se_GetLocalPlayers())
    {
        ePlayer *localp = ePlayer::NetToLocalPlayer(localnetp);
        if (localnetp && !localp->spectate && !localnetp->CurrentTeam())
            return true;
    }
    return false;
}

static void sn_ConsoleOut_handler(nMessage &m)
{
    if (sn_GetNetState() != nSERVER)
    {
        tString s;
        m >> s;
        con << s;

        if (sg_playerSilencedWatch)
        {
            if (s.Contains(tString("silenced by the server administrator.")) && (ePlayerMessages.CheckIfSilenced())) 
            {
                gHelperUtility::Debug("PLAYER_WATCH_SILENCED","Player was silenced.");
                sn_quitAction(true, sg_playerSilencedWatchQuit,"Player was silenced.");
            }
        }

        if (sn_playerSuspendWatch)
        {
            if (s.Contains(tString("suspended from playing for the next")) && validateSuspended())
            {
                gHelperUtility::Debug("PLAYER_WATCH_SUSPENDED","Player was suspended.");
                bool canQuit = !sn_playerSuspendWatchQuitWaitForAvoidPlayers || !avoidPlayerInGame();
                if (canQuit)
                {
                    sn_quitAction(true, sn_playerSuspendWatchQuit, "Player was suspended.");
                }
                else if (se_playerWatchAutoRandomName)
                {
                    gHelperUtility::Debug("PLAYER_WATCH_SUSPENDED","Setting forceRandomRename to true..");
                    forceRandomRename = true;
                }

                if (!canQuit)
                {
                    gHelperUtility::Debug("PLAYER_WATCH_SUSPENDED","Waiting for avoid players to leave (" + se_avoidPlayerWatchList.stdString() + ")..");
                }
            }
        }

        if (sn_playerUnableToRenameWatch)
        {
            if (s.Contains(tString("not allowed to rename to ")))
            {
                gHelperUtility::Debug("PLAYER_WATCH_RENAME","Unable to rename.");
                if (sn_playerUnableToRenameWatchRebuild)
                {
                    gHelperUtility::Debug("PLAYER_WATCH_RENAME","Running a complete rebuild.");
                    ePlayerNetID::CompleteRebuild();
                }
            }
        }

        if (sg_playerSpamProtectionWatch || (se_playerMessageTriggers && se_playerMessageTriggersResendSilencedMessages))
        {
            tString input = (sg_playerSpamProtectionWatchSearchString.empty() ? tString("silenced for the next ") : sg_playerSpamProtectionWatchSearchString);

            int startIdx = s.StrPos(input);

            if (startIdx != -1)
            {
                int startPos = startIdx + input.Len() - 1;

                while (startPos < s.Len() && s(startPos) == ' ')
                    startPos++;

                int endIdx = startPos;
                while (endIdx < s.Len() && !isblank(s(endIdx)) && s(endIdx) != '\n')
                    endIdx++;

                int len = endIdx - startPos;

                tString secondsStr = s.SubStr(startPos, len);

                std::istringstream stream(secondsStr.stdString());
                REAL seconds;
                if (stream >> seconds)
                {
                    ePlayerNetID::setNextSpeakTime(seconds);
                    gHelperUtility::Debug("SpamProtectionWatch", "Silenced detected. Next speak time for " +
                                                                 std::to_string(seconds + sg_playerSpamProtectionWatchExtraAdd) +
                                                                 " seconds");
                    if (se_playerMessageTriggers && se_playerMessageTriggersResendSilencedMessages)
                        eChatBot::SilencedAction();
                }
                else
                    gHelperUtility::Debug("SpamProtectionWatch", "Silence detected. Failed to determine number of seconds.");
            }

            input = (sg_playerSpamProtectionWatchCommonPrefixSearchString.empty() ? tString("your messages have a common prefix: ") : sg_playerSpamProtectionWatchCommonPrefixSearchString);

            startIdx = s.StrPos(input);

            if (startIdx != -1)
            {
                int startPos = startIdx + input.Len() - 1;

                while (startPos < s.Len() && s(startPos) == ' ')
                    startPos++;

                int endIdx = s.StrPos(startPos, "\nSPAM PROTECTION:");
                if (endIdx == -1)
                    endIdx = s.Len();

                int len = endIdx - startPos;

                tString prefix = s.SubStr(startPos, len);

                int secondsStartIdx = endIdx + strlen("\nSPAM PROTECTION: messages with this prefix will be allowed again in ");

                while (secondsStartIdx < s.Len() && s(secondsStartIdx) == ' ')
                    secondsStartIdx++;

                int secondsEndIdx = secondsStartIdx;
                while (secondsEndIdx < s.Len() && !isblank(s(secondsEndIdx)) && s(secondsEndIdx) != '\n')
                    secondsEndIdx++;

                int secondsLen = secondsEndIdx - secondsStartIdx;

                tString secondsStr = s.SubStr(secondsStartIdx, secondsLen);

                std::istringstream stream(secondsStr.stdString());
                REAL seconds;
                if (stream >> seconds)
                {
                    ePlayerNetID::setNextSpeakTimeCommonPrefix(prefix, seconds);
                    gHelperUtility::Debug("SpamProtectionWatch", "Common prefix detected: " +
                                                                 prefix.stdString() +
                                                                 ", Common Prefix Silence Set for " +
                                                                 std::to_string(seconds + sg_playerSpamProtectionWatchExtraAdd) +
                                                                 " seconds");
                    if (se_playerMessageTriggers && se_playerMessageTriggersResendSilencedMessages)
                        eChatBot::SilencedAction();
                }
                else
                    gHelperUtility::Debug("SpamProtectionWatch", "Common prefix detected. Failed to determine number of seconds.");
            }

            input = (sg_playerSpamProtectionWatchRepeatSearchString.empty() ? tString("you already said: ") : sg_playerSpamProtectionWatchRepeatSearchString);

            startIdx = s.StrPos(input);

            if (startIdx != -1)
            {
                int startPos = startIdx + input.Len() - 1;

                while (startPos < s.Len() && s(startPos) == ' ')
                    startPos++;

                int endIdx = s.StrPos(startPos, "\n");
                if (endIdx == -1)
                    endIdx = s.Len();

                int len = endIdx - startPos;

                tString repeatedMessage = s.SubStr(startPos, len);

                REAL seconds = sg_playerSpamProtectionWatchRepeatWaitTime;
                ePlayerNetID::setNextSpeakTimeCommonPrefix(repeatedMessage, seconds);
                gHelperUtility::Debug("SpamProtectionWatch", "Repeated Message Detected: '" +
                                                             repeatedMessage.stdString() +
                                                             "', Silence Set for " +
                                                             std::to_string(seconds + sg_playerSpamProtectionWatchExtraAdd) +
                                                             " seconds");
                if (se_playerMessageTriggers && se_playerMessageTriggersResendSilencedMessages)
                    eChatBot::SilencedAction();
            }
        }

        if ((se_playerMessageTriggers && sg_playerMessageMatchWinner) || se_playerStats)
        {
            if (s.Contains("Overall Winner:"))
            {
                con << s << "\n";

                ePlayerNetID *potentialWinner = ePlayerNetID::HighestScoringPlayer();
                bool isLocal = sg_playerMessageMatchWinnerSelf && potentialWinner && potentialWinner->isLocal();

                if (se_playerStats)
                    ePlayerStats::updateStatsMatchEnd(potentialWinner);

                if (se_playerMessageTriggers && sg_playerMessageMatchWinner)
                    eChatBot::InitiateAction(potentialWinner, isLocal ? tString("$matchwinnerself") : tString("$matchwinner"), true);
            }
        }
    }
}

static nDescriptor sn_ConsoleOut_nd(8,sn_ConsoleOut_handler,"sn_ConsoleOut");

// rough maximal packet size, better send nothig bigger, or it will
// get fragmented.
#define MTU 1400

// causes the connected clients to print a message
nMessage* sn_ConsoleOutMessage( tString const & message )
{
    // truncate message to about 1.5K, a safe size for all UDP packets
    static const int maxLen = MTU+100;
    static bool recurse = true;
    if ( message.Len() > maxLen && recurse )
    {
        recurse = false;
        tERR_WARN( "Long console message truncated.");
        nMessage * m = sn_ConsoleOutMessage( message.SubStr( 0, MTU ) );
        recurse = true;
        return m;
    }

    nMessage* m=new nMessage(sn_ConsoleOut_nd);
    *m << message;

    return m;
}

void sn_ConsoleOutRaw( tString & message,int client){
    tJUST_CONTROLLED_PTR< nMessage > m = sn_ConsoleOutMessage( message );

    if (client<0){
        m->BroadCast();
        con << message;
    }
    else if (client==sn_myNetID)
    {
        con << message;
    }
    else
    {
        m->Send(client);
    }
}

void sn_ConsoleOutString( tString & message,int client){
    // Make sure string has valid color codes
    if(st_verifyColorCodeStrictly)
    {
        message = tColoredString::EscapeBadColors(message);
    }

    // check if string is too long
    if ( message.Len() <= MTU )
    {
        // no? Fine, just send it in one go.
        message  << "0xffffff";
        sn_ConsoleOutRaw( message, client );

        return;
    }

    // darn, it is too long. Try to find a good spot to cut it
    int cut = MTU;
    while ( cut > 0 && message(cut) != '\n' )
    {
        --cut;
    }
    if ( cut == 0 )
    {
        // no suitable spot found, just cut anywhere.
        cut = MTU;
    }

    // split the string
    tString beginning = message.SubStr( 0, cut ) + "0xffffff";
    tString rest = message.SubStr( cut );

    // and send the bits
    sn_ConsoleOutRaw( beginning, client );
    sn_ConsoleOutString( rest, client );
}

void sn_ConsoleOut(const tOutput& o,int client){
    // transform message to string
    tString message( o );
    sn_ConsoleOutString( message, client );
}

bool roundWinnerProcessed = false;

static void client_cen_handler(nMessage &m)
{
    if (sn_GetNetState() != nSERVER)
    {
        tString s;
        m >> s;
        con.CenterDisplay(s);

        if (se_playerMessageTriggers && sg_playerMessageMatchWinner || se_playerStats)
        {
            if (s.Contains("Winner:"))
            {
                con << s << "\n";

                roundWinnerProcessed = true;
                if (se_playerStats)
                    ePlayerStats::updateStatsRoundEnd();
            }
        }
    }
}

static nDescriptor client_cen_nd(9,client_cen_handler,"client_cen");

// causes the connected clients to print a message in the center of the screeen
void sn_CenterMessage(const tOutput &o,int client){
    tString message(o);

    // Make sure string has valid color codes
    if(st_verifyColorCodeStrictly)
    {
        message = tColoredString::EscapeBadColors(message);
    }

    tJUST_CONTROLLED_PTR< nMessage > m=new nMessage(client_cen_nd);
    *m << message;
    if (client<0){
        m->BroadCast();
        con.CenterDisplay(message);
    }
    else if (client==sn_myNetID)
        con.CenterDisplay(message);
    else
        m->Send(client);
}

static void ConsoleOut_conf(std::istream &s)
{
    // read the message
    tString message;
    message.ReadLine( s, true );

    message += "\n";

    // display it
    sn_ConsoleOut( message );
}

static tConfItemFunc ConsoleOut_c("CONSOLE_MESSAGE",&ConsoleOut_conf);
static tAccessLevelSetter sn_ConsoleConfLevel( ConsoleOut_c, tAccessLevel_Moderator );

static void CeterMessage_conf(std::istream &s)
{
    // read the message
    tString message;
    message.ReadLine( s, true );

    // display it
    sn_CenterMessage( message );
}

static tConfItemFunc CenterMessage_c("CENTER_MESSAGE",&CeterMessage_conf);
static tAccessLevelSetter sn_CenterConfLevel( CenterMessage_c, tAccessLevel_Moderator );

planned_send::planned_send(REAL priority,int Peer){
    peer=Peer;

    SetVal( priority, send_queue[peer] );
}

planned_send::~planned_send(){
    RemoveFromHeap();
}

tHeapBase *planned_send::Heap() const
{
    return &send_queue[peer];
}

// change our priority:
void planned_send::add_to_priority(REAL diff)
{
    SetVal( Val() + diff, send_queue[peer] );
}

// **********************************************

nMessage_planned_send::nMessage_planned_send
(nMessage *M,REAL priority,bool Ack,int Peer)
        :planned_send(priority,Peer),m(M),ack(Ack){
    //if (m)
}

nMessage_planned_send::~nMessage_planned_send(){
}

void nMessage_planned_send::execute(){
    if ( Val() < -killTimeout-10){
        tOutput mess;
        mess.SetTemplateParameter(1, peer);
        mess << "$network_error_overflow";
        con << mess;
        st_Breakpoint();
        sn_DisconnectUser(peer, "$network_kill_overflow");
    }
    else if (m)
        m->SendImmediately(peer,ack);
}


// **********************************************

static REAL sn_SendPlanned1(){
    sn_OrderPriority = 0;

    // if possible, send waiting messages
    static double lastTime=-1;
    nTimeAbsolute time=tSysTimeFloat();
    if (lastTime<0)
        lastTime=time;

    if (time<lastTime-.01 || time>lastTime+1000)
#ifdef DEBUG
    {
        tERR_ERROR("Timer hiccup!");
    }
#else
    {
        tERR_WARN("Timer hiccup!");
        lastTime=time;
    }
#endif
    REAL dt = time - lastTime;

    //for(int i=MAXCLIENTS+1;i>=0;i--){
    for(int i=0;i<=MAXCLIENTS+1;i++){
        nConnectionInfo & connection = sn_Connections[i];
        if ( !connection.socket )
            continue;

        while (connection.ackPending<sn_maxNoAck &&
                connection.bandwidthControl_.CanSend()     &&
                send_queue[i].Len())
        {
            send_queue[i](0)->execute();
            if (send_queue[i].Len())
                delete send_queue[i](0);
        }

        // make everything a little more urgent:
        for(int j=send_queue[i].Len()-1;j>=0;j--)
            send_queue[i](j)->add_to_priority(-dt);
    }
    lastTime=time;

    return dt;
}

static void sn_SendPlanned2( REAL dt ){
    // empty the send buffers
    for(int i=0;i<=MAXCLIENTS+1;i++){
        nConnectionInfo & connection = sn_Connections[i];
        if ( connection.socket )
        {
            if (connection.sendBuffer_.Len()>0 && connection.bandwidthControl_.CanSend() )
                nMessage::SendCollected(i);

            // update bandwidth usage and other time related data
            connection.Timestep( dt );
        }
    }
}

void sn_SendPlanned()
{
    // propagate messages to buffers
    REAL dt = sn_SendPlanned1();

    // schedule the acks: send them if it's possible (bandwith limit) or if there already is a packet in the pipe.
    for(int i=0;i<=MAXCLIENTS+1;i++)
        if(sn_Connections[i].socket && sn_Connections[i].ackMess && !sn_Connections[i].ackMess->End()
                //	&& sn_ackAckPending[i] <= 1+sn_Connections[].ackMess[i]->DataLen()
                && ( sn_Connections[i].bandwidthControl_.CanSend() || sn_Connections[i].sendBuffer_.Len() > 0 )
          ){
            sn_Connections[i].ackMess->SendImmediately(i, false);
            sn_Connections[i].ackMess=NULL;
        }

    // schedule lost messages for resending
    nWaitForAck::Resend();

    // send everything out
    sn_SendPlanned2( dt );
}

void sn_Receive(){
    /*
      static bool reentry=false;
      if (reentry)
      return;
      reentry=true;
    */

    netTime=tSysTimeFloat();
    //	new_id=0;
    sn_Connections[MAXCLIENTS+1].ping.Reset();

    // create the ack messages (not required, is done on demand later)
    /*
    int i;
    for(i=0;i<=MAXCLIENTS+1;i++)
        if(sn_Connections[i].ackMess==NULL)
            sn_Connections[i].ackMess=new nAckMessage();
    */


    switch (current_state){
    case nSERVER:
        {
            peers[0] = nAddress{};

            // listen on all sockets
            nSocketListener const & listener = sn_BasicNetworkSystem.GetListener();
            for ( nSocketListener::iterator i = listener.begin(); i != listener.end(); ++i )
            {
                // clear peer info used for receiving
                peers[MAXCLIENTS+1] = nAddress{};

                // copy socket info over to [MAXCLIENTS+1] and receive. The copy
                // step is important, nAuthentication.cpp relies on the socket being set.
                if((sn_Connections[MAXCLIENTS+1].socket = (*i).CheckNewConnection() ) != NULL)
                {
                    rec_peer(MAXCLIENTS+1);
                    sn_Connections[MAXCLIENTS+1].socket = NULL;
                }
            }
        }
        // z-man: after much thought, the server does also need to listen to the
        // network control socket. .... Thinking again, it's only important for the master
        // servers, and they call rec_peer(0) separately.
        break;

    case nCLIENT:
        rec_peer(0);
        break;

    case nSTANDALONE:
    default:
        break;
    }

    /*
        // scedule regular messages
        REAL dt = sn_SendPlanned1();

        // actually resend messages
        sn_SendPlanned2( dt );
    */
}

void sn_KickUser(int i, const tOutput& reason, REAL severity, nServerInfoBase * redirectTo )
{
    // print it
    con << tOutput( "$network_kill_log", i, reason );

    // log it
    if ( severity > 0 )
    {
        nMachine::GetMachine(i).OnKick( severity );
    }

    // do it
    sn_DisconnectUser( i, reason, redirectTo );
}

void sn_DisconnectUser(int i, const tOutput& reason, nServerInfoBase * redirectTo )
{
    // don't be daft and kill yourself, server!
    if ( i == 0 && sn_GetNetState() == nSERVER )
    {
        tERR_WARN( "Server tried to disconnect from itself." );
        return;
    }

    // clients can only disconnect from the server
    if ( i != 0 && i <= MAXCLIENTS && sn_GetNetState() == nCLIENT )
    {
        tERR_WARN( "Client tried to disconnect from another client: impossible and a bad idea." );
        return;
    }

    // anything to do at all?
    if (!sn_Connections[i].socket)
    {
        return;
    }

    sn_DisconnectUserNoWarn( i, reason, redirectTo );
}

void sn_DisconnectUserNoWarn(int i, const tOutput& reason, nServerInfoBase * redirectTo )
{
    if(i < 0 || i > MAXCLIENTS+1)
        return;

    nCurrentSenderID senderID( i );

    nWaitForAck::AckAllPeer(i);

    static bool reentry=false;
    if (reentry)
        return;
    reentry=true;

    bool printMessage = false; // is it worth printing a message for this event?

    tString reasonString( reason );

    if (sn_Connections[i].socket)
    {
        // store IP:port for later
        lastPeers[i] = peers[i];

        nMessage::SendCollected(i);

        // send disconnect message to peer
        if ( i!=0 && sn_GetNetState() == nSERVER ){
            printMessage = true;
            for(int j=2;j>=0;j--){
                nMessage* mess = (new nMessage(login_deny));
                mess->ClearMessageID();
                *mess << reasonString;

                // write redirection
                tString redirection;
                int port = 0;
                if ( redirectTo )
                {
                    redirection = redirectTo->GetConnectionName();
                    port        = redirectTo->GetPort();
                }
                *mess << redirection;
                *mess << port;

                mess->SendImmediately(i, false);
                nMessage::SendCollected(i);
            }
        }
    }

    nWaitForAck::AckAllPeer(i);

    sn_Connections[i].ackMess=NULL;

    if (i==0 && sn_GetNetState()==nCLIENT)
        sn_SetNetState(nSTANDALONE);

    reset_last_acks(i);

    // peers[i].sa_family=0;

    sn_Connections[i].ackPending=0;
    //  sn_ackAckPending[i]=0;

    nCallbackLoginLogout::UserLoggedOut(i);

    if ( printMessage )
    {
        con << tOutput( "$network_killuser", i, sn_Connections[i].ping.GetPing(), peers[i].ToString(), reasonString );
    }

    // clear address, socket and send queue
    sn_Connections[i].sendBuffer_.Clear();
    sn_Connections[i].socket=NULL;
    peers[i] = nAddress();
    sn_Connections[i].Clear();
    while (send_queue[i].Len())
        delete (send_queue[i](0));

    reentry=false;

    sn_UpdateCurrentVersion();
}


int sn_QueueLen(int user){
    return send_queue[user].Len();
}


static tCallback* s_loginoutAnchor=NULL;
int  nCallbackLoginLogout::user;
bool nCallbackLoginLogout::login;

nCallbackLoginLogout::nCallbackLoginLogout(VOIDFUNC *f)
        :tCallback(s_loginoutAnchor,f){}

void nCallbackLoginLogout::UserLoggedIn(int u){
    bool loginBack = login;
    int userBack = user;

    login = true;
    user = u;
    Exec(s_loginoutAnchor);

    login = loginBack;
    user = userBack;
}

void nCallbackLoginLogout::UserLoggedOut(int u){
    bool loginBack = login;
    int userBack = user;

    login = false;
    user = u;
    Exec(s_loginoutAnchor);

    login = loginBack;
    user = userBack;
}

unsigned short nCallbackAcceptPackedWithoutConnection::descriptor=0;	// the descriptor of the incoming packet
static tCallbackOr* s_AcceptAnchor=NULL;

nCallbackAcceptPackedWithoutConnection::nCallbackAcceptPackedWithoutConnection(BOOLRETFUNC *f)
        : tCallbackOr( s_AcceptAnchor, f )
{
}

bool nCallbackAcceptPackedWithoutConnection::Accept( const nMessage& m )
{
    descriptor=m.Descriptor();
    return Exec( s_AcceptAnchor );
}

static tCallback* s_ReceivedCompleteAnchor=NULL;

nCallbackReceivedComplete::nCallbackReceivedComplete(VOIDFUNC *f)
        : tCallback( s_ReceivedCompleteAnchor, f )
{
}

void nCallbackReceivedComplete::ReceivedComplete( )
{
    Exec( s_ReceivedCompleteAnchor );
}

static bool net_Accept()
{
    return
        nCallbackAcceptPackedWithoutConnection::Descriptor()==login_accept.ID() ||
        nCallbackAcceptPackedWithoutConnection::Descriptor()==login_deny.ID();
}

static nCallbackAcceptPackedWithoutConnection net_acc( &net_Accept );

static void net_exit(){
    for (int i=MAXCLIENTS+1;i>=0;i--)
    {
        sn_Connections[i].ackMess = NULL;
        while (send_queue[i].Len())
            delete send_queue[i].Remove(0);
    }
}

static tInitExit net_ie(NULL, &net_exit);



void sn_Statistics()
{
    nTimeRolling time = tSysTimeFloat();
    REAL dt = time - sn_StatsTime;
    sn_StatsTime = time;

    if (dt > 0 && (sn_SentPackets || sn_SentBytes))
    {
        tOutput o;
        o.SetTemplateParameter(1,dt);
        o.SetTemplateParameter(2,sn_SentBytes);
        o.SetTemplateParameter(3,sn_SentPackets);
        o.SetTemplateParameter(4,sn_SentBytes/dt);
        o.SetTemplateParameter(5,sn_ReceivedBytes);
        o.SetTemplateParameter(6,sn_ReceivedPackets);
        o.SetTemplateParameter(7,sn_ReceivedBytes/dt);
        o << "$network_statistics1";
        o << "$network_statistics2";
        o << "$network_statistics3";

        con << o;
    }

    sn_SentPackets = 0;
    sn_SentBytes   = 0;
    sn_ReceivedPackets = 0;
    sn_ReceivedBytes   = 0;
}






nConnectionInfo::nConnectionInfo(){Clear();}
nConnectionInfo::~nConnectionInfo(){}

void nConnectionInfo::Clear(){
    socket     = NULL;
    ackPending = 0;
    ping.Reset();
    // crypt      = NULL;

    supportedAuthenticationMethods_ = "";

    sendBuffer_.Clear();

    bandwidthControl_.Reset();

    ackMess = NULL;

    // userName.SetLen(0);

    // start with 10% packet loss with low statistical weight
    packetLoss_.Reset();
    packetLoss_.Add(.1,10);
}

void nConnectionInfo::Timestep( REAL dt )  //!< call whenever an an reliable message got sent
{
    // update ping
    ping.Timestep( dt );

    // update bandwidth control
    bandwidthControl_.Update( dt );

    // update packet loss; average about a minute
    packetLoss_.Timestep( .02 * dt );
}

void nConnectionInfo::ReliableMessageSent()  //!< call whenever an an reliable message got sent
{
    packetLoss_.Add( 1 );
}

void nConnectionInfo::AckReceived()          //!< call whenever an ackownledgement message arrives
{
    packetLoss_.Add( -1 );
}

REAL nConnectionInfo::PacketLoss() const     //!< returns the average packet loss ratio
{
    if(tRecorder::DesyncedPlayback())
        return 0;
    REAL ret = packetLoss_.GetAverage();
    return ret > 0 ? ret : 0;
}

void sn_GetAdr(int user,  tString& name)
{
    peers[user].ToString( name );
}

unsigned int sn_GetPort(int user)
{
    return peers[user].GetPort();
}

unsigned int sn_GetServerPort()
{
    return sn_serverPort;
}

int sn_NumUsers( bool all )
{
    int ret = 0;
    for (int i=MAXCLIENTS; i>0; i--)
        if (sn_Connections[i].socket && ( all || ( sn_allowSameIPCountSoft > CountSameIP( i ) ) ) )
            ret++;

#ifndef DEDICATED
    ret++;
#endif

    return ret;
}

int sn_NumUsers()
{
    return sn_NumUsers( true );
}

int sn_NumRealUsers()
{
    return sn_NumUsers( false );
}

int sn_MaxUsers()
{
    return sn_maxClients;
}

int sn_MessagesPending(int user)
{
    return sn_Connections[user].ackPending + send_queue[user].Len();
}

nBasicNetworkSystem sn_BasicNetworkSystem;

// *******************************************************************************************
// *
// *	nKillHim
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nKillHim::nKillHim( void )
{
}

// *******************************************************************************************
// *
// *	~nKillHim
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nKillHim::~nKillHim( void )
{
}

// *******************************************************************************************
// *
// *	DoGetName
// *
// *******************************************************************************************
//!
//!		@return		short name
//!
// *******************************************************************************************

tString nKillHim::DoGetName( void ) const
{
    return tString( "Connektion kill request" );
}

// *******************************************************************************************
// *
// *	DoGetDescription
// *
// *******************************************************************************************
//!
//!		@return		description
//!
// *******************************************************************************************

tString nKillHim::DoGetDescription( void ) const
{
    return tString( "The currently handled peer must have done something illegal, so it should be terminated." );
}

// *******************************************************************************************
// *
// *	nIgnore
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nIgnore::nIgnore( void )
{
}

// *******************************************************************************************
// *
// *	~nIgnore
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nIgnore::~nIgnore( void )
{
}

// *******************************************************************************************
// *
// *	DoGetName
// *
// *******************************************************************************************
//!
//!		@return		short name
//!
// *******************************************************************************************

tString nIgnore::DoGetName( void ) const
{
    return tString( "Packet ignore request" );
}

// *******************************************************************************************
// *
// *	DoGetDescription
// *
// *******************************************************************************************
//!
//!		@return		description
//!
// *******************************************************************************************

tString nIgnore::DoGetDescription( void ) const
{
    return tString( "An error that should lead to the current message getting ingored was detected." );
}

// *******************************************************************************************
// *
// *	nAverager
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nAverager::nAverager( void )
        : weight_(0), sum_(0), sumSquared_(0), weightSquared_(0)
{
}

// *******************************************************************************************
// *
// *	~nAverager
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nAverager::~nAverager( void )
{
}

// *******************************************************************************************
// *
// *	Timestep
// *
// *******************************************************************************************
//!
//!		@param	decay	decay factor 0 .. infinity; larger values lead to more decay.
//!
// *******************************************************************************************

void nAverager::Timestep( REAL decay )
{
    REAL factor = 1/(1+decay);

    // pretend all data so far was collected with a weight of the original weight multiplied by factor
    weight_        *= factor;
    sum_           *= factor;
    sumSquared_    *= factor;
    weightSquared_ *= factor * factor;
}

// *******************************************************************************************
// *
// *	Add
// *
// *******************************************************************************************
//!
//!		@param	value	 the value to add
//!		@param	weight   its statistical weight (importance compared to other values)
//!
// *******************************************************************************************

void nAverager::Add( REAL value, REAL weight )
{
    tASSERT( weight >= 0 );
    weight_        += weight;
    sum_           += weight * value;
    sumSquared_    += weight * value * value;
    weightSquared_ += weight * weight;
}

// *******************************************************************************************
// *
// *	Reset
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

void nAverager::Reset( void )
{
    weightSquared_ = weight_ = sum_ = sumSquared_ = 0.0f;
}

// *******************************************************************************************
// *
// *	GetAverage
// *
// *******************************************************************************************
//!
//!		@return		the average value over the last time
//!
// *******************************************************************************************

REAL nAverager::GetAverage( void ) const
{
    if ( weight_ > 0 )
        return sum_ / weight_;
    else
        return 0;
}

// *******************************************************************************************
// *
// *	GetDataVariance
// *
// *******************************************************************************************
//!
//!		@return		the average recent variance in the incoming data
//!
// *******************************************************************************************

REAL nAverager::GetDataVariance( void ) const
{
    if ( weight_ > 0 )
    {
        REAL average       = sum_ / weight_;
        REAL averageSquare = sumSquared_ / weight_;
        REAL ret = averageSquare - average * average;
        if ( ret < 0 )
            ret = 0;
        return ret;
    }
    else
        return 0;
}

// *******************************************************************************************
// *
// *	GetAverageVariance
// *
// *******************************************************************************************
//!
//!		@return		the expected variance of the return value of GetAverage()
//!
// *******************************************************************************************

REAL nAverager::GetAverageVariance( void ) const
{
    if ( weight_ > 0 )
    {
        REAL square = weight_ * weight_;

        REAL denominator = square - weightSquared_;
        REAL numerator = GetDataVariance() * weightSquared_;
        if ( denominator > numerator * 1E-30 )
        {
            return numerator/denominator;
        }
        else
            return 1E+30;
    }
    else
        return 0;
}

// *******************************************************************************************
// *
// *	GetWeight
// *
// *******************************************************************************************
//!
//!		@return		the current total weight
//!
// *******************************************************************************************

REAL nAverager::GetWeight( void ) const
{
    return weight_;
}

// *******************************************************************************
// *
// *	operator <<
// *
// *******************************************************************************
//!
//!		@param	stream	stream to read from
//!		@return		    stream for chaining
//!
// *******************************************************************************

std::istream & nAverager::operator <<( std::istream & stream )
{
    char c;
    stream >> c;
    tASSERT( c == '(' );

    stream >> weight_ >> sum_ >> sumSquared_ >> weightSquared_;

    stream >> c;
    tASSERT( c == ')' );

    return stream;
}

// *******************************************************************************
// *
// *	operator >>
// *
// *******************************************************************************
//!
//!		@param	stream	stream to write to
//!		@return		    stream for chaining
//!
// *******************************************************************************

std::ostream & nAverager::operator >>( std::ostream & stream ) const
{
    stream << '(' << weight_ << ' ' << sum_  << ' ' << sumSquared_  << ' ' << weightSquared_  << ')';

    return stream;
}

// *******************************************************************************
// *
// *	operator >>
// *
// *******************************************************************************
//!
//!		@param	stream	stream to read to
//!		@param  averager averager to read
//!		@return		    stream for chaining
//!
// *******************************************************************************

std::istream & operator >> ( std::istream & stream, nAverager & averager )
{
    return averager << stream;
}

// *******************************************************************************
// *
// *	operator <<
// *
// *******************************************************************************
//!
//!		@param	stream	stream to write to
//!		@param  averager averager to write
//!		@return		    stream for chaining
//!
// *******************************************************************************

std::ostream & operator << ( std::ostream & stream, nAverager const & averager )
{
    return averager >> stream;
}


// *******************************************************************************************
// *
// *	nPingAverager
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nPingAverager::nPingAverager( void )
{
    Reset();
}

// *******************************************************************************************
// *
// *	~nPingAverager
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

nPingAverager::~nPingAverager( void )
{
}

// *******************************************************************************************
// *
// *	GetPing
// *
// *******************************************************************************************
//!
//!		@return		our best estimate for the ping
//!
// *******************************************************************************************

REAL nPingAverager::GetPing( void ) const
{
    if(tRecorder::DesyncedPlayback())
        return recordedPing_;

    // collect data
    // determine the lowest guessed value for variance.
    // lag spikes should not contribute here too much.
    REAL variance = 1;
    {
        REAL snailVariance = this->snail_.GetDataVariance();
        REAL slowVariance = this->slow_.GetDataVariance();
        REAL fastVariance = this->fast_.GetDataVariance();
        variance = variance < snailVariance ? variance : snailVariance;
        variance = variance < slowVariance ? variance : slowVariance;
        variance = variance < fastVariance ? variance : fastVariance;
    }

    REAL pingSnail  = this->GetPingSnail();
    REAL pingSlow   = this->GetPingSlow();
    REAL pingFast   = this->GetPingFast();

    // the proposed return value: defaults to the snail ping, it flucuates the least
    REAL pingReturn = pingSnail;

    // return slow average if that differs from the snail one by at least one standard deviation
    if ( ( pingSlow - pingReturn ) * ( pingSlow - pingReturn ) > variance )
    {
        // but clamp it to sane values
        if ( pingSlow > pingReturn * 2 )
            pingSlow = pingReturn * 2;

        pingReturn = pingSlow;
    }

    // same for fast ping
    if ( ( pingFast - pingReturn ) * ( pingFast - pingReturn ) > variance )
    {
        if ( pingFast > pingReturn * 2 )
            pingFast = pingReturn * 2;

        pingReturn = pingFast;
    }

    // return best estimate plus expected variance with fudge factor. It's better to err to the big ping side.
    return pingReturn + sqrtf(variance) * 1.5;
}

// *******************************************************************************************
// *
// *	operator REAL
// *
// *******************************************************************************************
//!
//!		@return		our best estimate for the ping
//!
// *******************************************************************************************

nPingAverager::operator REAL( void ) const
{
    return GetPing();
}

// *******************************************************************************************
// *
// *	GetPingSlow
// *
// *******************************************************************************************
//!
//!		@return		extremely longterm ping average
//!
// *******************************************************************************************

REAL nPingAverager::GetPingSnail( void ) const
{
    if(tRecorder::DesyncedPlayback())
        return recordedPing_;

    return snail_.GetAverage();
}

// *******************************************************************************************
// *
// *	GetPingSlow
// *
// *******************************************************************************************
//!
//!		@return		longterm ping average
//!
// *******************************************************************************************

REAL nPingAverager::GetPingSlow( void ) const
{
    if(tRecorder::DesyncedPlayback())
        return recordedPing_;

    return slow_.GetAverage();
}

// *******************************************************************************************
// *
// *	GetPingFast
// *
// *******************************************************************************************
//!
//!		@return		shortterm ping average
//!
// *******************************************************************************************

REAL nPingAverager::GetPingFast( void ) const
{
    if(tRecorder::DesyncedPlayback())
        return recordedPing_;

    return fast_.GetAverage();
}

// *******************************************************************************************
// *
// *	IsSpiking
// *
// *******************************************************************************************
//!
//!		@return		true if unusual high fluctuations exist in the ping
//!
// *******************************************************************************************

bool nPingAverager::IsSpiking( void ) const
{
    if(tRecorder::DesyncedPlayback())
        return false;

    REAL difference = slow_.GetAverage() - fast_.GetAverage();
    return slow_.GetAverageVariance() < difference * difference;
}

// *******************************************************************************************
// *
// *	Timestep
// *
// *******************************************************************************************
//!
//!		@param	decay	time since last timestep
//!
// *******************************************************************************************

void nPingAverager::Timestep( REAL decay )
{
    if( snail_.GetWeight() > 100 )
        snail_.Timestep( decay * .02 );
    if( slow_.GetWeight() > 30 )
        slow_.Timestep ( decay * .2 );
    if( fast_.GetWeight() > 10 )
        fast_.Timestep ( decay * 2 );
}

// *******************************************************************************************
// *
// *	Add
// *
// *******************************************************************************************
//!
//!		@param	value	the value to add
//!		@param	weight	the value's statistical weight
//!
// *******************************************************************************************

void nPingAverager::Add( REAL value, REAL weight )
{
    // add value to both averagers
    snail_.Add( value, weight );
    slow_.Add ( value, weight );
    fast_.Add ( value, weight );
}

// *******************************************************************************************
// *
// *	Add
// *
// *******************************************************************************************
//!
//!		@param	value	the value to add with default weight
//!
// *******************************************************************************************

void nPingAverager::Add( REAL value )
{
    // add value to both averagers
    Add( value, weight_ );
}

// *******************************************************************************************
// *
// *	Reset
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

void nPingAverager::Reset( void )
{
    snail_.Reset();
    slow_. Reset();
    fast_. Reset();

    // fill in some low weight values
    Add( 1, .000001 );
    Add( 0, .000001 );

    // pin snail averager close to zero
    // snail_.Add(0,10);
    // not such a good idea after all. The above line caused massive resending of packets.
}

// archive ping
void nPingAverager::Record()
{
    recordedPing_ = GetPing();

    static constexpr auto section = "PING";
    tRecorder::Playback(section, recordedPing_);
    tRecorder::Record(section, recordedPing_);
    if(tRecorder::DesyncedPlayback())
    {
        Add(recordedPing_, 1);
    }
}

REAL nPingAverager::weight_=1;





// *******************************************************************************
// *
// *	nMachine
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

nMachine::nMachine( void )
        : lastUsed_(tSysTimeFloat())
        , banned_(-1)
        , players_(0)
        , validated_(false)
        , decorators_(0)
{
    kph_.Add(0,.1666);
    lastPlayerAction_ = lastUsed_;
}

// *******************************************************************************
// *
// *	~nMachine
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

nMachine::~nMachine( void )
{
    // destroy and remove the decorators
    while ( decorators_ )
    {
        nMachineDecorator * decorator = decorators_;
        decorator->Remove();
        decorator->Destroy();
    }
}

// *******************************************************************************
// *
// *	operator ==
// *
// *******************************************************************************
//!
//!		@param	other	the machine to compare with
//!		@return		    true if they are equal
//!
// *******************************************************************************

bool nMachine::operator == ( nMachine const & other ) const
{
    return this == &other;
}

// *******************************************************************************
// *
// *	operator !=
// *
// *******************************************************************************
//!
//!		@param	other	the machine to compare with
//!		@return		    false if they are equal
//!
// *******************************************************************************

bool nMachine::operator !=( nMachine const & other ) const
{
    return this != &other;
}

// singleton machine map
class nMachinePTR
{
public:
    mutable nMachine * machine;
    nMachinePTR(): machine(tNEW(nMachine)()){}
    ~nMachinePTR(){tDESTROY(machine);}
    nMachinePTR(nMachinePTR const & other): machine(other.machine){other.machine=0;}
    nMachinePTR & operator=(nMachinePTR const & other){ machine = other.machine; other.machine=0;return *this;}
};

typedef sockaddr nMachineKey;

bool operator < ( nMachineKey const & a, nMachineKey const & b )
{
    sockaddr_in const & sa = reinterpret_cast< sockaddr_in const & >( a );
    sockaddr_in const & sb = reinterpret_cast< sockaddr_in const & >( b );
#ifdef DEBUG_X
// compare ports first to make different clients appear as different voters
    if(sa.sin_port != sb.sin_port)
        return sa.sin_port < sb.sin_port;
#endif

    return sa.sin_addr.s_addr < sb.sin_addr.s_addr;
}

typedef std::map< nMachineKey, nMachinePTR > nMachineMap;
static nMachineMap & sn_GetMachineMap()
{
    static nMachineMap map;
    return map;
}

static nMachine & sn_LookupMachine( nMachineKey const * address )
{
    // get map of all machines and look address up
    nMachineMap & map = sn_GetMachineMap();
    nMachine & ret = *map[ *address ].machine;
    if( ret.GetIP().Len() <= 2 )
    {
        nAddress addr;
        sockaddr * target = addr;
        *target = *address;
        ret.SetIP( addr.GetAddress() );
    }
    return ret;
}

static nMachine * sn_PeekMachine( nMachineKey const * address )
{
    // get map of all machines and look address up
    nMachineMap & map = sn_GetMachineMap();
    nMachineMap::const_iterator i = map.find( *address );
    if( i != map.end() )
    {
        return (*i).second.machine;
    }
    else
    {
        return NULL;
    }
}

static nMachine & sn_LookupMachine( tString const & address )
{
    nAddress addr;
    addr.SetAddress( address );
    return sn_LookupMachine( addr );
}

class nMachineIteratorPimpl: public nMachineMap::iterator
{
public:
    nMachineIteratorPimpl()
    : nMachineMap::iterator(sn_GetMachineMap().begin())
    {
    }
};

nMachine & nMachine::iterator::operator *() const
{
    nMachineMap::iterator & i = *pimpl_;
    nMachinePTR & ptr = (*i).second;
    return *ptr.machine;
}

nMachine::iterator::iterator()
{
    pimpl_ = new nMachineIteratorPimpl();
}

nMachine::iterator::~iterator()
{
    delete pimpl_;
}

void nMachine::iterator::operator ++()
{
    (*pimpl_)++;
}
void nMachine::iterator::operator ++(int)
{
    (*pimpl_)++;
}

bool nMachine::iterator::Valid()
{
    return (*pimpl_) != sn_GetMachineMap().end();
}

// *******************************************************************************
// *
// *	GetMachine
// *
// *******************************************************************************
//!
//!		@param	userID	the user ID to fetch the machine for
//!		@return		    the machine the user ID belongs to
//!
// *******************************************************************************

nMachine & nMachine::GetMachine( unsigned short userID )
{
    // throw out old machines
    Expire();

    // hardcoding: the server itself
    if ( userID == 0 && sn_GetNetState() != nCLIENT )
    {
        static nMachine server;
        return server;
    }

    if( sn_GetNetState() != nSERVER )
    {
        // tASSERT(userID == 0);

        // invalid ID, return invalid machine (clients don't track machines)
        static nMachine invalid;
        return invalid;
    }

    tASSERT( userID <= MAXCLIENTS+1 );

    // get address
    tVERIFY( userID <= MAXCLIENTS+1 );
    if( !sn_Connections[userID].socket )
    {
        // invalid ID, return invalid machine
        static nMachine invalid;
        return invalid;
    }

    // delegate
    return sn_LookupMachine( peers[userID] );
}

// *******************************************************************************
// *
// *	PeekMachine
// *
// *******************************************************************************
//!
//!		@param	userID	the user ID to fetch the machine for
//!		@return		    the machine the user ID belongs to
//!
// *******************************************************************************

nMachine * nMachine::PeekMachine( unsigned short userID )
{
    // hardcoding: the server itself
    if ( userID == 0 && sn_GetNetState() != nCLIENT )
    {
        return &GetMachine( userID );
    }

    tASSERT( userID <= MAXCLIENTS+1 );

    if( sn_GetNetState() != nSERVER )
    {
        // invalid ID, return invalid machine (clients don't track machines)
        return &GetMachine( userID );
    }

    // get address
    tVERIFY( userID <= MAXCLIENTS+1 );
    if( !sn_Connections[userID].socket )
    {
        // invalid ID, return invalid machine
        return &GetMachine( userID );
    }

    // delegate
    return sn_PeekMachine( peers[userID] );
}

// safely delete iterator from map
static void sn_Erase( nMachineMap & map, nMachineMap::iterator & iter )
{
    if ( iter != map.end() )
    {
        map.erase( iter );
        iter = map.end();
    }
}

// *******************************************************************************
// *
// *	Expire
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

void nMachine::Expire( void )
{
    static double lastTime = tSysTimeFloat();
    double time = tSysTimeFloat();
    REAL dt = time - lastTime;
    if (dt <= 60)
        return;
    lastTime = time;

    // iterate over known machines
    nMachineMap & map = sn_GetMachineMap();
    nMachineMap::iterator toErase = map.end();
    for( nMachineMap::iterator iter = map.begin(); iter != map.end(); ++iter )
    {
        // erase last deleted machine
        sn_Erase( map, toErase );

        nMachine & machine = *(*iter).second.machine;

        // advance the kick statistics if the user is not banned and has been active
        if ( time > machine.banned_ && ( machine.lastUsed_ > time - 300 || machine.players_ > 0 ) )
        {
            machine.kph_.Add( 0, dt / 3600 );
            machine.kph_.Timestep( dt / 3600*24 );
        }

        // if the machine is no longer in use, mark it for deletion
        if ( machine.players_ == 0 && machine.lastUsed_ < time - 300.0 && machine.banned_ < time && machine.kph_.GetAverage() < 0.5 )
            toErase = iter;

    }

    // erase last machine
    sn_Erase( map, toErase );
}

// maximal time a client without players is tolerated
static REAL sn_spectatorTime = 0;
static tSettingItem< REAL > sn_spectatorTimeConf( "NETWORK_SPECTATOR_TIME", sn_spectatorTime );

// *******************************************************************************
// *
// *	KickSpectators
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

void nMachine::KickSpectators( void )
{
    double time = tSysTimeFloat();

    // kick spectators
    if ( sn_GetNetState() == nSERVER && sn_spectatorTime > 0 )
    {
        for ( int i = MAXCLIENTS; i >= 1; --i )
        {
            if ( sn_Connections[i].socket )
            {
                nMachine & machine = GetMachine( i );
                if ( machine.players_ == 0 && machine.lastPlayerAction_ + sn_spectatorTime < time )
                {
                    sn_KickUser( i, tOutput("$network_kill_spectator"), 0 );
                }
            }
        }
    }
}

// settings for automatic banning
static REAL sn_autobanOffset = 5;  // bias that gets subtracted from the kills per hour
static REAL sn_autobanFactor = 10; // factor that gets multiplied on top of it to determine the ban time in minutes
static REAL sn_autobanMaxKPH = 30; // maximal value of kph

static tSettingItem< REAL > sn_autobanOffsetSetting( "NETWORK_AUTOBAN_OFFSET", sn_autobanOffset );
static tSettingItem< REAL > sn_autobanFactorSetting( "NETWORK_AUTOBAN_FACTOR", sn_autobanFactor );
static tSettingItem< REAL > sn_autobanMaxKPHSetting( "NETWORK_AUTOBAN_MAX_KPH", sn_autobanMaxKPH );

// *******************************************************************************
// *
// *	OnKick
// *
// *******************************************************************************
//!
//! @param severity the severity of the offense; 1 is standard.
//!
// *******************************************************************************

void nMachine::OnKick( REAL severity )
{
    // if the user is currently banned, don't count
    if ( banned_ > tSysTimeFloat() )
        return;

    // ban the user a bit, taking the kicks per hour into account
    REAL kph = kph_.GetAverage() - sn_autobanOffset;
    if ( kph > 0 )
    {
        // the faster you get kicked when you turn up, the longer you get banned
        REAL banTime = 60 * kph * sn_autobanFactor;
        Ban( banTime, tString(tOutput( "$network_ban_kick" )) );
    }

    // add it to the statistics
    if ( sn_autobanMaxKPH > 0 )
        kph_.Add( severity * sn_autobanMaxKPH, 2/sn_autobanMaxKPH );

    con << tOutput( "$network_ban_kph", GetIP(), GetKicksPerHour() );
}

static bool sn_printBans = true;

// *******************************************************************************
// *
// *	Ban
// *
// *******************************************************************************
//!
//!		@param	time	time in seconds the ban should be in effect
//!
// *******************************************************************************

void nMachine::Ban( REAL time )
{
    lastUsed_ = tSysTimeFloat();

    // set the banning timeout to the current time plus the given time
    banned_ = tSysTimeFloat() + time;

    // kick current clients
    if( time > 0 )
    {
        for( int i = MAXCLIENTS-1; i > 0; --i )
        {
            if ( sn_Connections[i].socket && &GetMachine(i) == this )
            {
                sn_DisconnectUser( i, banReason_ );
            }
        }

        for ( nMachineDecorator *decorator = decorators_; decorator != NULL; decorator = decorator->Next() )
        {
            decorator->OnBan();
        }
    }

    if ( sn_printBans )
    {
        if ( time > 0 )
            con << tOutput( "$network_ban", GetIP(), int(time/60), banReason_.Len() > 1 ? banReason_ : tOutput( "$network_ban_noreason" ) );
        else
            con << tOutput( "$network_noban", GetIP() );
    }
}

// *******************************************************************************
// *
// *	Ban
// *
// *******************************************************************************
//!
//!		@param	time	time in seconds the ban should be in effect
//!		@param	reason	the reason for the ban
//!
// *******************************************************************************

void nMachine::Ban( REAL time, tString const & reason )
{
    banReason_ = tString();
    if ( reason.Len() > 2 )
        banReason_ = reason;

    Ban( time );
}

// *******************************************************************************
// *
// *	IsBanned
// *
// *******************************************************************************
//!
//!		@return		kick time left
//!
// *******************************************************************************

REAL nMachine::IsBanned( void ) const
{
    // test for banning
    double time = tSysTimeFloat();
    if ( time > banned_ )
        return 0;

    return banned_ - time;
}

// *******************************************************************************
// *
// *	AddPlayer
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

void nMachine::AddPlayer( void )
{
    lastPlayerAction_ = lastUsed_ = tSysTimeFloat();

    players_++;
}

// *******************************************************************************
// *
// *	RemovePlayer
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

void nMachine::RemovePlayer( void )
{
    lastPlayerAction_ = lastUsed_ = tSysTimeFloat();

    players_--;
    if ( players_ < 0 )
        players_ = 0;
}

// *******************************************************************************
// *
// *	GetPlayerCount
// *
// *******************************************************************************
//!
//!		@return		the number of currently connected players
//!
// *******************************************************************************

int nMachine::GetPlayerCount( void )
{
    return players_;
}


static char const * sn_machinesFileName = "bans.txt";

class nMachinePersistor
{
public:
    // save ban info of machines
    static void SaveMachines()
    {
        std::ofstream s;
        if (tDirectories::Var().Open( s, sn_machinesFileName ) )
        {
            nMachineMap & map = sn_GetMachineMap();
            for( nMachineMap::iterator iter = map.begin(); iter != map.end(); ++iter )
            {
                nMachine & machine = *(*iter).second.machine;
                // if ( machine.IsBanned() > 0 )
                {
                    s << machine.GetIP() << " " << machine.IsBanned() << " " << machine.kph_ << " " << machine.GetBanReason() << "\n";
                }
            }
        }
    }

    // load and enter ban info of machines
    static void LoadMachines()
    {
        sn_printBans = false;

        tTextFileRecorder machines( tDirectories::Var(), sn_machinesFileName );
        while ( !machines.EndOfFile() )
        {
            std::stringstream line( machines.GetLine() );

            // address and ban time left
            tString address;
            REAL banTime;

            // read relevant info
            line >> address >> banTime;
            std::ws(line);

            // read kph averager
            nAverager kph;
            char c;
            line.get(c);
            line.putback(c);
            if ( c == '(' )
            {
                line >> kph;
                std::ws(line);
            }

            // read reason
            tString reason;
            reason.ReadLine( line );

            if ( address.Len() > 2 )
            {
                // ban
                nMachine & machine = sn_LookupMachine( address );
                machine.Ban( banTime, reason );
                machine.kph_ = kph;
            }
        }

        sn_printBans = true;
    }
}
;
// save ban info of machines
static void sn_SaveMachines()
{
    nMachinePersistor::SaveMachines();
}

// load and enter ban info of machines
static void sn_LoadMachines()
{
    nMachinePersistor::LoadMachines();
}

// *******************************************************************************
// *
// *	GetKicksPerHour
// *
// *******************************************************************************
//!
//!		@return		averaged kicks per hour of players from this machine
//!
// *******************************************************************************

REAL nMachine::GetKicksPerHour( void ) const
{
    return this->kph_.GetAverage();
}

// *******************************************************************************
// *
// *	GetKicksPerHour
// *
// *******************************************************************************
//!
//!		@param	kph	averaged kicks per hour of players from this machine to fill
//!		@return		A reference to this to allow chaining
//!
// *******************************************************************************

nMachine const & nMachine::GetKicksPerHour( REAL & kph ) const
{
    kph = this->kph_.GetAverage();
    return *this;
}

// *******************************************************************************
// *
// *	GetIP
// *
// *******************************************************************************
//!
//!		@return		IP address of the machine
//!
// *******************************************************************************

tString const & nMachine::GetIP( void ) const
{
    return this->IP_;
}

// *******************************************************************************
// *
// *	GetIP
// *
// *******************************************************************************
//!
//!		@param	IP	IP address of the machine to fill
//!		@return		A reference to this to allow chaining
//!
// *******************************************************************************

nMachine const & nMachine::GetIP( tString & IP ) const
{
    IP = this->IP_;
    return *this;
}

// *******************************************************************************
// *
// *	SetIP
// *
// *******************************************************************************
//!
//!		@param	IP	IP address of the machine to set
//!		@return		A reference to this to allow chaining
//!
// *******************************************************************************

nMachine & nMachine::SetIP( tString const & IP )
{
    lastUsed_ = tSysTimeFloat();

    this->IP_ = IP;
    return *this;
}

// *******************************************************************************
// *
// *	GetBanReason
// *
// *******************************************************************************
//!
//!		@return		Reason of the ban
//!
// *******************************************************************************

tString const & nMachine::GetBanReason( void ) const
{
    return this->banReason_;
}

// *******************************************************************************
// *
// *	GetBanReason
// *
// *******************************************************************************
//!
//!		@param	reason	Reason of the ban to fill
//!		@return		A reference to this to allow chaining
//!
// *******************************************************************************

nMachine const & nMachine::GetBanReason( tString & reason ) const
{
    reason = this->banReason_;
    return *this;
}

// *******************************************************************************
// *
// *	Banning and unbanning
// *
// *******************************************************************************

// unban IPs
static void sn_UnBanConf(std::istream &s)
{
    if ( !s.good() || s.eof() )
    {
        con << "Usage: UNBAN_IP <ip>\n";
        return;
    }

    // read IP to unban
    tString address;
    s >> address;

    if ( address.Len() < 8 )
    {
        con << "Usage: UNBAN_IP <ip>, no or too short ip given.\n";
    }
    // and unban
    else
    {
        sn_LookupMachine( address ).Ban( 0 );
    }
}

static tConfItemFunc sn_unBanConf("UNBAN_IP",&sn_UnBanConf);

// ban IPs
static void sn_BanConf(std::istream &s)
{
    // read IP to unban
    tString address;
    s >> address;

    if ( !s.good() && address.Len() < 7 )
    {
        con << "Usage: BAN_IP <ip> <time in minutes (defaults to 60)> <reason>\n";
        return;
    }

    REAL duration = 60;
    s >> duration;

    // read reason
    tString reason;
    std::ws(s);
    if ( s.good() )
    {
        reason.ReadLine(s);
    }

    // and ban
    if ( address.Len() > 4 )
    {
        sn_LookupMachine( address ).Ban( duration * 60, reason );
    }
}

static tConfItemFunc sn_banConf("BAN_IP",&sn_BanConf);

// list bans
static void sn_ListBanConf(std::istream &s)
{
    nMachineMap & map = sn_GetMachineMap();
    for( nMachineMap::iterator iter = map.begin(); iter != map.end(); ++iter )
    {
        nMachine & machine = *(*iter).second.machine;
        REAL banned = machine.IsBanned();
        if ( banned > 0 )
        {
            con << tOutput( "$network_ban", machine.GetIP(), int(banned/60), machine.GetBanReason() );
        }
    }
}

static tConfItemFunc sn_listBanConf("BAN_LIST",&sn_ListBanConf);

// *******************************************************************************
// *
// *	OnDestroy
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

void nMachineDecorator::OnDestroy( void )
{
}

void nMachineDecorator::OnBan()
{
}

// *******************************************************************************
// *
// *	nMachineDecorator
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

nMachineDecorator::nMachineDecorator( void )
{
}

// *******************************************************************************
// *
// *	~nMachineDecorator
// *
// *******************************************************************************
//!
//!
// *******************************************************************************

nMachineDecorator::~nMachineDecorator( void )
{
    Remove();
}

// *******************************************************************************
// *
// *	nMachineDecorator
// *
// *******************************************************************************
//!
//!		@param	machine
//!
// *******************************************************************************

nMachineDecorator::nMachineDecorator( nMachine & machine )
{
    Insert( machine.decorators_ );
}


std::vector<unsigned short> nMessage::nMessageToDataVector(nMessage& msg) {
    std::vector<unsigned short> data;
    for (int i = 0; i < msg.DataLen(); i++) {
        data.push_back(msg.Data(i));
    }
    return data;
}

void sn_quitAction(bool save, bool quit, std::string message)
{
    sn_quitAction(save, quit, tString(message));
}

#include "../tron/gGame.h"
void sn_quitAction(bool save, bool quit, tString message)
{
    gHelperUtility::Debug("sn_quitAction", "Action initiated. Save:" + std::to_string(save) + " Quit:" + std::to_string(quit) + " Reason:", message);

    FileManager(tString("banned.txt"), tDirectories::Var()).Write(message);

    if (se_playerStats)
    {
        eChatBotStats &stats = eChatBot::getInstance().Stats();

        time_t startOfDay = getStartTimeOfDay();

        if (stats.last_banned < startOfDay)
            stats.times_banned_today = 0;

        stats.last_banned = time(nullptr);
        stats.times_banned++;
        stats.times_banned_today++;

        if (se_playerWatchAutoRandomName)
        {
            gHelperUtility::Debug("sn_quitAction", "Forcing random name.");
            forceRandomRename = true;
        }
    }

    if (se_playerMessageTriggers && se_playerMessageTriggersBannedAction) 
        eChatBot::findResponse(eChatBot::getInstance(), tString(""), tString("$bannedaction"), message, true);

    if (save)
        st_SaveConfig();

    if (quit)
    {
        gHelperUtility::Debug("sn_quitAction", "Scheduling quit for " + std::to_string(sn_bannedWatchQuitTime) + " seconds.");
        gTaskScheduler.schedule("bannedWatchQuit", sn_bannedWatchQuitTime, []
                                {
                                  uMenu::quickexit = uMenu::QuickExit_Total;
                                });
    }
}

void sn_bannedWatchAction(tString reason)
{
    sn_quitAction(true, sn_bannedWatchQuit, reason);
}

static void sn_bannedCMD(std::istream &s)
{
    tString reason;
    reason.ReadLine(s, true);
    sn_bannedWatchAction(reason);
}
static tConfItemFunc sn_bannedCMDConf("BANNED", &sn_bannedCMD);
