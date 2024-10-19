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

#include "config.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include "tConfiguration.h"
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <sstream>
#include "tString.h"
#include "tToDo.h"
#include "tConsole.h"
#include "tDirectories.h"
#include "tLocale.h"
#include "tRecorder.h"
#include "tCommandLine.h"
#include "tResourceManager.h"
#include "tError.h"
#include "nConfig.h"

#include "ePlayerStats.h"
#include "tron/gHelper/gHelperUtilities.h"
#include <vector>
#include <string.h>

#ifndef WIN32
#include <signal.h>
#endif

/***********************************************************************
 * The new Configuration interface, currently not completely implemented
 */

#ifndef NEW_CONFIGURATION_NO_COMPILE

// Use this function to register your own configuration directive
//   returns true if it was successful, false if not
//   should return true even if it didn't register the new directive because
//       it was already there
bool tConfiguration::registerDirective(string newDirective, string defValue) {
    // THIS IS A STUB
}

// Use this function to set a configuration directive from a string
bool tConfiguration::setDirective(string oldDirective, string newValue) {
    // THIS IS A STUB
}

// Use this function to set a configuration directive from an int
bool tConfiguration::setDirective(string oldDirective, int newValue) {
    // THIS IS A STUB
}

// Use this function to set a configuration directive from a float
bool tConfiguration::setDirective(string oldDirective, double newValue) {
    // THIS IS A STUB
}

// Use this function to set a configuration directive from a bool
bool tConfiguration::setDirective(string oldDirective, bool newValue) {
    // THIS IS A STUB
}

// Use this function to get a configuration directive as a string
const string& tConfiguration::getDirective(string oldDirective) {
    // THIS IS A STUB
}

// Use this function to get a configuration directive as an int
const int& tConfiguration::getDirectiveI(string oldDirective) {
    // THIS IS A STUB
}

// Use this function to get a configuration directive as a double
const double& tConfiguration::getDirectiveF(string oldDirective) {
    // THIS IS A STUB
}

// Use this function to get a configuration directive as a bool
const bool& tConfiguration::getDirectiveB(string oldDirective) {
    // THIS IS A STUB
}

// Use this function to load a file
//  You *MUST* pass it a complete path from the root directory!
//  Returns TRUE if it can open the file and load at least some of it
//  Returns FALSE if complete failure.
bool tConfiguration::LoadFile(string filename) {
    // THIS IS A STUB
}

// Use this function to save current configuration to files
//   Returns TRUE on success, FALSE on failure
bool tConfiguration::SaveFile() {
    // THIS IS A STUB
}

// This function is used internally to actually set each directive
bool tConfiguration::_setDirective(string oldDirective, tConfigurationItem& newItem) {
    // THIS IS A STUB
}

// This function registers basic configuration directives
//  Create a new configuration directive by going to this function and
//  putting the appropriate line, then just use it in the game
//  It registers global defaults, but not object-specific defaults
void tConfiguration::_registerDefaults() {
    // THIS IS A STUB
}

tConfiguration* tConfiguration::_instance = 0;

const tConfiguration* tConfiguration::GetConfiguration() {
    if(_instance = 0) {
        _instance = new tConfiguration;
    }

    return _instance;
}

#endif


/*
 * The old stuff follows
 ************************************************************************/

bool           tConfItemBase::printChange=true;
bool           tConfItemBase::printErrors=true;

static void st_ToggleConfigItem( std::istream & s )
{
    tString name;
    s >> name;

    if ( name.Size() == 0 )
    {
        con << tOutput( "$toggle_usage_error" );
        return;
    }

    tConfItemBase *base = tConfItemBase::GetConfigItem( name );

    if ( !base )
    {
        con << tOutput( "$config_command_unknown", name );
        return;
    }

    tConfItem< bool > *confItem = dynamic_cast< tConfItem< bool > * >( base );
    if ( confItem && confItem->Writable() )
    {
        confItem->SetVal( !*confItem->GetTarget() );
    }
    else
    {
        con << tOutput( "$toggle_invalid_config_item", name );
    }
}

static tConfItemFunc toggleConfItemFunc( "TOGGLE", st_ToggleConfigItem );


//! @param newLevel         the new access level to set over the course of the lifetime of this object
//! @param allowElevation   only if set to true, getting higher access rights is possible. Use with extreme care.
tCurrentAccessLevel::tCurrentAccessLevel( tAccessLevel newLevel, bool allowElevation )
{
    // prevent elevation
    if ( !allowElevation && newLevel < currentLevel_ )
    {
        // you probably want to know when this happens in the debugger
        st_Breakpoint();
        newLevel = currentLevel_;
    }

    lastLevel_ = currentLevel_;
    currentLevel_ = newLevel;
}


tCurrentAccessLevel::tCurrentAccessLevel()
{
    lastLevel_ = currentLevel_;
}

tCurrentAccessLevel::~tCurrentAccessLevel()
{
    currentLevel_ = lastLevel_;
}

//! returns the current access level
tAccessLevel tCurrentAccessLevel::GetAccessLevel()
{
    tASSERT( currentLevel_ != tAccessLevel_Invalid );
    return currentLevel_;
}

// returns the name of an access level
tString tCurrentAccessLevel::GetName( tAccessLevel level )
{
    std::ostringstream s;
    s << "$config_accesslevel_" << level;
    return tString( tOutput( s.str().c_str() ) );
}

tAccessLevel tCurrentAccessLevel::currentLevel_ = tAccessLevel_Invalid; //!< the current access level

tAccessLevelSetter::tAccessLevelSetter( tConfItemBase & item, tAccessLevel level )
{
#ifdef KRAWALL_SERVER
    item.requiredLevel = level;
#endif
}

static std::map< tString, tConfItemBase * > * st_confMap = 0;
tConfItemBase::tConfItemMap & tConfItemBase::ConfItemMap()
{
    if (!st_confMap)
        st_confMap = tNEW( tConfItemMap );
    return *st_confMap;
}

tString tConfItemBase::lastLoadOutput("");
tString tConfItemBase::lastLoadCommandName("");

tConfItemBase::tConfItemMap const & tConfItemBase::GetConfItemMap()
{
    return ConfItemMap();
}

static bool st_preventCasacl = false;

tCasaclPreventer::tCasaclPreventer( bool prevent )
{
    previous_ = st_preventCasacl;
    st_preventCasacl = prevent;
}

tCasaclPreventer::~tCasaclPreventer()
{
    st_preventCasacl = previous_;
}

//! returns whether we're currently in an RINCLUDE file
bool tCasaclPreventer::InRInclude()
{
    return st_preventCasacl;
}

// changes the access level of a configuration item
class tConfItemLevel: public tConfItemBase
{
public:
    tConfItemLevel()
    : tConfItemBase( "ACCESS_LEVEL" )
    {
        requiredLevel = tAccessLevel_Owner;
    }

    virtual void ReadVal(std::istream &s)
    {
        // read name and access level
        tString name;
        s >> name;

        int levelInt;
        s >> levelInt;
        tAccessLevel level = static_cast< tAccessLevel >( levelInt );

        if ( s.fail() )
        {
            if(printErrors)
            {
                con << tOutput( "$access_level_usage" );
            }
            return;
        }

        // make name uppercase:
        tToUpper( name );

        // find the item
        tConfItemMap & confmap = ConfItemMap();
        tConfItemMap::iterator iter = confmap.find( name );
        if ( iter != confmap.end() )
        {
            // and change the level
            tConfItemBase * ci = (*iter).second;

            if( ci->requiredLevel < tCurrentAccessLevel::GetAccessLevel() )
            {
                con << tOutput( "$access_level_nochange_now",
                                name,
                                tCurrentAccessLevel::GetName( ci->requiredLevel ),
                                tCurrentAccessLevel::GetName( tCurrentAccessLevel::GetAccessLevel() ) );
            }
            else if( level < tCurrentAccessLevel::GetAccessLevel() )
            {
                con << tOutput( "$access_level_nochange_later",
                                name,
                                tCurrentAccessLevel::GetName( level ),
                                tCurrentAccessLevel::GetName( tCurrentAccessLevel::GetAccessLevel() ) );
            }
            else if ( ci->requiredLevel != level )
            {
                if(printChange)
                {
                    con << tOutput( "$access_level_change", name, tCurrentAccessLevel::GetName( ci->requiredLevel ) , tCurrentAccessLevel::GetName( level ) );
                }
                ci->requiredLevel = level;
            }
        }
        else if(printErrors)
        {
            con << tOutput( "$config_command_unknown", name );
        }
    }

    virtual void WriteVal(std::ostream &s)
    {
        tASSERT(0);
    }

    virtual void FetchVal(tString &val){};

    virtual bool Writable(){
        return false;
    }

    virtual bool Save(){
        return false;
    }

    // CAN this be saved at all?
    virtual bool CanSave(){
        return false;
    }
};

static tConfItemLevel st_confLevel;

#ifdef KRAWALL_SERVER

