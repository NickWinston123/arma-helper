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

Although it is already included in the GPL, let me clarify:
anybody, especially the Krawall Gaming network, is allowed
to run a server/master server build from modified versions of
this file WHITHOUT releasing the source to the public (provided
the executable is not distributed).


*/

#include "nKrawall.h"
#include "tString.h"
#include "tConsole.h"
#include "nNetwork.h"
#include "tConfiguration.h"
#include "tArray.h"

#include <algorithm>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <iterator>

static nKrawall::nMethod sn_bmd5("bmd5"), sn_md5("md5");

static tSettingItem< tString > sn_md5Prefix( "MD5_PREFIX", sn_md5.prefix );
static tSettingItem< tString > sn_md5Suffix( "MD5_SUFFIX", sn_md5.suffix );

//! fetch NULL-terminated list of locally supported methods
nKrawall::nMethod const * const * nKrawall::nMethod::LocalMethods()
{
    static nMethod const * methods[] =
    {
        &sn_md5,
        &sn_bmd5,
        NULL
    };

    return methods;
}

#ifdef KRAWALL_SERVER

#include <libxml/nanohttp.h>

// crude login structure with not too many feathres
struct nLogin
{
    tString authority_;

    // scrambled passwords according to our various methods
    std::map< tString, nKrawall::nScrambledPassword > scrambledPasswords_;

    // access level for the login
    tAccessLevel accessLevel_;

    // password for team accounts
    tString password_;

    static bool FixOK( tString const & fix )
    {
        return fix.StrPos( "%" ) < 0;
    }

    nLogin( char const * authority, tString const & password, tString const & username, tAccessLevel accessLevel )
    : authority_( authority ), accessLevel_( accessLevel )
    {
        // scramble the password before storing it (in case an evil attacker can read our memory,
        // but not the configuration file where they are stored in plain text :) )
        nKrawall::nMethod const * const * run = nKrawall::nMethod::LocalMethods();
        while ( * run )
        {
            if ( username.Len() <= 1 &&
                 ( !FixOK( (*run)->prefix ) || !FixOK( (*run)->suffix ) )
                )
            {
                // store plain text password
                password_ = password;
            }
            else
            {
                (*run)->ScramblePassword( nKrawall::nScrambleInfo( username ), password, scrambledPasswords_[ (*run)->method ] );
            }

            ++run;
        }
    }

    // check whether a scrambled password sent by a client is correct
    bool CheckPassword( nKrawall::nScrambleInfo const & info, nKrawall::nMethod const & method, nKrawall::nSalt const & salt, nKrawall::nScrambledPassword const & hash, tString & error ) const
    {
        // find the correct scrambled password
        std::map< tString, nKrawall::nScrambledPassword >::const_iterator scrambled = scrambledPasswords_.find( method.method );
        if ( scrambled != scrambledPasswords_.end() )
        {
            // check whether methods match exactly
            nKrawall::nMethod const * const * run = nKrawall::nMethod::LocalMethods();
            while ( * run )
            {
                nKrawall::nMethod const & compare = **run;
                if ( compare.method == method.method && !nKrawall::nMethod::Equal(compare, method ) )
                {
                    error = tOutput( "$login_error_methodmismatch" );
                    return false;
                }

                ++run;
            }

            // compare passwords
            nKrawall::nScrambledPassword scrambledCorrect;
            method.ScrambleWithSalt( info, (*scrambled).second, salt, scrambledCorrect );
            bool ret = nKrawall::ArePasswordsEqual( hash, scrambledCorrect );
            if ( !ret )
            {
                error = tOutput( "$login_error_local_password", info.username );
            }
            return ret;
        }
        else
        {
            if ( password_ == "" )
            {
                error = "Internal error, local method not found, and no plaintext password stored.";
                return false;
            }

            // do a full check on the stored password
            nKrawall::nScrambledPassword scrambled, correctHash;
            method.ScramblePassword( info, password_, scrambled );
            method.ScrambleWithSalt( info, scrambled, salt, correctHash );

            bool ret = nKrawall::ArePasswordsEqual( hash, correctHash );
            if ( !ret )
            {
                error = tOutput( "$login_error_local_password", info.username );
            }
            return ret;
        }

        return false;
    }

    nLogin()
    {
    }
};

