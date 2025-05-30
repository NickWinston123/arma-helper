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

#ifndef ArmageTron_CONFIGURATION_H
#define ArmageTron_CONFIGURATION_H

#include "tList.h"
#include "tString.h"
#include "tLinkedList.h"
#include "tException.h"
#include "tLocale.h"
#include "tConsole.h"
#include "tLocale.h"
#include "../network/nNetObject.h"
#include <iostream>
#include <ctype.h>
#include <string>
#include <map>
#include <deque>
#include <functional>
#include <unordered_map>

// Define this to disable compiling the new interface
#define NEW_CONFIGURATION_NO_COMPILE

/***********************************************************************
 * The new Configuration interface, currently not completely implemented
 */

#ifndef NEW_CONFIGURATION_NO_COMPILE

// This class stores a single configuration item
class tConfigurationItem {
public:
    // Constructors for each data type supported
    tConfigurationItem(string newValue);
    tConfigurationItem(int newValue);
    tConfigurationItem(double newValue);
    tConfigurationItem(bool newValue);

    // Sets the source of this directive.  Possible values are:
    //   GAME = set during the game
    //   $path = path to the file it came from
    //   DEFAULT = default value, set by the object that registered it
    void setSource(string source);

    // Sets the destination of this directive.  Optional, defaults to "NONE"
    // Possible values are:
    //   NONE = throw it away when the game shuts down
    //   $path = save it to $path
    void setDestination(string dest);

    // Setters for each type of data supported
    void setValue(string newValue);
    void setValue(int newValue);
    void setValue(double newValue);
    void setValue(bool newValue);
private:
    // _value is the authoritative value of the directive
    string _value;

    string _source;
    string _destination;

    int _intCache;
    double _doubleCache;
    bool _boolCache;
}

// This class is the workhorse configuration class.  Use it and only it
class tConfiguration {
public:
    // Use this function to get the instance of the configuration object
    //   If it doesn't exist, it will be created
    static const tConfiguration* GetConfiguration();

    // Use this function to register your own configuration directive
    //   returns true if it was successful, false if not
    //   should return true even if it didn't register the new directive because
    //       it was already there
    bool registerDirective(string newDirective, string defValue);

    // Use this function to set a configuration directive from a string
    bool setDirective(string oldDirective, string newValue);

    // Use this function to set a configuration directive from an int
    bool setDirective(string oldDirective, int newValue);

    // Use this function to set a configuration directive from a float
    bool setDirective(string oldDirective, double newValue);

    // Use this function to set a configuration directive from a bool
    bool setDirective(string oldDirective, bool newValue);

    // Use this function to get a configuration directive as a string
    const string& getDirective(string oldDirective);

    // Use this function to get a configuration directive as an int
    const int& getDirectiveI(string oldDirective);

    // Use this function to get a configuration directive as a double
    const double& getDirectiveF(string oldDirective);

    // Use this function to get a configuration directive as a bool
    const bool& getDirectiveB(string oldDirective);

    // Use this function to load a file
    //  You *MUST* pass it a complete path from the root directory!
    //  Returns TRUE if it can open the file and load at least some of it
    //  Returns FALSE if complete failure.
    bool LoadFile(string filename);

    // Use this function to save current configuration to files
    //   Returns TRUE on success, FALSE on failure
    bool SaveFile();

private:
    static tConfiguration* _instance;

    // This function is used internally to actually set each directive
    bool _setDirective(string oldDirective, tConfigurationItem& newItem);

    // This function registers basic configuration directives
    //  Create a new configuration directive by going to this function and
    //  putting the appropriate line, then just use it in the game
    //  It registers global defaults, but not object-specific defaults
    void _registerDefaults();
}

#endif
/*
 * The old stuff follows
 ************************************************************************/