static char const *st_casacl = "CASACL";

//! casacl (Check And Set ACcess Level) command: elevates the access level for the context of the current configuration file
class tCasacl: tConfItemBase
{
public:
    tCasacl()
    : tConfItemBase( st_casacl )
    {
        requiredLevel = tAccessLevel_Program;
    }

    virtual void ReadVal( std::istream & s )
    {
        int required_int = 0, elevated_int = 20;

        // read required and elevated access levels
        s >> required_int;
        s >> elevated_int;

        tAccessLevel elevated = static_cast< tAccessLevel >( elevated_int );
        tAccessLevel required = static_cast< tAccessLevel >( required_int );

        if ( s.fail() )
        {
            con << tOutput( "$casacl_usage" );
            throw tAbortLoading( st_casacl );
        }
        else if ( tCurrentAccessLevel::GetAccessLevel() > required )
        {
            con << tOutput( "$access_level_error",
                            "CASACL",
                            tCurrentAccessLevel::GetName( required ),
                            tCurrentAccessLevel::GetName( tCurrentAccessLevel::GetAccessLevel() )
                );
            throw tAbortLoading( st_casacl );
        }
        else if ( st_preventCasacl )
        {
            con << tOutput( "$casacl_not_allowed" );
            throw tAbortLoading( st_casacl );
        }
        else
        {
            tString().ReadLine(s); // prevent commands following this one without a newline
            tCurrentAccessLevel::currentLevel_ = elevated;
        }
    }

    virtual void WriteVal(std::ostream &s)
    {
        tASSERT(0);
    }

    virtual void FetchVal(tString &val){};

    virtual bool Writable(){
        return false;
    }

    virtual bool Save(){
        return false;
    }

    // CAN this be saved at all?
    virtual bool CanSave(){
        return false;
    }
};

static tCasacl st_sudo;

#endif

bool st_FirstUse=true;
static tConfItem<bool> fu("FIRST_USE",st_FirstUse);
//static tConfItem<bool> fu("FIRST_USE","help_first_use",st_FirstUse);


bool st_displayValue = true;
static tConfItem<bool> st_displayValueConf("CONSOLE_DISPLAY_VALUE", st_displayValue);

tAbortLoading::tAbortLoading( char const * command )
: command_( command )
{
}

tString tAbortLoading::DoGetName() const
{
    return tString(tOutput( "$abort_loading_name"));
}

tString tAbortLoading::DoGetDescription() const
{
    return tString(tOutput( "$abort_loading_description", command_ ));
}

tConfItemBase::tConfItemBase(const char *t, const std::vector<std::pair<std::string, std::string>>& initValueMap)
        :id(-1),title(t), valueMap(initValueMap),
changed(false){

    tConfItemMap & confmap = ConfItemMap();
    if ( confmap.find( title ) != confmap.end() )
        tERR_ERROR_INT("Two tConfItems with the same name " << t << "!");

    // compose help name
    tString helpname;
    helpname << title << "_help";
    tToLower( helpname );

    const_cast<tOutput&>(help).AddLocale(helpname);

    confmap[title] = this;

    requiredLevel = tAccessLevel_Admin;
    setLevel      = tAccessLevel_Owner;
}

tConfItemBase::tConfItemBase(const char *t, const tOutput& h, const std::vector<std::pair<std::string, std::string>>& initValueMap)
        :id(-1),title(t), help(h), valueMap(initValueMap),
changed(false){

    tConfItemMap & confmap = ConfItemMap();
    if ( confmap.find( title ) != confmap.end() )
        tERR_ERROR_INT("Two tConfItems with the same name " << t << "!");

    confmap[title] = this;

    requiredLevel = tAccessLevel_Admin;
    setLevel      = tAccessLevel_Owner;
}

tConfItemBase::~tConfItemBase()
{
    tConfItemMap & confmap = ConfItemMap();
    confmap.erase(title);
    if ( confmap.size() == 0 )
    {
        delete st_confMap;
        st_confMap = 0;
    }
}

void tConfItemBase::ExportAll(){
    SaveAll(std::cout, true);
}

void tConfItemBase::SaveAll(std::ostream &s, bool all){
    tConfItemMap & confmap = ConfItemMap();
    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;
        if ((!all && ci->Save())||(all && ci->CanSave())){
            s << ci->title << " ";
            ci->WriteVal(s);
            s << "\n";
        }
    }
}

int tConfItemBase::EatWhitespace(std::istream &s){
    int c=' ';

    while(isblank(c) &&
            c!='\n'    &&
            s.good()  &&
            !s.eof())
        c=s.get();

    if( s.good() )
    {
        s.putback(c);
    }

    return c;
}
#include <regex>

bool tConfItemBase::applyValueToMatchedConfigs(const std::string &pattern, tConfItemBase::tConfItemMap &confmap, const std::string &valueStr)
{
    std::regex re(pattern);
    bool matched = false;

    for (auto &item : confmap)
    {
        tString title = item.second->GetTitle();
        std::string title_str(title);
        std::string pattern_str(pattern);
        std::smatch match;

        if (std::regex_search(title_str, match, re) && match[0].length() == title_str.length())
        {
            std::istringstream valueStream(valueStr);
            item.second->ReadVal(valueStream);
            matched = true;
        }
    }

    return matched;
}
void tConfItemBase::LoadLine(std::istream &s, bool wildCardEnabled)
{
    if (!s.eof() && s.good())
    {
        tConfItemBase::lastLoadOutput = tString("");
        tString name;
        s >> name;

        // make name uppercase:
        tToUpper(name);

        bool found = false;

        if (name[0] == '#')
        { // comment. ignore rest of line
            char c = ' ';
            while (c != '\n' && s.good() && !s.eof())
                c = s.get();
            found = true;
        }

        if (strlen(name) == 0) // ignore empty lines
            found = true;

        tConfItemMap &confmap = ConfItemMap();
        tConfItemMap::iterator iter = confmap.find(name);
        if (iter != confmap.end())
        {
            tConfItemBase *ci = (*iter).second;

            bool cb = ci->changed;
            ci->changed = false;

            tConfItemBase::lastLoadCommandName = name;
#ifdef DEBUG
            con << "Current level: " << tCurrentAccessLevel::GetAccessLevel() << "\n";
            con << "Required level: " << ci->requiredLevel << "\n";
#endif
            bool allowedChangeF = ci->allowedChange();
            if (ci->requiredLevel >= tCurrentAccessLevel::GetAccessLevel())
            {
                ci->setLevel = tCurrentAccessLevel::GetAccessLevel();

                if (allowedChangeF)
                {
                    ci->ReadVal(s);
                    if (ci->changed)
                    {
                        ci->WasChanged();
                    }
                    else
                    {
                        ci->changed = cb;
                    }
                }
            }
            else
            {
                tString discard;
                discard.ReadLine(s);

                tConfItemBase::lastLoadOutput << tOutput("$access_level_error",
                               name,
                               tCurrentAccessLevel::GetName(ci->requiredLevel),
                               tCurrentAccessLevel::GetName(tCurrentAccessLevel::GetAccessLevel()));
                con << tConfItemBase::lastLoadOutput;
                if (allowedChangeF)
                    return;
            }


            found = true;
            if (found && !allowedChangeF)
                 found = false;
        }

        if (!found)
        {
            // Store the current position of the input stream
            std::istream::pos_type pos = s.tellg();

            // Read the rest of the input line into a string
            tString rest;
            rest.ReadLine(s);

            bool wildcardMatched = false;
            if (wildCardEnabled && name.Contains("*"))
            {
                s.clear();
                s.seekg(pos);

                std::string pattern(name);
                std::string replaced_pattern;
                std::regex_replace(std::back_inserter(replaced_pattern), pattern.begin(), pattern.end(), std::regex("\\*"), "[^_]*"); // Replace * with [^_]*

                tString replaced_pattern_tstr(replaced_pattern);
                tToUpper(replaced_pattern_tstr);
                replaced_pattern = std::string(replaced_pattern_tstr);

                replaced_pattern = "^" + replaced_pattern + "$";
                std::string valueStr(rest);
                wildcardMatched = tConfItemBase::applyValueToMatchedConfigs(replaced_pattern, confmap, valueStr);
            }

            if (printErrors && !wildcardMatched)
            {
                tOutput o;
                o.SetTemplateParameter(1, name);
                o << "$config_command_unknown";
                con << o;
                tConfItemBase::lastLoadOutput << o;

                if (printChange)
                {
                    int sim_maxlen = -1;

                    for (tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end(); ++iter)
                    {
                        tConfItemBase *ci = (*iter).second;
                        if (strstr(ci->title, name) &&
                            static_cast<int>(strlen(ci->title)) > sim_maxlen)
                            sim_maxlen = strlen(ci->title);
                    }

                    if (sim_maxlen > 0 && printChange)
                    {
                        int len = name.Len() - 1;
                        int printMax = 1 + 3 * len * len * len;
                        con << tOutput("$config_command_other");
                        tConfItemBase::lastLoadOutput <<  tOutput("$config_command_other");
                        for (tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end(); ++iter)
                        {
                            tConfItemBase *ci = (*iter).second;
                            bool allowedChange = ci->allowedChange();

                            if (!allowedChange)
                                continue;

                            if (strstr(ci->title, name))
                            {
                                tString help(ci->help);
                                if (--printMax > 0)
                                {
                                    tString mess;
                                    mess << ci->title;
                                    mess.SetPos(sim_maxlen + 2, false);
                                    mess << "(";
                                    mess << help;
                                    mess << ")\n";
                                    if (st_displayValue)
                                        mess << " -  Value: " << ci->getFormatedValueMapValue(ci->getValue()) << "\n";
                                    con << mess;
                                }
                            }
                        }
                        if (printMax <= 0){
                            con << tOutput("$config_command_more");
                            tConfItemBase::lastLoadOutput << tOutput("$config_command_more");
                        }
                    }
                }
                else
                {
                    con << '\n';
                }
            }
        }
    }

    //  std::cout << line << " lines read.\n";
}
tConfItemBase *tConfItemBase::GetConfigItem(std::string const &name) {
    return tConfItemBase::GetConfigItem(tString(name));
}