typedef std::map< tString, nLogin > nLoginMap;

// database of exact logins
static nLoginMap sn_exactLogins;

// database of inprecise logins
static nLoginMap sn_partialLogins;

// add an admin account
static void sn_ReadPassword( std::istream & s )
{
    tString username, password;
    s >> username;
    if ( !s.good() )
    {
        con << tOutput( "$local_user_syntax" );
        return;
    }
    tConfItemBase::EatWhitespace(s);
    password.ReadLine(s);
    if ( password == "" )
    {
        con << tOutput( "$local_user_syntax" );
        return;
    }

    sn_exactLogins[ username ] = nLogin( "", password, username, tAccessLevel_Local );
}

static tConfItemFunc sn_kpa( "LOCAL_USER", sn_ReadPassword );
static tAccessLevelSetter sn_kpal( sn_kpa, tAccessLevel_Owner );

// add team account
static void sn_ReadTeamPassword( std::istream & s )
{
    tString username, password;
    s >> username;
    if ( !s.good() )
    {
        con << tOutput( "$local_team_syntax" );
        return;
    }
    tConfItemBase::EatWhitespace(s);
    password.ReadLine(s);
    if ( password == "" )
    {
        con << tOutput( "$local_team_syntax" );
        return;
    }

    sn_partialLogins[ username ] = nLogin( tString("L_TEAM_") + username, password, tString(""), tAccessLevel_TeamMember );
}

static tConfItemFunc sn_kta( "LOCAL_TEAM", sn_ReadTeamPassword );
static tAccessLevelSetter sn_ktal( sn_kta, tAccessLevel_Owner );

// finds a login element as iterator
nLoginMap::iterator sn_FindLoginIterator( tString const & username, nLoginMap * & map, bool exact = false )
{
    // find exact login
    map = &sn_exactLogins;
    nLoginMap::iterator found = sn_exactLogins.find( username );
    if ( found != sn_exactLogins.end() )
    {
        return found;
    }

    map = &sn_partialLogins;
    for( int i = username.Len(); i >= 1; --i )
    {
        tString partial = username.SubStr( 0, i );
        nLoginMap::iterator found = map->find( partial );
        if ( found != map->end() )
        {
            return found;
        }

        if ( exact )
        {
            return map->end();
        }
    }

    return map->end();
}

// finds login pointer
nLogin const * sn_FindLogin( tString const & username )
{
    // find login
    nLoginMap * map;
    nLoginMap::iterator found = sn_FindLoginIterator( username, map );
    if ( found != map->end() )
    {
        return &found->second;
    }

    return 0;
}

// remove an account
static void sn_ReadPasswordRemove( std::istream & s )
{
    tString username;
    s >> username;
    nLoginMap * map;
    nLoginMap::iterator found = sn_FindLoginIterator( username, map, true );
    if ( found != map->end() )
    {
        map->erase( found );
        con << tOutput( "$md5_password_removed", username );
    }
    else
    {
        con << tOutput( "$md5_password_remove_notfound", username );
    }
}

static tConfItemFunc sn_kpr( "USER_REMOVE", sn_ReadPasswordRemove );