//! access levels for admin interfaces; lower numeric values are better
enum tAccessLevel
{
    tAccessLevel_Hoster = -1,      // the server hoster
    tAccessLevel_Owner = 0,        // the server owner
    tAccessLevel_Admin = 1,        // one of his admins
    tAccessLevel_Moderator = 2,    // one of the moderators
    tAccessLevel_3 = 3,            // reserved
    tAccessLevel_4 = 4,            // reserved
    tAccessLevel_Armatrator = 5,   // reserved
    tAccessLevel_6 = 6,            // reserved
    tAccessLevel_TeamLeader = 7,   // a team leader
    tAccessLevel_TeamMember = 8,   // a team member
    tAccessLevel_9 = 9,            // reserved
    tAccessLevel_10 = 10,          // reserved
    tAccessLevel_11 = 11,          // reserved
    tAccessLevel_Local      = 12,  // user with a local account
    tAccessLevel_13 = 13,          // reserved
    tAccessLevel_14 = 14,          // reserved
    tAccessLevel_Remote = 15,      // user with remote account
    tAccessLevel_DefaultAuthenticated = 15,     // default access level for authenticated users
    tAccessLevel_FallenFromGrace = 16,          // authenticated, but not liked
    tAccessLevel_Shunned = 17,          // authenticated, but disliked
    tAccessLevel_18 = 18,          // reserved
    tAccessLevel_Authenticated = 19,// any authenticated player
    tAccessLevel_Program = 20,     // a regular player
    tAccessLevel_21 = 21,          // reserved
    tAccessLevel_22 = 22,          // reserved
    tAccessLevel_23 = 23,          // reserved
    tAccessLevel_24 = 24,          // reserved
    tAccessLevel_25 = 25,          // reserved
    tAccessLevel_Invalid = 255,    // completely invalid level
    tAccessLevel_Default = 20,     // default (program)
    tAccessLevel_Punished = 21     // punished
};

//! class to temporarily allow/forbid the use of casacl
class tCasaclPreventer
{
public:
    tCasaclPreventer( bool prevent = true );
    ~tCasaclPreventer();

    static bool InRInclude(); //!< returns whether we're currently in an RINCLUDE file
private:
    bool previous_; //!< previous value of prevention flag
};

//! class managing the current access level
class tCurrentAccessLevel
{
    friend class tCasacl;
public:
    //! for the lifetime of this object, change the user's admit level to the passed one.
    tCurrentAccessLevel( tAccessLevel newLevel, bool allowElevation = false );

    //! does not change the access level on construction, but resets it on destruction
    tCurrentAccessLevel();
    ~tCurrentAccessLevel();

    //! returns the current access level
    static tAccessLevel GetAccessLevel();

    //! returns the name of an access level
    static tString GetName( tAccessLevel level );
private:
    tCurrentAccessLevel( tCurrentAccessLevel const & );
    tCurrentAccessLevel & operator = ( tCurrentAccessLevel const & );

    tAccessLevel lastLevel_; //!< used to restore the last admin level when the object goes out of scope
public:
    static tAccessLevel currentLevel_; //!< the current access level
};

class tConfItemBase
{
    friend class tCheckedPTRBase;
    friend class tConfItemLevel;
    friend class tAccessLevelSetter;

    int id;
protected:
    const tString title;
    const tOutput help;
    //  class tString value;
    bool changed;

    std::vector<std::pair<std::string, std::string>> valueMap;

    tAccessLevel requiredLevel; //!< access level required to change this setting
    tAccessLevel setLevel;      //!< access level of the user making the last change to this setting

public:
    // the map of all configuration items
    typedef std::map< tString, tConfItemBase * > tConfItemMap;
    static tConfItemMap const & GetConfItemMap();

    static tConfItemMap & ConfItemMap();
public:

    static tString lastLoadOutput;
    static tString lastLoadCommandName;
    static bool applyValueToMatchedConfigs(const std::string& pattern, tConfItemBase::tConfItemMap& confmap, const std::string& valueStr);
    // static tConfItemBase* s_ConfItemAnchor;
    //static tConfItemBase* Anchor(){return dynamic_cast<tConfItemBase *>(s_ConfItemAnchor);}
    static bool printChange; //!< if set, setting changes are printed to the console and, if printErrors is set as well, suggestions of typo fixes are given.
    static bool printErrors; //!< if set, unknown settings are pointed out.

    tConfItemBase(const char *title, const tOutput& help, const std::vector<std::pair<std::string, std::string>>& initValueMap = {});
    tConfItemBase(const char *title, const std::vector<std::pair<std::string, std::string>>& initValueMap = {});

    virtual ~tConfItemBase();
    const std::vector<std::pair<std::string, std::string>>& GetValueMap() const 
    {
        return valueMap;
    }

    std::string GetValueMapValueForKey(const std::string& key) const 
    {
        for (const auto& pair : valueMap) 
        {
            if (pair.first == key) 
                return pair.second;
        }
        return "";
    }