tConfItemBase *tConfItemBase::GetConfigItem(tString const &name) {
    tConfItemMap & confmap = ConfItemMap();
    tConfItemMap::iterator iter = confmap.find( name.ToUpper() );
    if ( iter != confmap.end() ) {
        return iter->second;
    } else {
        return 0;
    }
}
std::deque<tString> tConfItemBase::GetCommands(){
    std::deque<tString> ret;
    tConfItemMap & confmap = ConfItemMap();
    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;

        ret.push_back(ci->title.ToLower());
    }
    return ret;
}


tString tConfItemBase::FindConfigItem(tString name,int pos)
{
    int currPos = 0;
    tConfItemMap & confmap = ConfItemMap();
    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;
        tString command;
        command << ci->title.ToLower();
        if (command.Contains(name.ToLower()))
        {
            if (command.StartsWith(name.ToLower()))
            {
                if(currPos != pos)
                {
                    currPos++;
                    continue;
                }
                return ci->title;
            }
        }
    }
    return tString("");
}

/** LISTING BEGIN **/
// writes the list of all commands and their help to config_all.cfg in the var directory
void tConfItemBase::WriteAllToFile()
{
    tConfItemMap & confmap = ConfItemMap();
    int sim_maxlen = -1;

    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;
        if (static_cast<int>(strlen(ci->title)) > sim_maxlen)
            sim_maxlen = strlen(ci->title);
    }

    std::ofstream w;
    if ( tDirectories::Var().Open(w, "config_all.cfg"))
    {
        /*w << "{| border=\"2\" cellspacing=\"0\" cellpadding=\"4\" rules=\"all\" style=\"margin:1em 1em 1em 0; border:solid 1px #AAAAAA; border-collapse:collapse; background-color:#F9F9F9; font-size:95%; empty-cells:show;\"\n";
        w << "!Command\n";
        w << "!Default\n";
        w << "!Meaning\n";
        w << "|-\n";
        */
        for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
        {
            tConfItemBase * ci = (*iter).second;
            tString help ( ci->help );

            tString mess, value;

            //  fetch the value set for this setting.
            ci->FetchVal(value);

            mess << ci->title << " ";

            mess.SetPos( sim_maxlen+2, false );
            mess << value << " ";
            mess << " # ";
            mess << help;
            mess << "\n";

            w << mess;
            /*w << "| " << ci->title << " || " << value << " || " << help << "\n";
            w << "|-\n";*/
        }
        //w << "|}\n";
    }
    w.close();
}

static void sg_ListAllCommands(std::istream &s)
{
    tConfItemBase::WriteAllToFile();
}
static tConfItemFunc sg_ListAllCommandsConf("LIST_ALL_COMMANDS", &sg_ListAllCommands);

/** LISTING END **/

/** LISTING ACCESS_LEVEL BEGIN **/
// writes the list of all commands and their help to config_all_levels.cfg in the var directory
void tConfItemBase::WriteAllLevelsToFile()
{
    tConfItemMap & confmap = ConfItemMap();
    int sim_maxlen = -1;

    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;
        if (static_cast<int>(strlen(ci->title)) > sim_maxlen)
            sim_maxlen = strlen(ci->title);
    }

    std::ofstream w;
    if ( tDirectories::Var().Open(w, "config_all_levels.cfg"))
    {
        for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
        {
            tConfItemBase * ci = (*iter).second;
            tString help ( ci->help );

            tString mess;

            mess << "ACCESS_LEVEL ";
            mess << ci->title << " ";

            //mess.SetPos( sim_maxlen+2, false );
            mess << ci->requiredLevel;
            mess.SetPos( sim_maxlen+5, false );
            mess << " # ";
            mess << help;
            mess << "\n";

            w << mess;
        }
    }
    w.close();
}

static void sg_ListAllCommandsLevels(std::istream &s)
{
    tConfItemBase::WriteAllLevelsToFile();
}
static tConfItemFunc sg_ListAllCommandsLevelsConf("LIST_ALL_COMMANDS_LEVELS", &sg_ListAllCommandsLevels);

/** LISTING ACCESS_LEVEL END **/

/** SET ALL ACCESS_LEVEL BEGIN **/
void tConfItemBase::SetAllAccessLevel(int newLevel)
{
    tConfItemMap & confmap = ConfItemMap();
    tAccessLevel level = static_cast< tAccessLevel >( newLevel );

    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;
        ci->requiredLevel = level;
    }

    tString message;
    message << "All access levels of commands have been changed to " << tCurrentAccessLevel::GetName(level) << "!\n";
    con << message;
}

static void st_SetCommandsAccessLevel(std::istream &s)
{
    tString levelStr;
    s >> levelStr;

    int newLevel = atoi(levelStr);
    tConfItemBase::SetAllAccessLevel(newLevel);
}
static tConfItemFunc st_SetCommandsAccessLevelConf("SET_COMMANDS_ACCESSLEVEL", &st_SetCommandsAccessLevel);
/** SET ALL ACCESS_LEVEL END **/

tString st_configChanged( "config_changed.cfg" );
void tConfItemBase::WriteChangedToFile()
{
    if( !st_configChanged.EndsWith(".cfg") )
    {
        st_configChanged << ".cfg";
    }

    if(
        !tPath::IsValidPath( st_configChanged ) ||
        st_configChanged.StartsWith("user") ||
        st_configChanged.StartsWith("autoexec") ||
        !st_configChanged.EndsWith(".cfg")
    )
    {
        con << "Cowardly refusing to save file to " << st_configChanged << "\n";
        st_configChanged = "config_changed.cfg";
        return;
    }

    tConfItemMap & confmap = ConfItemMap();
    int sim_maxlen = -1;

    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;
        if (static_cast<int>(strlen(ci->title)) > sim_maxlen)
            sim_maxlen = strlen(ci->title);
    }

    std::ofstream w;
    if ( tDirectories::Var().Open(w, st_configChanged))
    {
        for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
        {
            tConfItemBase * ci = (*iter).second;
            if (!ci->changed) continue;

            tString help ( ci->help );

            tString mess, value;

            //  fetch the value set for this setting.
            ci->FetchVal(value);

            mess << ci->title << " ";

            mess.SetPos( sim_maxlen+2, false );
            mess << value << " ";
            mess << " # ";
            mess << help;
            mess << "\n";

            w << mess;
        }
    }
    w.close();
    st_configChanged = "config_changed.cfg";
}

tString settingsDownloadCfg("server_settings.cfg");
void tConfItemBase::DownloadSettings_Go(nMessage &m)
{
    static tString currentDownloadFile;

    //  download the config if this is a client
    if (sn_GetNetState() == nCLIENT)
    {
        if( settingsDownloadCfg.Len() > 1 )
        {
            if( !settingsDownloadCfg.EndsWith(".cfg") )
            {
                settingsDownloadCfg << ".cfg";
            }

            if(
                !tPath::IsValidPath( settingsDownloadCfg ) ||
                settingsDownloadCfg.StartsWith("user") ||
                settingsDownloadCfg.StartsWith("autoexec") ||
                !settingsDownloadCfg.EndsWith(".cfg")
            )
            {
                con << "Cowardly refusing to save file to " << settingsDownloadCfg << "\n";
                settingsDownloadCfg = "";
                return;
            }
        }

        tString m_title;
        m >> m_title;
        if ((m_title == "DOWNLOAD_BEGIN") && m.End())
        {
            if( settingsDownloadCfg.Len() <= 1 ) return;

            con << "Downloading config from server...\n";

            //  truncate the file for fresh settings
            std::ofstream o;
            if ( tDirectories::Var().Open(o, settingsDownloadCfg, std::ios::trunc) ) {}
            o.close();

            currentDownloadFile = settingsDownloadCfg;

            return;
        }
        else if ((m_title == "DOWNLOAD_END") && m.End())
        {
            if( settingsDownloadCfg.Len() > 1 )
                con << "Download complete!\n";

            settingsDownloadCfg = "server_settings.cfg";
            return;
        }
        tString m_value;
        m >> m_value;

        if( settingsDownloadCfg.Len() <= 1 ) return;

        std::ofstream o;
        if (tDirectories::Var().Open(o, currentDownloadFile, std::ios::app))
        {
            o << m_title << " " << m_value << "\n";
        }
        o.close();
    }
}