// fetch the scrambled password of username from the users database
void nKrawall::CheckScrambledPassword( nCheckResultBase & result,
                                       nPasswordCheckData const & data )
{
    // extra salt scrambling process
    nSalt salt = data.salt;
    data.method.ScrambleSalt( salt, data.serverAddress );

    // local users
    if ( result.authority.Len() <= 1 )
    {
        // fetch login data
        nLogin const * login = sn_FindLogin( result.username );
        if ( !login )
        {
            result.success = false;
            result.error = tOutput( "$login_error_local_nouser", result.username );
            return;
        }

        // store relevant authority
        result.authority = login->authority_;

        // check password
        result.success = login->CheckPassword( nScrambleInfo( result.username ), data.method, salt, data.hash, result.error );
        result.accessLevel = login->accessLevel_;

        return;
    }
    else
    {
        // remote users: build query URL
        std::ostringstream request;

        request << "?query=check";
        request << "&method=" << EncodeString( data.method.method );
        request << "&user="   << EncodeString( result.username );
        request << "&salt="   << EncodeScrambledPassword( salt );
        request << "&hash="   << EncodeScrambledPassword( data.hash );

        /*tString msg;
        msg << "password " << "" << "\n";
        sn_ConsoleOut(msg);*/

        // read URL content
        std::stringstream content;
        int rc = FetchURL( data.fullAuthority, request.str().c_str(), content );

        if (rc == -1)
        {
            result.error = tOutput( "$login_error_invalidurl_notfound", result.authority );
            result.success = false;
            return;
        }

        // lots of string copying going on, probably should find a better way.
        char * buf_temp = strdup( content.str().c_str() );
        unsigned int len = strlen(buf_temp);

        // get rid of newlines
        for ( unsigned int i = 0; i < len; ++i )
        {
            if ( buf_temp[i] == '\n' )
            {
                buf_temp[i] = ' ';
            }
        }

        // trailing spaces are ugly
        while ( len > 0 && buf_temp[len-1] == ' ' )
        {
            buf_temp[len-1] = 0;
            --len;
        }

        tString buf( buf_temp );
        free( buf_temp );

        // catch various error codes
        if ( rc != 200 ) {
            result.success = false;
            switch ( rc )
            {
            case 404:
                result.error = tOutput( "$login_error_nouser", buf );
                break;
            case 403:
            case 401:
                result.error = tOutput( "$login_error_password", buf );
                break;
            default:
                result.error = tOutput( "$login_error_unknown", rc, buf );
                break;
            }

            return;
        }

        // read the buffer
        tString ret;
        content >> ret;
        tToLower( ret );

        // catch the same errros frm the server's response
        if ( ret == "unknown_user" )
        {
            result.error = tOutput( "$login_error_nouser", buf );
            return;
        }

        if ( ret == "password_fail" )
        {
            result.error = tOutput( "$login_error_password", buf );
            return;
        }

        if ( ret != "password_ok" )
        {
            result.error << tOutput( "$login_error_unexpected_answer", "PASSWORD_OK ...", buf );
            return;
        }

        // everything fine so far. Read the full username returned from the authority.
        tString fullUserName;
        fullUserName.ReadLine( content );

        // check on it
        if ( fullUserName != "" )
        {
            tString claimedAuthority;
            SplitUserName( fullUserName, result.username, claimedAuthority );
            if ( !CanClaim ( result.authority, claimedAuthority ) )
            {
                result.error << tOutput( "$login_error_invalidclaim",
                                         result.authority,
                                         result.username + "@" + claimedAuthority );
                return;
            }
            else
            {
                result.authority = claimedAuthority;
            }
        }

        // read additional data, let caller handle it
        while( true )
        {
            tString blurb;
            blurb.ReadLine( content );
            if ( content.eof() || content.fail() )
            {
                break;
            }
            result.blurb.push_back( blurb );
        }

        result.accessLevel = tAccessLevel_Remote;
        result.success = true;
        return;
    }
}

//!<  Shorthand code BEGIN

//  Custom Shorthand List: Keeps a list of shorthands to connect to
class CustomShorthandClass
{
    public:
        tString shorthand;
        bool enabled = true;
        std::map<int, tString> links;

        CustomShorthandClass() {}
        CustomShorthandClass(tString s, tString link)
        {
            shorthand = s;
            enabled = true;
            links.insert({links.size(), link});
        }
};
static tList<CustomShorthandClass> sn_CustomShorthandList;