    tString getFormatedValueMapValue(tString key)
    {
        return getFormatedValueMapValue(key.stdString());
    }

    tString getFormatedValueMapValue(std::string key)
    {
        tString val;
        val << key;

        tString keyValue;
        keyValue << GetValueMapValueForKey(key);
        if (!keyValue.empty())
        {
            val << " ("
                << keyValue
                << ")";
        }

        return val;
    }

    void PrintValueMap(std::string currentVal = "") const 
    {
        tString printedMap;
        for (const auto& pair : valueMap) 
        {
            if (!currentVal.empty() && pair.first == currentVal) 
                printedMap << "* " << pair.first << ": " << pair.second << "\n";
            else
                printedMap << pair.first << ": " << pair.second << "\n";
        }

        con << printedMap;
    }

    void SetValueMap(const std::vector<std::pair<std::string, std::string>>& newValueMap) 
    {
        valueMap = newValueMap;
    }

    tString const & GetTitle() const {
        return title;
    }

    tAccessLevel GetRequiredLevel() const { return requiredLevel; }
    tAccessLevel GetSetLevel() const { return setLevel; }
    void         SetSetLevel( tAccessLevel level ) { setLevel = level; }
    static int EatWhitespace(std::istream &s); // eat whitespace from stream; return: first non-whitespace char

    static void LoadAllFromMenu(std::istream &s);
    static void ExportAll();
    static void SaveAll(std::ostream &s, bool all=false);
    static void LoadAll(std::istream &s, bool record = false );  //! loads configuration from stream
    static void LoadAll(std::ifstream &s, bool record = false );  //! loads configuration from file
    static void LoadLine(std::istream &s, bool wildCardEnabled = false); //! loads one configuration line
    static bool LoadPlayback( bool print = false ); //! loads configuration from playback
    static void DocAll(std::ostream &s);
    static void DocAll();
    static int AccessLevel(std::istream &s); //! Returns access level needed for command. -1 if command not found.
    static void WriteAllToFile();
    static void WriteAllLevelsToFile();
    static void WriteChangedToFile();
    static std::deque<tString> GetCommands(void);
    static tString FindConfigItem(tString name,int pos = 0);    //! Returns the config name of the searching string name
    static tConfItemBase *GetConfigItem(tString const &name);
    static tConfItemBase *GetConfigItem(std::string const &name);
    static void SetAllAccessLevel(int newLevel);
    static void FindConfigItems(const tString &word, std::deque<tString> &results);
    static void DownloadSettings_Go(nMessage &m);
    static void DownloadConfig_Go(nMessage &m);
    static void DownloadSettings_To(int peer);
    static void DownloadConfig_To(tString file, int peer);
    static tString getLastLoadOutput(){ return lastLoadOutput;};
    // helper functions for files (use these, they manage recording and playback properly)
    enum SearchPath
    {
        Config = 1,
        Var    = 2,
        All    = 3
    };

    static bool OpenFile( std::ifstream & s, tString const & filename, SearchPath path ); //! opens a file stream for configuration reading
    static void ReadFile( std::ifstream & s ); //! loads configuration from a file

    virtual bool allowedChange() { return true; }

    virtual bool IsDefault() { return true; };
    virtual void displayChangedMsg() { con << tString("BASE ?"); }
    virtual tString getValue() { return tString("BASE ?"); }
    virtual void SetDefault() {};
    virtual void ReadVal(std::istream &s)=0;
    virtual void WriteVal(std::ostream &s)=0;
    virtual void FetchVal(tString &val)=0;

    virtual void WasChanged(){} // what to do if a read changed the thing

    virtual bool Writable(){
        return true;
    }

    // should this be saved into user.cfg?
    virtual bool Save(){
        return true;
    }

    // CAN this be saved at all?
    virtual bool CanSave(){
        return true;
    }
};

//! just to do some work in static initializers, to modify default access levels:
class tAccessLevelSetter
{
public:
    //! modifies the access level of <item> to <level>
    tAccessLevelSetter( tConfItemBase & item, tAccessLevel level );
};

// Arg! Msvc++ could not handle bool IO. Seems to be fine now.
#ifdef _MSC_VER_XXX
inline std::istream & operator >> (std::istream &s,bool &b){
    int x;
    s >> x;
    b=(x!=0);
    return s;
}

inline std::ostream & operator << (std::ostream &s,bool b){
    if (b)
        return s << 1;
    else
        return s << 0;
}
#endif