static nDescriptor downloadSettings(61,tConfItemBase::DownloadSettings_Go, "download settings");

void tConfItemBase::DownloadSettings_To(int peer)
{
    //  transfer the settings to the requested client
    //  Client ID must be greater than 0 for it to work
    if ((sn_GetNetState() == nSERVER) && (peer > 0))
    {
        {
            nMessage *m=new nMessage(downloadSettings);
            *m << tString("DOWNLOAD_BEGIN");
            m->Send(peer);
        }

        tConfItemMap & confmap = ConfItemMap();
        for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
        {
            tConfItemBase * item = (*iter).second;
            if (item && item->CanSave())
            {
                nMessage *m=new nMessage(downloadSettings);
                *m << item->title;

                tString value;
                item->FetchVal(value);
                *m << value;

                m->Send(peer);
            }
        }

        {
            nMessage *m=new nMessage(downloadSettings);
            *m << tString("DOWNLOAD_END");
            m->Send(peer);
        }
    }
}

tString configFileDownload("");
void tConfItemBase::DownloadConfig_Go(nMessage &m)
{
    //  download the config if this is a client
    if (sn_GetNetState() == nCLIENT)
    {
        tString c_line;
        m >> c_line;
        if ((c_line == "DOWNLOAD_BEGIN") && !m.End())
        {
            m >> configFileDownload;
            con << "Downloading \"" << configFileDownload << "\" from server...\n";

            //  truncate the file for fresh settings
            std::ofstream o;
            tString configFile;
            configFile << "public/" << configFileDownload;
            if ( tDirectories::Config().Open(o, configFile, std::ios::trunc) ) {}
            o.close();
            return;
        }
        else if ((c_line == "DOWNLOAD_END") && m.End())
        {
            con << "Download complete!\n";
            configFileDownload = "";
            return;
        }

        if (configFileDownload == "") return;

        std::ofstream o;
        tString configFile;
        configFile << "public/" << configFileDownload;
        if (tDirectories::Config().Open(o, configFile, std::ios::app))
        {
            o << c_line << "\n";
        }
        o.close();
    }
}

static nDescriptor downloadConfigs(62,tConfItemBase::DownloadConfig_Go, "download config");

bool st_downloadConfigProcess = false;
void tConfItemBase::DownloadConfig_To(tString file, int peer)
{
    //  don't process if its under process
    if (st_downloadConfigProcess) return;
    st_downloadConfigProcess = true;

    // refuse to load illegal paths
    if( !tPath::IsValidPath( file ) )
        return;

    nMessage *m = NULL;

    std::ifstream i;
    tString configFile;
    configFile << "public/" << file;

    //  transfer the settings from config to the client
    //  Client ID must be greater than 0 for it to work
    if ((sn_GetNetState() == nSERVER) && (peer > 0) && tDirectories::Config().Open(i, configFile))
    {
        m = new nMessage(downloadConfigs);
        *m << tString("DOWNLOAD_BEGIN");
        *m << file;
        m->Send(peer);

        std::istream &s(i);
        int i = 0;
        while (!s.eof() && s.good())
        {
            tString line;
            line.ReadLine(s);

            m = new nMessage(downloadConfigs);
            *m << line;
            m->Send(peer);

            i++;
        }

        m = new nMessage(downloadConfigs);
        *m << tString("DOWNLOAD_END");
        m->Send(peer);
    }
    i.close();

    st_downloadConfigProcess = false;
}


int tConfItemBase::AccessLevel(std::istream &s){
    if(!s.eof() && s.good()){
        tString name;
        s >> name;
        // make name uppercase:
        tToUpper( name );
        tConfItemMap & confmap = ConfItemMap();
        tConfItemMap::iterator iter = confmap.find( name );
        if ( iter != confmap.end() )
        {
            tConfItemBase * ci = (*iter).second;
            return ci->requiredLevel;
        }
    }
    return -1;

}

static char const * recordingSection = "CONFIG";

/*
bool LoadAllHelper(std::istream * s)
{
    tString line;

    // read line from recording
    if ( s || !tRecorder::Playback( recordingSection, line ) )
    {
        // return on failure
        if (!s)
            return false;

        // read line from stream
        line.ReadLine( *s );

        // write line to recording
        tRecorder::Record( recordingSection, line );
    }
    line << '\n';

    // process line
    std::stringstream str(static_cast< char const * >( line ) );
    tConfItemBase::LoadLine(str);

    return true;
}
*/

// test if a line should enter the recording; check if the line contains one of the passed
// substrings. The array of substrings is supposed to be zero terminated.
static bool s_Veto( tString line_in, std::vector< tString > const & vetos )
{
    // make name uppercase:
    tToUpper( line_in );

    // eat whitespace at the beginning
    char const * test = line_in;
    while( isblank(*test) )
        test++;

    // skip "LAST_"
    tString line( test );
    if ( line.StartsWith( "LAST_" ) )
        line = tString( static_cast< char const * >(line) + 5 );

    // iterate throug vetoed config items and test each one
    for ( std::vector< tString >::const_iterator iter = vetos.begin(); iter != vetos.end(); ++iter )
    {
        tString const & veto = *iter;

        if ( line.StartsWith( veto ) )
        {
#ifdef DEBUG_X
            if ( !line.StartsWith( "INCLUDE" ) && tRecorder::IsRunning() )
            {
                con << "Veto on config line: " << line << "\n";
            }
#endif

            return true;
        }
    }

    return false;
}

// test if a line should be read from a recording
// sound and video mode settings shold not
static std::vector< tString > st_Stringify( char const * vetos[] )
{
    std::vector< tString > ret;

    char const * * v = vetos;
    while ( *v )
    {
        ret.push_back( tString( *v ) );
        ++v;
    }

    return ret;
}

static bool s_VetoPlayback( tString const & line )
{
    static char const * vetos_char[]=
        { "USE_DISPLAYLISTS", "CHECK_ERRORS", "ZDEPTH",
          "COLORDEPTH", "FULLSCREEN ", "ARMAGETRON_LAST_WINDOWSIZE",
          "ARMAGETRON_WINDOWSIZE", "ARMAGETRON_LAST_SCREENMODE",
          "ARMAGETRON_SCREENMODE", "CUSTOM_SCREEN", "SOUND",
          "PASSWORD", "ADMIN_PASS",
          "ZTRICK", "MOUSE_GRAB", "PNG_SCREENSHOT", // "WHITE_SPARKS", "SPARKS",
          "KEEP_WINDOW_ACTIVE", "TEXTURE_MODE", "TEXTURES_HI", "LAG_O_METER", "INFINITY_PLANE",
          "SKY_WOBBLE", "LOWER_SKY", "UPPER_SKY", "DITHER", "HIGH_RIM", "FLOOR_DETAIL",
          "FLOOR_MIRROR", "SHOW_FPS", "SHOW_RECORDING_TIME", "TEXT_OUT", "SMOOTH_SHADING", "ALPHA_BLEND",
          "PERSP_CORRECT", "POLY_ANTIALIAS", "LINE_ANTIALIAS", "FAST_FORWARD_MAXSTEP",
          "DEBUG_GNUPLOT", "FLOOR_", "MOVIEPACK_", "RIM_WALL_",
          0 };

    static std::vector< tString > vetos = st_Stringify( vetos_char );

    // delegate
    return s_Veto( line, vetos );
}

// test if a line should enter the recording
// passwords should not.
static bool s_VetoRecording( tString const & line )
{
    static char const * vetos_char[]=
        { "#", "PASSWORD", "ADMIN_PASS", "LOCAL_USER", "LOCAL_TEAM",
          0 };

    static std::vector< tString > vetos = st_Stringify( vetos_char );

    // delegate
    return s_Veto( line, vetos ) || s_VetoPlayback( line );
}

void tConfItemBase::LoadAllFromMenu(std::istream &s){
    tCurrentAccessLevel levelResetter;

    try{

    while(!s.eof() && s.good())
    {
        tString line;

        // read line from stream
        line.ReadLine( s );

        /// concatenate lines ending in a backslash
        while ( line.Len() > 1 && line[line.Len()-2] == '\\' && s.good() && !s.eof() )
        {
            line[line.Len()-2] = '\0';

            // unless it is a double backslash
            if ( line.Len() > 2 && line[line.Len()-3] == '\\' )
            {
                break;
            }

            line.SetLen( line.Len()-1 );
            tString rest;
            rest.ReadLine( s );
            line << rest;
        }

        if ( line.Len() <= 1 )
            continue;

        //        std::cout << line << '\n';

        // process line
        // line << '\n';
        if ( !tRecorder::IsPlayingBack() || s_VetoPlayback( line ) )
        {
            std::stringstream str(static_cast< char const * >( line ) );
            tConfItemBase::LoadLine(str, true);
        }
    }
    }
    catch( tAbortLoading const & e )
    {
        // loading was aborted
        con << e.GetDescription() << "\n";
    }
}