static void sn_CustomShorthandAdd(std::istream &s)
{
    int pos = 0;
    tString params;
    params.ReadLine(s);

    tString shorthand = params.ExtractNonBlankSubString(pos);
    tString link      = params.ExtractNonBlankSubString(pos);

    if (shorthand.Filter() != "")
    {
        if (link.Filter() != "")
        {
            bool found = false;
            CustomShorthandClass *sData;

            if (sn_CustomShorthandList.Len() > 0)
            {
                for(int i = 0; i < sn_CustomShorthandList.Len(); i++)
                {
                    CustomShorthandClass *sShort = sn_CustomShorthandList[i];
                    if (sShort->shorthand == shorthand.Filter())
                    {
                        found = true;
                        sData = sShort;
                        break;
                    }
                }
            }

            if(found)
            {
                bool sLinkFound = false;
                for(int i = 0; i < sData->links.size(); i++)
                {
                    tString sLink = sData->links.at(i);
                    if (sLink == link.Filter())
                    {
                        sLinkFound = true;
                        break;
                    }
                }

                if (!sLinkFound)
                {
                    sData->links.insert({sData->links.size(), link});
                    sn_ConsoleOut(tOutput("$custom_shorthand_link_added", sData->shorthand, link.Filter()), 0);
                }
                else sn_ConsoleOut(tOutput("$custom_shorthand_link_exists", sData->shorthand), 0);
            }
            else
            {
               sn_CustomShorthandList.Insert(new CustomShorthandClass(shorthand.Filter(), link.Filter()));
               sn_ConsoleOut(tOutput("$custom_shorthand_added", shorthand, link.Filter()), 0);
            }
        }
    }
}
static tConfItemFunc sn_CustomShorthandAddConf("CUSTOM_SHORTHAND_ADD", &sn_CustomShorthandAdd);
static tAccessLevelSetter sn_CustomShorthandAddConfLevel( sn_CustomShorthandAddConf, tAccessLevel_Moderator );

static void sn_CustomShorthandRemove(std::istream &s)
{
    int pos = 0;
    tString params;
    params.ReadLine(s);

    tString shorthand = params.ExtractNonBlankSubString(pos);

    if (shorthand.Filter() != "")
    {
        bool found = false;
        int foundKey = 0;
        for(int i = 0; i < sn_CustomShorthandList.Len(); i++)
        {
            CustomShorthandClass *sShort = sn_CustomShorthandList[i];
            if (sShort->shorthand == shorthand.Filter())
            {
                found = true;
                foundKey = i;
                break;
            }
        }

        if(found)
        {
            sn_CustomShorthandList.RemoveAt(foundKey);
            sn_ConsoleOut(tOutput("$custom_shorthand_removed", shorthand.Filter()), 0);
        }
        else sn_ConsoleOut(tOutput("$custom_shorthand_does_not_exist", shorthand.Filter()), 0);
    }
}
static tConfItemFunc sn_CustomShorthandRemoveConf("CUSTOM_SHORTHAND_REMOVE", &sn_CustomShorthandRemove);
static tAccessLevelSetter sn_CustomShorthandRemoveConfLevel( sn_CustomShorthandRemoveConf, tAccessLevel_Moderator );

static void sn_CustomShorthandAllowed(std::istream &s)
{
    int pos = 0;
    tString params;
    params.ReadLine(s);

    tString shorthand  = params.ExtractNonBlankSubString(pos);
    tString booleanStr = params.ExtractNonBlankSubString(pos);

    bool is_enabled;
    if (booleanStr.toInt() >= 1)
        is_enabled = true;
    else
        is_enabled = false;

    if (shorthand.Filter() != "")
    {
        bool found = false;
        CustomShorthandClass *sData;
        for(int i = 0; i < sn_CustomShorthandList.Len(); i++)
        {
            sData = sn_CustomShorthandList[i];
            if (sData->shorthand == shorthand.Filter())
            {
                found = true;
                break;
            }
        }

        if(found)
        {
            if (sData->enabled != is_enabled)
            {
                sData->enabled = is_enabled;

                if (sData->enabled)
                    sn_ConsoleOut(tOutput("$custom_shorthand_allowed", sData->shorthand), 0);
                else
                    sn_ConsoleOut(tOutput("$custom_shorthand_notallowed", sData->shorthand), 0);
            }
            else sn_ConsoleOut(tOutput("$custom_shorthand_allowed_nodiff", sData->shorthand), 0);
        }
        else sn_ConsoleOut(tOutput("$custom_shorthand_does_not_exist", shorthand.Filter()), 0);
    }
}
static tConfItemFunc sn_CustomShorthandAllowedConf("CUSTOM_SHORTHAND_ALLOWED", &sn_CustomShorthandAllowed);
static tAccessLevelSetter sn_CustomShorthandAllowedConfLevel( sn_CustomShorthandAllowedConf, tAccessLevel_Moderator );