//! type modifying class mapping types in memory to types to stream
template< class T > struct tTypeToConfig
{
    typedef T TOSTREAM;             //!< type to put into the stream
    typedef int DUMMYREQUIRED;      //!< change this type to "int *" to indicate the conversion is required
};

//! macro declaring that type TYPE should be converted to type STREAM before
// recording (and back after playback) by specializing the tTypeToStream class template
#define tCONFIG_AS( TYPE, STREAM ) \
template<> struct tTypeToConfig< TYPE > \
{ \
    typedef STREAM TOSTREAM; \
  typedef int * DUMMYREQUIRED; \
} \

//! macro for configuration enums: convert them to int.
#define tCONFIG_ENUM( TYPE ) tCONFIG_AS( TYPE, int )

tCONFIG_ENUM( tAccessLevel );

//! exception to be thrown when the current script should be aborted
class tAbortLoading: public tException
{
public:
    tAbortLoading( char const * command );
private:
    tString command_; //!< the command responsible for the abort

    virtual tString DoGetName() const;
    virtual tString DoGetDescription() const;
};

template<class T> class tConfItem:virtual public tConfItemBase{
public:
    typedef bool (*ShouldChangeFuncT)(T const &newValue);
protected:
    T    *target;
    T    defaultValue;
    ShouldChangeFuncT shouldChangeFunc_;

    tConfItem(T &t):tConfItemBase(""),target(&t), shouldChangeFunc_(NULL), defaultValue(t) {}
public:
    tConfItem(const char *title,const tOutput& help,T& t, const std::vector<std::pair<std::string, std::string>>& initValueMap = {})
            :tConfItemBase(title,help, initValueMap),target(&t), shouldChangeFunc_(NULL), defaultValue(t) {
            }

    tConfItem(const char *title,T& t,const std::vector<std::pair<std::string, std::string>>& initValueMap = {})
            :tConfItemBase(title, initValueMap),target(&t), shouldChangeFunc_(NULL), defaultValue(t) {
            }

    tConfItem(const char*title, T& t, ShouldChangeFuncT changeFunc, const std::vector<std::pair<std::string, std::string>>& initValueMap = {})
            :tConfItemBase(title, initValueMap),target(&t),shouldChangeFunc_(changeFunc), defaultValue(t) {
            }

    virtual ~tConfItem(){}


    virtual bool allowedChange()
    {
        if (!shouldChangeFunc_)
            return true;


        T dummy( *target );

        if (shouldChangeFunc_(dummy)) {
            return true;
        }


        return false;
    }

    tConfItem<T> & SetShouldChangeFunc( ShouldChangeFuncT changeFunc )
    {
        this->shouldChangeFunc_ = changeFunc;
        return *this;
    }

    virtual void displayChangedMsg()
    {
        tOutput o;
        o.SetTemplateParameter(1, title);
        o.SetTemplateParameter(2, *target);
        o.SetTemplateParameter(3, defaultValue);
        o << "$config_value_was_changed";
        con << o;
    }

    virtual tString getValue()
    {
        tString val;
        val << *target;
        return val;
    }

    virtual bool IsDefault() { return *target == defaultValue; }

    virtual void SetDefault()
    {
        tOutput o;
        o.SetTemplateParameter(1, title);
        o.SetTemplateParameter(2, *target);
        o.SetTemplateParameter(3, defaultValue);
        o << "$config_value_changed";
        con << o;

        *target = defaultValue;
    }

    typedef typename tTypeToConfig< T >::DUMMYREQUIRED DUMMYREQUIRED;

    // read without conversion
    static void DoRead(std::istream &s, T & value, int )
    {
        s >> value;
    }

    // read with conversion
    static void DoRead(std::istream &s, T & value, int * )
    {
        typename tTypeToConfig< T >::TOSTREAM dummy;
        s >> dummy;
        value = static_cast< T >( dummy );
    }

    // write without conversion
    static void DoWrite(std::ostream &s, T const & value, int )
    {
        s << value;
    }

    // write with conversion
    static void DoWrite(std::ostream &s, T const & value, int * )
    {
        s << static_cast< typename tTypeToConfig< T >::TOSTREAM >( value );
    }

    void SetVal( T const & val )
    {
        if (!shouldChangeFunc_ || shouldChangeFunc_( val ))
        {
            if (printChange)
            {
                tOutput o;
                o.SetTemplateParameter(1, title);
                o.SetTemplateParameter(2, *target);
                o.SetTemplateParameter(3, val);
                o << "$config_value_changed";
                con << o;
            }

            *target = val;
            changed = true;
        }
        else
        {
            con << tOutput("$config_value_not_changed", title, *target, val);
        }
    }

    const T *GetTarget() const
    {
        return target;
    }

    virtual void ReadVal(std::istream &s){
        // eat whitepsace
        int c= EatWhitespace(s);
        //con << *target << "\n";
        T dummy( *target );
        if (c!='\n' && s && !s.eof() && s.good()){
            DoRead( s, dummy, DUMMYREQUIRED() );
            if (!s.good() && !s.eof() )
            {
                tOutput o;
                o.SetTemplateParameter(1, title);
                o << "$config_error_read";
                con << o;

                if (!GetValueMap().empty()) 
                {
                    con << "Recommended values from value map: \n";
                    PrintValueMap(tString::ConvertToTString(*target).stdString());
                }

                tConfItemBase::lastLoadOutput << o;
            }
            else
                if (dummy!=*target){
                    if (!Writable())
                    {
                        tOutput o;
                        o.SetTemplateParameter(1, title);
                        o << "$nconfig_error_protected";
                        con << "";
                        tConfItemBase::lastLoadOutput << o;
                    }
                    else{
                        if (!shouldChangeFunc_ || shouldChangeFunc_(dummy))
                        {
                            if (printChange)
                            {
                                tOutput o;
                                o.SetTemplateParameter(1, title);
                                o.SetTemplateParameter(2,  getFormatedValueMapValue(tString::ConvertToTString(*target)));
                                o.SetTemplateParameter(3,  getFormatedValueMapValue(tString::ConvertToTString(dummy)));
                                o << "$config_value_changed";
                                con << o;
                                tConfItemBase::lastLoadOutput << o;
                            }

                            *target = dummy;
                            changed = true;
                        }
                        else
                        {
                            con << tOutput("$config_value_not_changed", title, *target, dummy);
                            tConfItemBase::lastLoadOutput << tOutput("$config_value_not_changed", title, *target, dummy);
                        }
                    }
                }
        }
        else
        {
            tOutput o;
            o.SetTemplateParameter(1, title);
            o.SetTemplateParameter(2, getFormatedValueMapValue(tString::ConvertToTString(*target)));
            o << "$config_message_info";
            con << o;
            tConfItemBase::lastLoadOutput << o;
        }

        // read the rest of the line
        c=' ';
        while (c!='\n' && s.good() && !s.eof()) c=s.get();
    }

    virtual void WriteVal(std::ostream &s){
        DoWrite( s, *target, DUMMYREQUIRED() );
    }

    virtual void FetchVal(tString &val)
    {
        val << *target;
    }
};