//! @param s        stream to read from
//! @param record   set to true if the configuration is to be recorded and played back. That's usually only required if s is a file stream.
void tConfItemBase::LoadAll(std::istream &s, bool record ){
    tCurrentAccessLevel levelResetter;

    try{

    while(!s.eof() && s.good())
    {
        tString line;

        // read line from stream
        line.ReadLine( s );

        /// concatenate lines ending in a backslash
        while ( line.Len() > 1 && line[line.Len()-2] == '\\' && s.good() && !s.eof() )
        {
            line[line.Len()-2] = '\0';

            // unless it is a double backslash
            if ( line.Len() > 2 && line[line.Len()-3] == '\\' )
            {
                break;
            }

            line.SetLen( line.Len()-1 );
            tString rest;
            rest.ReadLine( s );
            line << rest;
        }

        if ( line.Len() <= 1 )
            continue;

        // write line to recording
        if ( record && !s_VetoRecording( line ) )
        {
            // don't record supid admins' instant chat logins
            static tString instantChat("INSTANT_CHAT_STRING");
            if ( line.StartsWith( instantChat ) && strstr( line, "/login" ) )
            {
                tString newLine = line.SubStr( 0, strstr( line, "/login" ) - (char const *)line );
                newLine += "/login NONE";
                if ( line[strlen(line)-1] == '\\' )
                    newLine += '\\';
                tRecorder::Record( recordingSection, newLine );
            }
            else
                tRecorder::Record( recordingSection, line );
        }

        //        std::cout << line << '\n';

        // process line
        // line << '\n';
        if ( !record || !tRecorder::IsPlayingBack() || s_VetoPlayback( line ) )
        {
            std::stringstream str(static_cast< char const * >( line ) );
            tConfItemBase::LoadLine(str);
            // std::cout << line << '\n';
        }
    }
    }
    catch( tAbortLoading const & e )
    {
        // loading was aborted
        con << e.GetDescription() << "\n";
    }
}

void tConfItemBase::DocAll(std::ostream &s){
    tConfItemMap & confmap = ConfItemMap();
    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;

        tString help ( ci->help );
        if ( help != "UNDOCUMENTED" )
        {
            tString line;
            line << ci->title;
            line.SetPos( 30, false );
            line << help;
            s << line << '\n';
        }
    }
}

void tConfItemBase::DocAll()
{
    tConfItemMap & confmap = ConfItemMap();
    FileManager fileManager(tString("config_all.cfg"), tDirectories::Var());
    fileManager.Clear(false);

    tString documented;
    documented   << "##############################################################################\n"
                 << "#                                 DOCUMENTED                                 #\n"
                 << "##############################################################################\n";

    tString undocumented;
    undocumented << "##############################################################################\n"
                 << "#                                UNDOCUMENTED                                #\n"
                 << "##############################################################################\n";
        
    for(tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; ++iter)
    {
        tConfItemBase * ci = (*iter).second;

        tString help ( ci->help );
        tString line;
        line << ci->title;
        line.SetPos( 30, false );
        
        tString undocumentedHelpStr;

        undocumentedHelpStr << line.ToLower().StripWhitespace() << "_help";

        if ( help != "UNDOCUMENTED" && help != undocumentedHelpStr)
        {
            line << help;
            documented << line << '\n';
        } else {
            line << help;
            undocumented << line << '\n';
        }
    }
    tString output;
    output << documented 
           << "\n\n\n\n"
           << undocumented;

    fileManager.Write(output);
}

static void sg_ListAllCommandsSplitByDocumented(std::istream &s)
{
    tConfItemBase::DocAll();
}
static tConfItemFunc sg_ListAllCommandsSplitByDocumentedConf("LIST_ALL_COMMANDS_SPLIT_BY_DOCUMENTED", &sg_ListAllCommandsSplitByDocumented);


//! @param s        stream to read from
//! @param record   set to true if the configuration is to be recorded and played back. That's usually only required if s is a file stream, so it defaults to true here.
void tConfItemBase::LoadAll(std::ifstream &s, bool record )
{
    std::istream &ss(s);
    LoadAll( ss, record );
}


//! @param s        file stream to be used for reading later
//! @param filename name of the file to open
//! @param path     whether to look in var directory
//! @return success flag
bool tConfItemBase::OpenFile( std::ifstream & s, tString const & filename, SearchPath path )
{
    bool ret = ( ( path & Config ) && tDirectories::Config().Open(s, filename ) ) || ( ( path & Var ) && st_StringEndsWith(filename, ".cfg") && tDirectories::Var().Open(s, filename ) );

    static char const * section = "INCLUDE_VOTE";
    tRecorder::Playback( section, ret );
    tRecorder::Record( section, ret );

    return ret;
}

//! @param s        file to read from
void tConfItemBase::ReadFile( std::ifstream & s )
{
    if ( !tRecorder::IsPlayingBack() )
    {
        tConfItemBase::LoadAll(s, true );
    }
    else
    {
        tConfItemBase::LoadPlayback();
    }
}

/*
  void tConfItemBase::ReadVal(std::istream &s);
  void tConfItemBase::WriteVal(std::istream &s);
*/

/*
tString configfile(){
	tString f;
	//#ifndef WIN32
	//  f << static_cast<const char *>(getenv("HOME"));
	//  f << st_LogDir <<
	//  f << "/.ArmageTronrc";
	//#else
		const tPath& vpath = tDirectories::Var();
		for ( int prio = vpath.MaxPriority(); prio>=0; --prio )
		{
			tString path = vpath.Path( prio );
			prio = -1;

	f << st_LogDir << "/user.cfg";
	//#endif
	return f;
}
*/

bool tConfItemBase::LoadPlayback( bool print )
{
    if ( !tRecorder::IsPlayingBack() )
        return false;

    // read line from recording
    tString line;
    while ( tRecorder::Playback( recordingSection, line ) )
    {
        tRecorder::Record( recordingSection, line );
        if ( !s_VetoPlayback( line ) )
        {
            // process line
            if ( print ) con << "Playback input : " << line << '\n';
            std::stringstream str(static_cast< char const * >( line ) );
            tConfItemBase::LoadLine(str);
        }
    }

    return true;
}

static bool Load( const tPath& path, const char* filename )
{
    // read from file
    if ( !filename )
    {
        return false;
    }

    std::ifstream s;
    if ( path.Open( s, filename ) )
    {
        tConfItemBase::LoadAll( s, true );
        return true;
    }
    else
    {
        return false;
    }
}

// flag indicating whether settings were read from a playback
static bool st_settingsFromRecording = false;

#ifdef DEDICATED
tString extraConfig("NONE");

class tExtraConfigCommandLineAnalyzer: public tCommandLineAnalyzer
{
private:
    bool DoAnalyze( tCommandLineParser & parser, int pass ) override
    {
        if(pass > 0)
            return false;

        return parser.GetOption(extraConfig, "--extraconfig", "-e");
    }

    void DoHelp( std::ostream & s ) override
    {                                      //
        s << "-e, --extraconfig            : open an extra configuration file after\n"
        << "                               settings_dedicated.cfg\n";
    }
};

static tExtraConfigCommandLineAnalyzer s_extraAnalyzer;
#endif

static void st_InstallSigHupHandler();

static tString st_customConfigLine = tString("");
static tSettingItem<tString> st_customConfigLineConf("CUSTOM_CONFIGS", st_customConfigLine);

//  CUSTOM_CONFIGS config1.cfg;config2.cfg;
void st_LoadCustomConfigs()
{
    if (st_customConfigLine.Filter() != "")
    {
        tArray<tString> configs = st_customConfigLine.Split(";");
        if (configs.Len() > 0)
        {
            for(int i = 0; i < configs.Len(); i++)
            {
                tString config = configs[i];

                Load(tDirectories::Config(), config);
                Load(tDirectories::Var(), config);
                Load(tDirectories::Resource(), config);
            }
        }
    }
}
static void st_LoadCustomConfigsStr(std::istream &s)
{
    st_LoadCustomConfigs();
}
static tConfItemFunc st_LoadCustomConfigsConf("LOAD_CUSTOM_CONFIGS", &st_LoadCustomConfigsStr);