static void sn_CustomShorthandViewList(std::istream &s)
{
    tOutput out;
    out << "Custom Shorthand (" << sn_CustomShorthandList.Len() << "): ";
    if (sn_CustomShorthandList.Len() > 0)
    {
        for(int i = 0; i < sn_CustomShorthandList.Len(); i++)
        {
            out << sn_CustomShorthandList[i]->shorthand << "|";
            if (sn_CustomShorthandList[i]->enabled)
                out << "T";
            else out << "F";
            out << ", ";
        }
    }
    out << "\n";
    sn_ConsoleOut(out, 0);
}
static tConfItemFunc sn_CustomShorthandViewListConf("CUSTOM_SHORTHAND_LIST", &sn_CustomShorthandViewList);
static tAccessLevelSetter sn_CustomShorthandViewListConfLevel( sn_CustomShorthandViewListConf, tAccessLevel_Moderator );

static void sn_CustomShorthandLinksList(std::istream &s)
{
    int pos = 0;
    tString params;
    params.ReadLine(s);

    tString shorthand = params.ExtractNonBlankSubString(pos);

    if (shorthand.Filter() != "")
    {
        bool found = false;
        CustomShorthandClass *sData;
        for(int i = 0; i < sn_CustomShorthandList.Len(); i++)
        {
            sData = sn_CustomShorthandList[i];
            if (sData->shorthand == shorthand.Filter())
            {
                found = true;
                break;
            }
        }

        if(found)
        {
            tString data;
            for(int j = 0; j < sData->links.size(); j++)
                data << sData->links.at(j) << ", ";

            con <<  "Custom Shorthand '" << sData->shorthand << "': " << data << "\n";
        }
        else sn_ConsoleOut(tOutput("$custom_shorthand_does_not_exist", shorthand.Filter()), 0);
    }
}
static tConfItemFunc sn_CustomShorthandLinksListConf("CUSTOM_SHORTHAND_LINKS_LIST", &sn_CustomShorthandLinksList);
static tAccessLevelSetter sn_CustomShorthandLinksListConfLevel( sn_CustomShorthandLinksListConf, tAccessLevel_Moderator );

bool sn_CustomShorthandEnabled = false;
static tSettingItem<bool> sn_CustomShorthandEnabledConf("CUSTOM_SHORTHAND_ENABLED", sn_CustomShorthandEnabled);

int nKrawall::CustomShorthandCheck(int userID, tString authority, tString &fullAuthority, tOutput & error)
{
    if (!sn_CustomShorthandEnabled) return 0;

    //  check if calling custom shorthand exists
    bool found = false;
    CustomShorthandClass *sData;
    for(int i = 0; i < sn_CustomShorthandList.Len(); i++)
    {
        sData = sn_CustomShorthandList[i];
        if (sData->shorthand == authority.Filter())
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        int response = -1;
        std::stringstream answer;

        for(int j = 0; j < sData->links.size(); j++)
        {
            //  good, put full authority with the custom connection of shorthand
            fullAuthority = sData->links.at(j);

            //  get the response number after fetching the methods from custom shhorthand url
            response = nKrawall::FetchURL( fullAuthority, "?query=methods", answer );

            if (response == -1)
                continue;

        }

        if (response == -1)
        {
            error.SetTemplateParameter(1, fullAuthority);
            error << "$custom_shorthand_url_unknown";

            return -1;
        }
        else return 1;
    }

    return 0;
}

bool nKrawall::CustomShorthandMethods(int userID, tString authority, tString & fullAuthority, tOutput & error, int & response, tString & id, tString & methods)
{
    if (CustomShorthandCheck(userID, authority, fullAuthority, error) == 1)
    {
        std::stringstream answer;
        response = nKrawall::FetchURL( fullAuthority, "?query=methods", answer );

        tString custom_id;
        answer >> custom_id;
        tToLower(custom_id);
        id = custom_id;

        tString custom_methods;
        std::ws(answer);
        custom_methods.ReadLine( answer );
        tToLower(custom_methods);
        methods = custom_methods;

        return true;
    }

    return false;
}
//  shorthand code END

//  authority code BEGIN
bool sn_CustomAuthorityEnabled = false;
static tSettingItem<bool> sn_CustomAuthorityEnabledConf("CUSTOM_AUTHORITY_ENABLED", sn_CustomAuthorityEnabled);

tString sn_CustomAuthority("");
static tSettingItem<tString> sn_CustomAuthorityConf("CUSTOM_AUTHORITY", sn_CustomAuthority);