template<class T> class tSettingItem:public tConfItem<T>{
public:
    //  tSettingItem(const char *title,const tOutput& help,T& t)
    //    :tConfItemBase(title,help),tConfItem<T>(t){}

    tSettingItem(const char *title,T& t)
            :tConfItemBase(title),tConfItem<T>(title, t){}

    tSettingItem(const char *title, T& t, typename tConfItem<T>::ShouldChangeFuncT changeFunc)
            :tConfItemBase(title), tConfItem<T>(title, t, changeFunc) {}

    virtual ~tSettingItem(){}

    virtual bool Save(){
        return false;
    }
};


/*
#include <functional>

template<class T>
class tConfItemWithCommand:public tConfItem<T> {
public:
    typedef std::function<bool(ePlayerNetID*, tString)> CommandFunc;

    CommandFunc command;

    tConfItemWithCommand(const char *title, T& t, CommandFunc cmd)
        : tConfItem<T>(title, t), command(cmd) {}

    virtual ~tConfItemWithCommand(){}

    virtual bool Save() {
        return false;
    }
};
*/

class tConfItemLine:public tConfItem<tString>, virtual public tConfItemBase{
private:
    tString defaultValue;
public:
    tConfItemLine(const char *title,const char *help,tString &s)//, const tString &defaultVal)
            :tConfItemBase(title,help),tConfItem<tString>(title,help,s),defaultValue(s){}

    virtual ~tConfItemLine(){}