void st_LoadConfig( bool printChange )
{
    // default include files are executed at owner level
    tCurrentAccessLevel level( tAccessLevel_Owner, true );

    st_InstallSigHupHandler();

    const tPath& var = tDirectories::Var();
    const tPath& config = tDirectories::Config();
    const tPath& data = tDirectories::Data();

    tConfItemBase::printChange=printChange;
#ifdef DEDICATED
    tConfItemBase::printErrors=false;
#endif
    tConfItemBase::printErrors=true;
    Load( config, "settings.cfg" );

    Load( config, "user.cfg" );
    Load( var, "user_extended.cfg" );

#ifdef DEDICATED
    Load( config, "settings_dedicated.cfg" );
    if( extraConfig != "NONE" ) Load( config, extraConfig );
#else
    if (st_FirstUse)
    {
        Load( config, "default.cfg" );
    }

    Load( config, "settings_client.cfg" );
#endif

    Load( data, "moviepack/settings.cfg" );

    Load( config, "autoexec.cfg" );
    Load( var, "autoexec.cfg" );
    st_LoadCustomConfigs();

    // load configuration from playback
    tConfItemBase::LoadPlayback();
    st_settingsFromRecording = tRecorder::IsPlayingBack();

    tConfItemBase::printChange=true;
}

#include "eChatCommands.h"
void sg_saveConfigCmd(std::istream &s)
{
    con << tThemedTextBase.LabelText("CONFIG")
        << "Saving Config..\n";
    st_SaveConfig();
}

static tConfItemFunc sg_saveConfigConf("SAVE_CONFIG", &sg_saveConfigCmd);

static void st_ReLoadConfig(std::istream &s)
{
    con << "Reloading config..\n";
    st_LoadConfig(false);
}
static tConfItemFunc st_ReLoadConfigConf("RELOAD_CONFIG", &st_ReLoadConfig);
static tAccessLevelSetter st_ReLoadConfigConfLevel( st_ReLoadConfigConf, tAccessLevel_Moderator );

static bool st_UserCfgSave = true;
static tConfItem<bool> st_UserCfgSaveConf("CFG_USER_SAVE", st_UserCfgSave);

extern std::map < std::string, float > sg_zoneHeights;
extern REAL sg_zoneHeight, sg_zoneSegLength, sg_zoneBottom;
extern int sg_zoneSegments, sg_zoneAlphaToggle;
extern bool sg_zoneAlphaBlend;

void st_GrabConfigInfo( std::map < tString, bool > * _saved, std::map < tString, tString > * _oldval, const char * ignore[] )
{
    std::ifstream i;
    tDirectories::Var().Open( i, "user_extended.cfg" );

    while( !i.eof() && i.good() )
    {
        tString line;

        // read line from stream
        line.ReadLine( i );

        int pos = 0;

        /// concatenate lines ending in a backslash
        while ( line.Len() > 1 && line[line.Len()-2] == '\\' && i.good() && !i.eof() )
        {
            line[line.Len()-2] = '\0';

            // unless it is a double backslash
            if ( line.Len() > 2 && line[line.Len()-3] == '\\' )
            {
                break;
            }

            line.SetLen( line.Len()-1 );
            tString rest;
            rest.ReadLine( i );
            line << rest;
        }

        if ( line.Len() <= 1 )
            continue;

        tString cmd = line.ExtractNonBlankSubString(pos);

        if( ignore )
        {
            const char ** i = ignore;
            while(*i)
            {
                if( cmd == *i ) break;
                i++;
            }
            if(*i) continue;
        }

        if(_saved) (*_saved) [ cmd ] = true;
        if(_oldval) (*_oldval) [ cmd ] = line;
    }
}

void st_SaveConfig()
{

    if (se_playerStats)
        ePlayerStats::saveStatsToDB();

    // don't save while playing back
    if ( st_settingsFromRecording )
    {
        return;
    }

    std::ofstream s;
    if ( tDirectories::Config().Open( s, "user.cfg", std::ios::out, true ) )
    {
        if (st_UserCfgSave)
            tConfItemBase::SaveAll(s);
    }
    else
    {
        tOutput o("$config_file_write_error");
        con << o;
        std::cerr << o;
    }

    if( !tRecorder::IsPlayingBack() )
    {
        std::map < tString, bool > saved {};
        std::map < tString, tString > oldval {};

        static const char * ignoreList[] = {
            "ZONE_ALPHA_BLEND",
            "ZONE_ALPHA_TOGGLE",
            "ZONE_BOTTOM",
            "ZONE_HEIGHT",
            "ZONE_SEGMENTS",
            "ZONE_SEG_LENGTH",
        0};

        st_GrabConfigInfo( &saved, &oldval, ignoreList );

        std::ofstream s;
        if ( tDirectories::Var().Open( s, "user_extended.cfg", std::ios::out, true ) )
        {
            if( st_UserCfgSave )
            {
                int id, i;

                for(auto i=saved.begin();i!=saved.end();i++)
                {
                    auto ci = tConfItemBase::GetConfigItem(i->first);
                    if(ci)
                    {
                        s << ci->GetTitle() << " ";
                        ci->WriteVal(s);
                        s << "\n";
                    }
                    else
                    {
                        s << oldval[ i->first ] << "\n";
                    }
                }

                // for(id=0;id<4;++id)
                // {
                //     for(i=25; i<99; ++i)
                //     {
                //         tString confname; confname << "INSTANT_CHAT_STRING_" << id+1 << '_' <<  i+1;
                //         if( saved[ confname ] ) continue;
                //         auto ci = tConfItemBase::GetConfigItem(confname);
                //         if(ci)
                //         {
                //             s << ci->GetTitle() << " ";
                //             ci->WriteVal(s);
                //             s << "\n";
                //         }
                //     }
                // }

                // s << "ZONE_ALPHA_BLEND" << " " << sg_zoneAlphaBlend << "\n";
                // s << "ZONE_ALPHA_TOGGLE" << " " << sg_zoneAlphaToggle << "\n";

                // s << "ZONE_BOTTOM" << " " << sg_zoneBottom << "\n";

                // s << "ZONE_HEIGHT" << " " << sg_zoneHeight << "\n";
                // for(auto i=sg_zoneHeights.begin();i!=sg_zoneHeights.end();i++)
                // {
                //     tString c("ZONE_HEIGHT");
                //     c << " " << i->first;
                //     c << " " << i->second;
                //     s << c << "\n";
                // }

                // s << "ZONE_SEGMENTS" << " " << sg_zoneSegments << "\n";
                // s << "ZONE_SEG_LENGTH" << " " << sg_zoneSegLength << "\n";
            }
        }
        else
        {
            tOutput o("$config_file_write_error");
            con << o;
            std::cerr << o;
        }
    }
}

tString st_AddToUserExt( tArray<tString> commands )
{
    tString out;

    if( tRecorder::IsPlayingBack() ) return out;

    std::map < tString, bool > saved {};
    st_GrabConfigInfo( &saved, nullptr, nullptr );

    std::ofstream s;
    tDirectories::Var().Open( s, "user_extended.cfg", std::ios::app, true );

    int pos = -1, savedi = 0;

    tString cmd;
    while( commands[ ++pos ].Len() > 1 )
    {
        cmd = commands[ pos ].ToUpper();
        auto ci = tConfItemBase::GetConfigItem( cmd );
        if(ci)
        {
            if( !ci->Save() && !saved [ ci->GetTitle() ] )
            {
                s << ci->GetTitle() << " ";
                ci->WriteVal(s);
                s << "\n";

                ++savedi;
            }
            else
            {
                out << "Setting " << ci->GetTitle() << " will already be saved. Skipping...\n";
            }
        }
        else
        {
            out << "Command " << cmd << " invalid.\n";
        }
    }
    out << "Done, saved " << savedi << " / " << ( pos ) << " settings.\n";

    return out;
}

void st_LoadUserConfig()
{
    const tPath& var = tDirectories::Var();
    Load(var, "user.cfg");
    Load( var, "user_extended.cfg" );
}

void st_LoadConfig()
{
    st_LoadConfig( false );
}

static void st_DoHandleSigHup()
{
    con << tOutput("$config_sighup");
    st_SaveConfig();
    st_LoadConfig();
}

static void st_HandleSigHup( int signal )
{
    st_ToDo_Signal( st_DoHandleSigHup );
}

static void st_InstallSigHupHandler()
{
#ifndef WIN32
    static bool installed = false;
    if ( !installed )
    {
        signal( SIGHUP, &st_HandleSigHup );
        installed = true;
    }
#endif
}

void tConfItemLine::ReadVal(std::istream &s){
    tString dummy;
    dummy.ReadLine(s, true);
    if(strcmp(dummy,*target)){
        if (printChange)
        {
            tColoredString oldval;
            oldval << *target << tColoredString::ColorString(1,1,1);
            tColoredString newval;
            newval << dummy << tColoredString::ColorString(1,1,1);
            tOutput o;
            o.SetTemplateParameter(1, title);
            o.SetTemplateParameter(2, oldval);
            o.SetTemplateParameter(3, newval);
            o << "$config_value_changed";
            con << o;
            tConfItemBase::lastLoadOutput << o;
        }
        *target=dummy;
        changed=true;
    }

    *target=dummy;
}