tString sn_CustomAuthorityConnection("");
static tSettingItem<tString> sn_CustomAuthorityConnectionConf("CUSTOM_AUTHORITY_CONNECTION", sn_CustomAuthorityConnection);

int nKrawall::CustomAuthorityCheck(int userID, tString authority, tString &fullAuthority, tOutput & error)
{
    if (!sn_CustomAuthorityEnabled) return 0;

    //  check if calling custom shorthand exists
    if (authority == sn_CustomAuthority)
    {
        std::stringstream answer;

        //  good, put full authority with the custom connection of shorthand
        fullAuthority = sn_CustomAuthorityConnection;

        //  get the response number after fetching the methods from custom shhorthand url
        int response = nKrawall::FetchURL( fullAuthority, "?query=methods", answer );
        if (response == -1)
        {
            error.SetTemplateParameter(1, fullAuthority);
            error << "$custom_authority_url_unknown";

            return -1;
        }
        else
        {
            return 1;
        }
    }

    return 0;
}

bool nKrawall::CustomAuthorityMethods(int userID, tString authority, tString & fullAuthority, tOutput & error, int & response, tString & id, tString & methods)
{
    if (CustomAuthorityCheck(userID, authority, fullAuthority, error) == 1)
    {
        std::stringstream answer;
        response = nKrawall::FetchURL( fullAuthority, "?query=methods", answer );

        tString custom_id;
        answer >> custom_id;
        tToLower(custom_id);
        id = custom_id;

        tString custom_methods;
        std::ws(answer);
        custom_methods.ReadLine( answer );
        tToLower(custom_methods);
        methods = custom_methods;

        return true;
    }

    return false;
}
//  authority code END

int nKrawall::FetchURL( tString const & authority, char const * query, std::ostream & target, int maxlen )
{
    // compose real URL
    std::ostringstream fullURL;
    fullURL << "http://" << authority << "/armaauth/0.1/";
    fullURL << query;

    //con << fullURL.str() << "\n";

    // better not. output is not thread safe.
    // con << "Fetching authentication URL " << fullURL.str() << "\n";

    // fetch URL
    void * ctxt = xmlNanoHTTPOpen( fullURL.str().c_str(), NULL);
    if (ctxt == NULL)
    {
        return -1;
    }

    int rc = xmlNanoHTTPReturnCode(ctxt);

    // read content
    char buf[1000];
    buf[0] = 0;
    unsigned int len = 1;
    while ( len > 0 && maxlen > 0 )
    {
        int max = sizeof(buf);
        if ( max > maxlen )
            max = maxlen;
        len = xmlNanoHTTPRead( ctxt, &buf, max );
        target.write( buf, len );
        maxlen -= len;
    }

    xmlNanoHTTPClose(ctxt);

    return rc;
}

#ifdef KRAWALL_SERVER_LEAGUE
// TODO: REALLY change this!!
static nKrawall::nScrambledPassword key =
    {
        13, 12, 12, 12, 12, 12, 12, 12
    };


// secret key to encrypt server->master server league transfer
const nKrawall::nScrambledPassword& nKrawall::SecretLeagueKey()
{
    return key;
}

// called ON THE MASTER when victim drives against killer's wall
void nKrawall::MasterFrag(const tString &killer, const tString& victim)
{
    con << killer << " killed " << victim << "\n";
    // TODO: REAL league management
}


// called ON THE MASTER at the end of a round; the last survivor is stored in
// players[numPlayers-1], the first death in players[0]
void nKrawall::MasterRoundEnd(const tString* players, int numPlayers)
{
    if (numPlayers > 1)
    {
        con << players[numPlayers-1] << " survived over ";
        for (int i = numPlayers-2; i>=0; i--)
        {
            con << players[i];
            if (i > 0)
                con << " and ";
        }
        con << ".\n";
    }
    // TODO: REAL league management
}



// first validity check for the league messages
bool nKrawall::IsFromKrawall(tString& adress, unsigned int port)
{
    return (adress.Len() > 3 &&
            !strncmp(adress, "127.0.0", 7));
}

// check if a user is from germany (so the master server will require
// a password check)
bool nKrawall::RequireMasterLogin(tString& adress, unsigned int port)
{
    return (adress.Len() > 3 &&
            !strncmp(adress, "127.0.0", 7));
}

#endif
#endif