    tConfItemLine(const char *title, tString &s)//, const tString &defaultVal)
            :tConfItemBase(title),tConfItem<tString>(title,s),defaultValue(s){}

    virtual void ReadVal(std::istream &s);
    virtual void WriteVal(std::ostream &s);
    virtual bool IsDefault() { return *target == defaultValue; }

    virtual void displayChangedMsg()
    {
        tOutput o;
        o.SetTemplateParameter(1, title);
        o.SetTemplateParameter(2, *target);
        o.SetTemplateParameter(3, defaultValue);
        o << "$config_value_was_changed";
        con << o;
    }

    virtual tString getValue()
    {
        tString val;
        val << *target;
        return val;
    }

    virtual void SetDefault()
    {
        tOutput o;
        o.SetTemplateParameter(1, title);
        o.SetTemplateParameter(2, *target);
        o.SetTemplateParameter(3, defaultValue);
        o << "$config_value_changed";
        con << o;
        *target = defaultValue;
    }
};

class tSettingItemLine:public tConfItemLine{
public:
    tSettingItemLine(const char *title,const char *help,tString &s)
            :tConfItemBase(title,help),tConfItemLine(title,help,s){}

    tSettingItemLine(const char *title, tString &s)
            :tConfItemBase(title),tConfItemLine(title,s){}

    virtual ~tSettingItemLine() = default;

    bool Save() override{
        return false;
    }
};

typedef void CONF_FUNC(std::istream &s);

class tConfItemFunc:public tConfItemBase{
    CONF_FUNC *f;
public:
    typedef bool (*ShouldChangeFuncT)();
    ShouldChangeFuncT shouldChangeFunc_;

    tConfItemFunc(const char *title, CONF_FUNC *func);
    tConfItemFunc(const char *title, CONF_FUNC *func, ShouldChangeFuncT changeFunc )
    :tConfItemBase(title),f(func), shouldChangeFunc_(changeFunc){}

    virtual bool allowedChange()
    {
        return shouldChangeFunc_ == NULL || shouldChangeFunc_();
    }

    virtual ~tConfItemFunc();

    virtual void ReadVal(std::istream & s);
    virtual void WriteVal(std::ostream & s);
    virtual void FetchVal(tString & val);

    virtual tString getValue()
    {
        tString val;
        val << "Function";
        return val;
    }

    virtual bool Save();

    virtual bool CanSave()
    {
        return false;
    }
};

// includes a single configuration file by name, searches in var and config directories
void st_Include( tString const & file );

void st_LoadConfig();
void st_SaveConfig();
void st_LoadUserConfig();
void st_SaveChangedConfig();

struct tConfigMigration
{
    using Callback = std::function<void(tString const &)>;

    // registers a config migration callback
    explicit tConfigMigration(Callback &&cb);

    // calls all migration callbacks
    static void Migrate(tString const &savedInVersion);

    // alphanumerically compares version strings
    static bool SavedBefore(char const *savedInVersion, char const *beforeVersion);
};

extern bool st_FirstUse;

extern tString st_AddToUserExt(tArray<tString> commands);

#define CONFITEMS_STORED_SIZE 150
class TempConfItemManager
{
    private:
    tConfItemBase *configuration[CONFITEMS_STORED_SIZE];
    int            CurrentConfitem;

    public:
    TempConfItemManager() {
        CurrentConfitem = 0;
    }

    ~TempConfItemManager() {
        DeleteConfitems();
    }

    void StoreConfitem(tConfItemBase *c)
    {
        tASSERT(CurrentConfitem < CONFITEMS_STORED_SIZE);
        configuration[CurrentConfitem++] = c;
    }

    void DeleteConfitem(const tString& command);
    void DeleteConfitems();

};

void CommandShortcutLoader();
bool CommandShortcutRunnerStr(tString &input);
void CommandShortcutRunner(std::istream& input);


extern tString se_variableStorageFile;
struct VariableEntry {
    tString type;
    tString defaultValue;
};

extern std::unordered_map<tString, VariableEntry> se_variables;

extern TempConfItemManager *variableConfItems;

void VariableCreatorAddVariable(const tString &type, const tString &varName, const tString &defaultValue);
bool SaveVariableCreatorVariablesToFile();
void VariableCreatorLoader();
void RemoveVariableCreatorVariable(const tString &varName);
void ListVariableCreatorVariables();
void ClearVariableCreatorVariables();
void VariableCreatorCommand(std::istream &s);

#endif