void tConfItemLine::WriteVal(std::ostream &s){
    // slow, but correct: escape special characters on the fly
    int len = target->Len();
    for(int i = 0; i < len; ++i)
    {
        char c = (*target)[i];
        switch(c)
        {
            case 0:
                continue;
            case '\\':
            case '\'':
            case '"':
            case ' ':
                s << '\\';
        }
        s << c;
    }
}

tConfItemFunc::tConfItemFunc
(const char *title, CONF_FUNC *func)
    :tConfItemBase(title),f(func), shouldChangeFunc_(NULL){}

tConfItemFunc::~tConfItemFunc(){}

void tConfItemFunc::ReadVal(std::istream &s){(*f)(s);}
void tConfItemFunc::WriteVal(std::ostream &){}
void tConfItemFunc::FetchVal(tString &val){}

bool tConfItemFunc::Save(){return false;}

void st_Include( tString const & file )
{
    // refuse to load illegal paths
    if( !tPath::IsValidPath( file ) )
        return;

    if ( !tRecorder::IsPlayingBack() )
    {
        // really load include file
        if ( !file.EndsWith(".cfg") || !Load( tDirectories::Var(), file ) )
        {
            if (!Load( tDirectories::Config(), file ) && tConfItemBase::printErrors )
            {
                con << tOutput( "$config_include_not_found", file );
            }
        }
    }
    else
    {
        // just read configuration, and don't forget to reset the config level
        tCurrentAccessLevel levelResetter;
        tConfItemBase::LoadPlayback();
    }

    // mark section
    char const * section = "INCLUDE_END";
    tRecorder::Record( section );
    tRecorder::PlaybackStrict( section );

}

static void Include(std::istream& s, bool error )
{
    // allow CASACL
    tCasaclPreventer allower(false);

    tString file;
    s >> file;

    // refuse to load illegal paths
    if( !tPath::IsValidPath( file ) )
        return;

    if ( !tRecorder::IsPlayingBack() )
    {
        // really load include file
        if ( !st_StringEndsWith(file, ".cfg") || !Load( tDirectories::Var(), file ) )
        {
            if (!Load( tDirectories::Config(), file ) && error )
            {
                con << tOutput( "$config_include_not_found", file );
            }
        }
    }
    else
    {
        // just read configuration, and don't forget to reset the config level
        tCurrentAccessLevel levelResetter;
        tConfItemBase::LoadPlayback();
    }

    // mark section
    char const * section = "INCLUDE_END";
    tRecorder::Record( section );
    tRecorder::PlaybackStrict( section );

}

static void Include(std::istream& s )
{
    Include( s, true );
}

static void SInclude(std::istream& s )
{
    Include( s, false );
}

static tConfItemFunc s_Include("INCLUDE",  &Include);
static tConfItemFunc s_SInclude("SINCLUDE",  &SInclude);

// obsoleted settings that still are around in some distruted configuration files
static void st_Dummy(std::istream &s){tString rest; rest.ReadLine(s);}
static tConfItemFunc st_DummyMpHack("MOVIEPACK_HACK",&st_Dummy);

#ifdef DEDICATED
// settings missing in the dedicated server
static tConfItemFunc st_Dummy1("ARENA_WALL_SHADOW_NEAR", &st_Dummy);
static tConfItemFunc st_Dummy2("ARENA_WALL_SHADOW_DIST", &st_Dummy);
static tConfItemFunc st_Dummy3("ARENA_WALL_SHADOW_SIDEDIST", &st_Dummy);
static tConfItemFunc st_Dummy4("ARENA_WALL_SHADOW_SIZE", &st_Dummy);
static tConfItemFunc st_Dummy5("BUG_TRANSPARENCY_DEMAND", &st_Dummy);
static tConfItemFunc st_Dummy6("BUG_TRANSPARENCY", &st_Dummy);
static tConfItemFunc st_Dummy7("SHOW_OWN_NAME", &st_Dummy);
static tConfItemFunc st_Dummy8("FADEOUT_NAME_DELAY", &st_Dummy);
static tConfItemFunc st_Dummy9("FLOOR_MIRROR_INT", &st_Dummy);
#endif

namespace
{
std::vector<tConfigMigration::Callback> &st_MigrationCallbacks()
{
    static std::vector<tConfigMigration::Callback> callbacks;
    return callbacks;
}

#ifdef DEBUG
struct tConfigMigrationTester
{
    tConfigMigrationTester()
    {
        tASSERT(!tConfigMigration::SavedBefore("0.2.9","0.2.9"));

        auto AssertOrdered = [](char const *a, char const *b)
        {
            tASSERT(tConfigMigration::SavedBefore(a,b));
            tASSERT(!tConfigMigration::SavedBefore(b,a));
        };

        AssertOrdered("0.2.8","0.2.9");
        AssertOrdered("0.2.8","0.10.9");
        AssertOrdered("0.4.0_alpha100","0.4.0_beta1");
        AssertOrdered("0.4.0_beta229","0.4.0_rc1");
        AssertOrdered("0.4.0_rc19","0.4.0");
        AssertOrdered("0.","0.0");
        AssertOrdered("0.","0.1");
        AssertOrdered("0.","0.9");
        AssertOrdered("0","0.");
        AssertOrdered("0_","0");
    }
};

static tConfigMigrationTester st_ConfigMigrationTester;
#endif
}

tConfigMigration::tConfigMigration(tConfigMigration::Callback &&cb)
{
    st_MigrationCallbacks().push_back(std::move(cb));
}

void tConfigMigration::Migrate(const tString &savedInVersion)
{
    for(auto &callback: st_MigrationCallbacks())
    {
        callback(savedInVersion);
    }
}

bool tConfigMigration::SavedBefore(char const *savedInVersion, char const *beforeVersion)
{
    char const *pA = savedInVersion, *pB = beforeVersion;
    while(*pA && *pB)
    {
        // equality -> advance
        if(*pA == *pB)
        {
            ++pA;
            ++pB;
            continue;
        }

        // non-number difference -> regular alphabetic order
        if(!isdigit(*pA) || !isdigit(*pB))
        {
            return *pA < *pB;
        }

        // this leaves number differences.
        auto lengthOfNumber = [](char const *pBegin)
        {
            auto *pC = pBegin;
            while(isdigit(*pC))
                ++pC;
            return pC - pBegin;
        };

        // longer numbers are bigger
        auto const lA = lengthOfNumber(pA);
        auto const lB = lengthOfNumber(pB);
        if(lA < lB)
            return true;
        if(lA > lB)
            return false;

        // equal length numbers can be compared digit by digit
        while(isdigit(*pA))
        {
            if(*pA == *pB)
            {
                ++pA;
                ++pB;
                continue;
            }

            return *pA < *pB;
        }
    }
    if(!*pA && !*pB)
        return false;

    // reversed order if one version string is longer than the other;
    // we want _ to be less than nothing, but . more, and they sit on
    // opposite sides of the digits.
    if(!*pA)
    {
        return '9' >= *pB;
    }
    else
    {
        tASSERT(!*pB);
        return '9' < *pA;
    }
}


void sg_setDefaultValue(std::istream &s)
{
    // String user passes with commands.
    tString passedString;
    passedString.ReadLine(s);

    tToUpper(passedString);

    if (passedString.Filter() == "")
    {
        con << tOutput( "$config_command_unknown","");
        return;
    }

    #ifndef DEDICATED
    tCurrentAccessLevel level(tAccessLevel_Owner, true);
    #endif

    tArray<tString> passedCommands = passedString.Split(" ");
    if (passedCommands.Len() > 0)
    {
        for (int i = 0; i < passedCommands.Len(); i++)
        {
            bool foundCommand = false;
            tConfItemBase::tConfItemMap confmap = tConfItemBase::ConfItemMap();
            tConfItemBase::tConfItemMap::iterator iter = confmap.find( passedCommands[i] );
            if ( iter != confmap.end() )
            {
                tConfItemBase *ci = iter->second;
                {
                    // check if the item has a default value
                    if (!ci->IsDefault())
                    {
                        ci->SetDefault();
                        break;
                    }
                    foundCommand = true;
                }
            }

            if (!foundCommand)
            {
                con << tOutput( "$config_command_unknown", passedCommands[i] );
            }
        }
    }
}

static tConfItemFunc sg_setDefaultValueConf("SET_DEFAULT", &sg_setDefaultValue);


void sg_listChangedCommands(std::istream &s)
{

    tConfItemBase::tConfItemMap confmap = tConfItemBase::ConfItemMap();
    for( tConfItemBase::tConfItemMap::iterator iter = confmap.begin(); iter != confmap.end() ; iter++)
    {
        if (!iter->second->IsDefault()){
            iter->second->displayChangedMsg();
        }
    }
}

static tConfItemFunc sg_listChangedCommandsConf("LIST_CHANGED_COMMANDS", &sg_listChangedCommands);


void sg_loadLine(std::istream &s)
{
    tString fullInput;
    fullInput.ReadLine(s);

    if (fullInput.Filter() == "")
    {
        con << tOutput("$config_command_unknown", "");
        return;
    }

    #ifndef DEDICATED
    tCurrentAccessLevel level(tAccessLevel_Owner, true);
    #endif

    tArray<tString> inputLines = fullInput.Split("\\n");

    if (inputLines.Len() > 0)
    {
        for (int i = 0; i < inputLines.Len(); i++)
        {
            std::stringstream currentLineStream(static_cast<char const *>(inputLines[i]));
            tConfItemBase::LoadAll(currentLineStream);
        }
    }
}

static tConfItemFunc sg_loadLineConf("LOAD_LINE", &sg_loadLine);


bool sn_unlocknSettings=false;
static tConfItem<bool> sn_unlocknSettingsConf = HelperCommand::tConfItem("NETWORK_SETTINGS_ALLOW_CHANGE",sn_unlocknSettings);

void TempConfItemManager::DeleteConfitems()
{
    while (CurrentConfitem > 0)
    {
        CurrentConfitem--;
        tDESTROY(configuration[CurrentConfitem]);
    }
}

void TempConfItemManager::DeleteConfitem(const tString& command)
{
    for (int i = 0; i < CurrentConfitem; i++)
    {
        if (configuration[i]->GetTitle() == command)
        {
            tDESTROY(configuration[i]);

            for (int j = i; j < CurrentConfitem - 1; j++)
            {
                configuration[j] = configuration[j + 1];
            }
            CurrentConfitem--;
            return;
        }
    }
}


tString se_symLinkedCommands("");
static tConfItem<tString> se_symLinkedCommandsConf("SYM_LINKED_COMMANDS_STORED_COMMAND_STR", se_symLinkedCommands);
static TempConfItemManager *symLinkedConfItems = NULL;
std::unordered_map<tString, tString> symLinkedCommands;

bool SymLinkedCommandRunnerCore(tString commandStr)
{
    if (symLinkedCommands.find(commandStr) != symLinkedCommands.end())
    {
        tString action = symLinkedCommands[commandStr];

        tCurrentAccessLevel level(tAccessLevel_Owner, true);
        std::stringstream s(action.stdString());
        tConfItemBase::LoadAll(s);
        return true;
    }
    return false;
}

void SymLinkedCommandRunner(std::istream& input)
{
    tString commandStr;
    commandStr.ReadLine(input, true);
    SymLinkedCommandRunnerCore(commandStr);
}

bool SymLinkedCommandRunnertStr(tString &input)
{
    return SymLinkedCommandRunnerCore(input);
}

void AddSymLinkedCommand(const tString& originalCommand, const tString& action)
{
    tString command(originalCommand.ToUpper());

    if(symLinkedCommands.find(command) != symLinkedCommands.end())
    {
        con << "Command " << command << " already exists. Updating the action...\n";

        tArray<tString> entries = se_symLinkedCommands.Split(";");
        se_symLinkedCommands.Clear();
        for (int i = 0; i < entries.Len(); ++i)
        {
            tArray<tString> entry = entries[i].Split(",");
            if (entry.Len() == 2)
            {
                tString storedCommand = entry[0].TrimWhitespace().ToUpper();
                if (storedCommand != command)
                {
                    if (!se_symLinkedCommands.empty())
                        se_symLinkedCommands += ";";
                    se_symLinkedCommands << entries[i];
                }
            }
        }

        if (symLinkedConfItems)
            symLinkedConfItems->DeleteConfitem(tString(command));
    }
    else if (!se_symLinkedCommands.empty())
    {
        se_symLinkedCommands += ";";
    }

    symLinkedCommands[command] = action;
    tString replacedAct(action);
    tString processedAction = replacedAct.Replace("\n", "\\\n");
    se_symLinkedCommands << command << "," << processedAction;

    if (symLinkedConfItems == nullptr)
    {
        symLinkedConfItems = new TempConfItemManager();
    }
    symLinkedConfItems->StoreConfitem(new tConfItemFunc(tString(command), &SymLinkedCommandRunner));
}


void RemoveSymLinkedCommand(const tString& command)
{
    tString upperCommand(command.ToUpper());

    symLinkedCommands.erase(upperCommand);

    tArray<tString> entries = se_symLinkedCommands.Split(";");
    se_symLinkedCommands.Clear();
    for (int i = 0; i < entries.Len(); ++i)
    {
        tArray<tString> entry = entries[i].Split(",");
        if (entry.Len() == 2)
        {
            tString storedCommand = entry[0].TrimWhitespace().ToUpper();
            if (storedCommand != upperCommand)
            {
                if (!se_symLinkedCommands.empty())
                    se_symLinkedCommands += ";";
                se_symLinkedCommands << entries[i];
            }
        }
    }

    if (symLinkedConfItems)
        symLinkedConfItems->DeleteConfitem(tString(upperCommand));
}

void ClearSymLinkedCommands()
{
    symLinkedCommands.clear();
    se_symLinkedCommands.Clear();
    symLinkedConfItems->DeleteConfitems();
}
static void SymLinkCommandManager(std::istream &s)
{
    tString params;
    params.ReadLine(s, true);

    if (params.empty())
    {
        con << "Usage for adding symbolic link: SYM_LINK_MANAGE ADD <commandName> <command to be executed>\n";
        con << "Usage for removing symbolic link: SYM_LINK_MANAGE REMOVE <commandName>\n";
        con << "Usage for listing all symbolic links: SYM_LINK_MANAGE LIST\n";

        con << "List of symbolic links:\n";
        for (const auto& pair : symLinkedCommands)
        {
            con << pair.first << " -> " << pair.second << "\n";
        }
        return;
    }

    int pos = 0;
    tString action = params.ExtractNonBlankSubString(pos).TrimWhitespace();

    if (action.ToLower() == "add")
    {
        tString commandName = params.ExtractNonBlankSubString(pos).TrimWhitespace().ToUpper();
        tString commandToExecute = params.SubStr(pos).TrimWhitespace();

        if (commandName.empty() || commandToExecute.empty())
        {
            con << "For adding a symbolic link, please provide both commandName and the command to be executed.\n";
            return;
        }

        AddSymLinkedCommand(commandName, commandToExecute);
        con << "Added symbolic link: '" << commandName << "' -> '" << commandToExecute.Replace("\n","\\n") << "'\n";
    }
    else if (action.ToLower() == "remove")
    {
        tString commandName = params.ExtractNonBlankSubString(pos).TrimWhitespace().ToUpper();

        if (symLinkedCommands.find(commandName) == symLinkedCommands.end())
        {
            con << "No symbolic link found for: '" << commandName << "'\n";
            return;
        }
        RemoveSymLinkedCommand(commandName);
        con << "Removed symbolic link for: '" << commandName << "'\n";
    }
    else if (action.ToLower() == "list")
    {
        if (symLinkedCommands.empty())
        {
            con << "No symbolic links found.\n";
            return;
        }

        con << "List of symbolic links:\n";
        for (const auto& pair : symLinkedCommands)
        {
            tString thisAction(pair.second);
            thisAction = thisAction.Replace("\n","\\n");
            con << pair.first << " -> " << thisAction << "\n";
        }
    }
    else if (action.ToLower() == "clear")
    {
        ClearSymLinkedCommands();
        con << "Cleared all symbolic links.\n";
    }
    else
    {
        con << "Unknown action: '" << action << "'. Valid actions are ADD, REMOVE, CLEAR, LIST.\n";
    }
}
static tConfItemFunc SymLinkCommandManagerConf("SYM_LINK_MANAGE", &SymLinkCommandManager);

void SymLinkedCommandsLoader()
{
    if (symLinkedConfItems == nullptr)
        symLinkedConfItems = new TempConfItemManager();

    if (!symLinkedConfItems)
        return;

    symLinkedConfItems->DeleteConfitems();

    if (se_symLinkedCommands.empty())
        return;

    tArray<tString> symLinkedEntries = se_symLinkedCommands.Split(";");

    for (int i = 0; i < symLinkedEntries.Len(); ++i)
    {
        tArray<tString> entry = symLinkedEntries[i].Split(",");
        if (entry.Len() == 2)
        {
            tString action = entry[1].TrimWhitespace();
            tString command = entry[0].TrimWhitespace().ToUpper();

            if(symLinkedCommands.find(command) == symLinkedCommands.end())
            {
                symLinkedCommands[command] = action;

                symLinkedConfItems->StoreConfitem(new tConfItemFunc(tString(command), &SymLinkedCommandRunner));
            }
            else
            {
                con << "DUPLICATE COMMAND AT INDEX " << i << ": " << command << ". Skipping...\n";
            }
        }
        else
        {
            con << "MALFORMED INDEX AT " << i << ": " << symLinkedEntries[i] << "\n";
        }
    }
}

