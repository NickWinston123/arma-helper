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

#include "rSDL.h"

#include "gServerFavorites.h"

#include "gLogo.h"
#include "gGame.h"

#include "uMenu.h"

#include "tConfiguration.h"
#include "tDirectories.h"

#include "gServerBrowser.h"
#include "nServerInfo.h"

#ifdef CONNECTION_STRESS
static bool sg_ConnectionStress = false;
#endif

#include <sstream>

enum
{
    NUM_FAVORITES = 100
};

//! favorite server information, just to connect
/*
class gServerInfoFavorite: public nServerInfoBase
{
public:
    // construct a server directly with connection name and port
    gServerInfoFavorite( tString const & connectionName, unsigned int port )
    {
        nServerInfoBase::SetConnectionName( connectionName );
        nServerInfoBase::SetPort( port );
    };
};
*/
typedef nServerInfoRedirect gServerInfoFavorite;

static tString sg_ConfName(int ID, char const *prefix, char const *name)
{
    std::stringstream s;
    s << "BOOKMARK_" << prefix << ID + 1 << name;

    return tString(s.str().c_str());
}


//! server favorite: holds connection info and configuration items
class gServerFavorite
{
public:
    friend class gServerFavoritesHolder;

    //! constructor
    gServerFavorite(int ID, char const *prefix)
        : name_(""),
          port_(sn_defaultPort),
          index_(ID),
          confName_(sg_ConfName(ID, prefix, "_NAME"), name_),
          confAddress_(sg_ConfName(ID, prefix, "_ADDRESS"), address_),
          confPort(sg_ConfName(ID, prefix, "_PORT"), port_),
          server_(nullptr)
    {
    }

    //! connects to the server
    void Connect()
    {
        gServerInfoFavorite fav(address_, port_);

        gLogo::SetDisplayed(false);

        ConnectToServer(&fav);
    }

    //! returns the index in the favorite holder
    int GetIndex()
    {
        return index_;
    }

    void PrepareServerQuery()
    {
        if (!server_)
            return;

        server_->SetQueryType(nServerInfo::QUERY_ALL);
        server_->QueryServer();
        server_->ClearInfoFlags();
    }

    static void SendServerQuery()
    {
        sn_Receive();
        sn_SendPlanned();
    }

    bool isEmpty()
    {
        return name_ == "" || name_ == "Empty" || address_ == "";
    }

    tString lastRefreshTimeStr()
    {
        REAL lastTime = -999;
        if (server_)
            lastTime = server_->timeQuerySent;

        lastTime = tSysTimeFloat() - lastTime;

        tString output;

        if (lastTime < 0)
            output << "Never!";
        else
            output << lastTime << " seconds";
        return output;
    }

public:
    tString name_;    //!< the human readable name
    tString address_; //!< connection address
    int port_;        //!< port to connect to
    nServerInfo * server_;
private:
    int index_; //!< index in favorite holder

    tConfItemLine confName_;    //!< configuration item holding the name
    tConfItemLine confAddress_; //!< configuration item holding the address
    tConfItem<int> confPort;    //!< configuration item holding the port
};

//! server favorites management class: holds an array of servers
class gServerFavoritesHolder
{
public:
    gServerFavoritesHolder(char const *prefix)
        : custom(-1, prefix)
    {
        // generate favorites
        for (int i = NUM_FAVORITES - 1; i >= 0; --i)
            favorites[i] = new gServerFavorite(i, prefix);
    }

    ~gServerFavoritesHolder()
    {
        // destroy favorites
        for (int i = NUM_FAVORITES - 1; i >= 0; --i)
            delete favorites[i];
    }

    //! returns the favorite server info with the given index
    gServerFavorite &GetFavorite(int index)
    {
        if (index == -1)
            return custom;

        tASSERT(index >= 0 && index < NUM_FAVORITES);
        tASSERT(favorites[index]);
        return *favorites[index];
    }

    bool AddFavorite(nServerInfoBase const *server)
    {
        if (!server)
            return false;

        for (int i = NUM_FAVORITES - 1; i >= 0; --i)
        {
            gServerFavorite &fav = GetFavorite(i);

            if (fav.name_ == "" || fav.name_ == "Empty")
            {
                fav.name_ = tColoredString::RemoveColors(server->GetName());
                fav.address_ = server->GetConnectionName();
                fav.port_ = server->GetPort();

                return true;
            }
        }

        return false;
    }

    bool IsFavorite(nServerInfoBase const *server)
    {
        if (!server)
            return false;

        for (int i = NUM_FAVORITES - 1; i >= 0; --i)
        {
            gServerFavorite &fav = GetFavorite(i);

            if (fav.name_ != "" && fav.name_ != "Empty" && fav.address_ == server->GetConnectionName() && fav.port_ == static_cast<int>(server->GetPort()))
            {
                return true;
            }
        }

        return false;
    }

private:
    // regular bookmarks
    gServerFavorite *favorites[NUM_FAVORITES];

    // custom connect server
    gServerFavorite custom;
};

// server bookmarks
static gServerFavoritesHolder sg_favoriteHolder("");

// alternative master servers
static gServerFavoritesHolder sg_masterHolder("_MASTER");


static void sg_GenerateConnectionItems();

//! edit submenu item quitting the parent menu when it's done
class gMenuItemEditSubmenu : public uMenuItemSubmenu
{
private:
    gServerFavorite *favorite_;
public:

    gMenuItemEditSubmenu(uMenu *M, uMenu *s,
                         const tOutput &help, gServerFavorite *fav)
        : uMenuItemSubmenu(M, s, help), favorite_(fav)
    {
    }

    //! enters the submenu
    virtual void Enter()
    {
        // delegate to base
        uMenuItemSubmenu::Enter();

        // exit the parent menu (so we don't have to update the edit menu)
        menu->Exit();
    }

    virtual bool Event(SDL_Event &e) override {
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_DELETE:
                    if (favorite_)
                    {
                        favorite_->name_ = "";
                        favorite_->address_ = "";
                        favorite_->server_ = nullptr;
                        menu->RemoveItem(menu->Item(GetID()));;
                    }
                    return true;
                default:
                    break;
            }
        }
        return uMenuItemSubmenu::Event(e);
    }
};

//! connect to a favorite server
static void sg_ConnectFavorite(int ID)
{
    sg_favoriteHolder.GetFavorite(ID).Connect();
}

//! browse servers on alternative master server
static void sg_AlternativeMaster(int ID)
{
    // generate suffix for filename
    std::stringstream suffix;
    suffix << "_" << ID;

    // fetch server info
    gServerFavorite &favorite = sg_masterHolder.GetFavorite(ID);
    gServerInfoFavorite fav(favorite.address_, favorite.port_);

    // browse master info
    gServerBrowser::BrowseSpecialMaster(&fav, suffix.str().c_str());
}

// current connection function
static INTFUNCPTR sg_Connect = &sg_ConnectFavorite;
// current server holder
static gServerFavoritesHolder *sg_holder = &sg_favoriteHolder;

// current language id prefix
static char const *sg_languageIDPrefix = "$bookmarks_";
// yeah, this could all be more elegant.

// generates a language string item fitting the current situation
static void sg_AddBookmarkString(char const *suffix, tOutput &addTo)
{
    std::ostringstream s;
    s << sg_languageIDPrefix;
    s << suffix;
    addTo << s.str().c_str();
}

// generates a language string item fitting the current situation
static tOutput sg_GetBookmarkString(char const *suffix)
{
    tOutput ret;
    sg_AddBookmarkString(suffix, ret);
    return ret;
}

//! conglomerate of menus and entries for custom connect
class gCustomConnectEntries
{
public:
    void Generate(gServerFavorite &fav, uMenu *menu)
    {
        // prepare output reading "Edit <server name>"
        // create menu items (autodeleted when the edit menu is killed)
        tNEW(uMenuItemFunctionInt)(menu, sg_GetBookmarkString("menu_edit_connect_text"), sg_GetBookmarkString("menu_edit_connect_help"), sg_Connect, fav.GetIndex());
        tNEW(uMenuItemInt)(menu, "$network_custjoin_port_text", "$network_custjoin_port_help", fav.port_, 0, 0x7fffffff);
        tNEW(uMenuItemString)(menu, sg_GetBookmarkString("menu_address"), sg_GetBookmarkString("menu_address_help"), fav.address_);
    }

    gCustomConnectEntries()
    {
    }

    gCustomConnectEntries(gServerFavorite &fav, uMenu *menu)
    {
        Generate(fav, menu);
    }

    ~gCustomConnectEntries()
    {
    }

private:
};

//! conglomerate of menus and entries
class gServerFavoriteMenuEntries : public gCustomConnectEntries
{
public:
    gServerFavoriteMenuEntries(gServerFavorite &fav, uMenu &edit_menu)
    {
        // prepare output reading "Edit <server name>"
        tString serverName = fav.server_ ? fav.server_->GetName() : fav.name_;
        if (serverName == "" || serverName == "Empty")
        {
            std::stringstream s;
            s << "Server " << fav.GetIndex() + 1;

            serverName = s.str().c_str();
        }

        tOutput fe;
        fe.SetTemplateParameter(1, serverName);
        sg_AddBookmarkString("menu_edit_slot", fe);

        // create edit menu
        edit_ = tNEW(uMenu)(fe);
        editmenu_ = tNEW(gMenuItemEditSubmenu)(&edit_menu, edit_, fe, &fav);

        Generate(fav, edit_);

        tNEW(uMenuItemString)(edit_, sg_GetBookmarkString("menu_name"), sg_GetBookmarkString("menu_name_help"), fav.name_);
    }

    ~gServerFavoriteMenuEntries()
    {
        delete editmenu_;
        editmenu_ = 0;
        delete edit_;
        edit_ = 0;
    }

private:
    uMenu *edit_;
    uMenuItem *editmenu_;
};


static void sg_AddBookmarkMenu()
{
    int i;
    gServerFavoriteMenuEntries *entries[NUM_FAVORITES] = {nullptr};
    uMenu add_bookmark_menu(sg_GetBookmarkString("menu_add"));

    for (i = NUM_FAVORITES - 1; i >= 0; --i)
    {
        gServerFavorite &fav = sg_holder->GetFavorite(i);

        if (fav.isEmpty()) {
            entries[i] = tNEW(gServerFavoriteMenuEntries)(fav, add_bookmark_menu);
        }
    }

    add_bookmark_menu.Enter();

    for (i = NUM_FAVORITES - 1; i >= 0; --i) {
        if (entries[i] != nullptr) {
            delete entries[i];
            entries[i] = nullptr;
        }
    }

    // regenerate parent menu
    sg_GenerateConnectionItems();
}

// Edit servers submenu function
static void sg_EditServers()
{
    int i;

    // create menu
    uMenu edit_menu(sg_GetBookmarkString("menu_edit"));

    // create menu entries
    gServerFavoriteMenuEntries *entries[NUM_FAVORITES] = {nullptr};
    for (i = NUM_FAVORITES - 1; i >= 0; --i)
    {
        gServerFavorite &fav = sg_holder->GetFavorite(i);
        if (!fav.isEmpty()) {
            entries[i] = tNEW(gServerFavoriteMenuEntries)(fav, edit_menu);
        }
    }


    // enter menu
    edit_menu.Enter();

    // delete menu entries
    for (i = NUM_FAVORITES - 1; i >= 0; --i) {
        if (entries[i] != nullptr) {
            delete entries[i];
            entries[i] = nullptr;
        }
    }

    // regenerate parent menu
    sg_GenerateConnectionItems();
}

// ugly hack: functions clearing and filling the connection menu
static uMenu *sg_connectionMenu = 0;
static uMenuItem *sg_connectionMenuItemKeep = 0;
static uMenuItem *sg_connectionMenuItemAddKeep = 0;
static void sg_ClearConnectionItems()
{
    tASSERT(sg_connectionMenu);

    // delete old connection items
    for (int i = sg_connectionMenu->NumItems() - 1; i >= 0; --i)
    {
        uMenuItem *item = sg_connectionMenu->Item(i);
        if (item != sg_connectionMenuItemKeep && item != sg_connectionMenuItemAddKeep)
            delete item;
    }
}

#include "uMenu.h"
#include "SDL.h"

static REAL shrink = .6f;
static REAL displace = .15;
static REAL sg_menuBottom = -.9;
static REAL sg_requestBottom = -.9;

static REAL text_width = 0.025;
static REAL aspect = 1;
static int resW = 1, resH = 1;
static REAL text_height = .05;
class gFavoriteMenuItem;

static void sg_GenerateConnectionItem(gServerFavorite &fav, int index);
#include "rScreen.h"

class gFavoriteMenuItem : public uMenuItemFunctionInt
{
public:
    gServerFavorite *favorite_;

    gFavoriteMenuItem(uMenu *M, const tOutput &name,
                      const tOutput &help, INTFUNCPTR f, int arg, gServerFavorite *fav)
        : uMenuItemFunctionInt(M, name, help, f, arg), favorite_(fav)
    {
    }

    virtual ~gFavoriteMenuItem()
    {
    }

    void RenderBackground()
    {
    #ifndef DEDICATED
        REAL helpTopReal = sg_requestBottom * shrink + displace - .05;

        if (!favorite_)
            return;

        uMenuItemFunctionInt::RenderBackground();

        rTextField::SetDefaultColor(tColor(1, 1, 1));

        aspect = (REAL(sr_screenHeight) / sr_screenWidth) * 1.15;
        text_width = 0.025 * aspect;
        resH = sr_screenHeight;
        resW = sr_screenWidth;

        rTextField players(-.9, helpTopReal, text_width, text_height);
        nServerInfo *server = favorite_->server_;

        if (server)
        {
            players << tOutput("$network_master_players") << tOutput(" ");
            if (server->UserNamesOneLine().Len() > 2)
                players << server->UserNamesOneLine();
            else
                players << tOutput("$network_master_players_empty");
            players << "\n"
                    << tColoredString::ColorString(1, 1, 1);
            tColoredString uri;
            uri << server->Url() << tColoredString::ColorString(1, 1, 1);
            players << tOutput("$network_master_serverinfo", server->Release(), uri, server->Options());
        }

        REAL helpSpace = players.GetTop() - players.GetBottom();
        REAL helpTop = -.85 + helpSpace;
        REAL helpTopScaled = (helpTop - displace) / shrink;
        REAL helpTopMax = .25;
        REAL helpTopMin = -.9;

        if (helpTopScaled > helpTopMax)
            helpTopScaled = helpTopMax;

        if (helpTopScaled < helpTopMin)
            helpTopScaled = helpTopMin;

        sg_requestBottom = helpTopScaled;
    #endif
    }

    void UpdateMenuItem(bool create = true)
    {
        int menuItemID = GetID();

        uMenuItem *menuItem = sg_connectionMenu->Item(menuItemID);
        if (!menuItem)
            return;

        if (create)
            sg_GenerateConnectionItem(*favorite_, getArg());

        sg_connectionMenu->RemoveItem(menuItem);
    }

    virtual bool Event(SDL_Event &event)
    {
#ifndef DEDICATED
        if (event.type != SDL_KEYDOWN)
        {
            return false;
        }

        switch (event.key.keysym.sym)
        {
        case (SDLK_p):
            if (favorite_)
            {
                favorite_->PrepareServerQuery();
                gServerFavorite::SendServerQuery();
                UpdateMenuItem();
            }
            return true;
            break;
        case (SDLK_r):
            for (int i = NUM_FAVORITES - 1; i >= 0; --i)
            {
                gServerFavorite &fav = sg_holder->GetFavorite(i);
                if (!fav.isEmpty())
                    favorite_->PrepareServerQuery();
            }
            gServerFavorite::SendServerQuery();
            sg_GenerateConnectionItems();
            return true;
            break;
        default:
            break;
        }
#endif
        return uMenuItemFunctionInt::Event(event);
    }
};

static void sg_GenerateConnectionItems()
{
    tASSERT(sg_connectionMenu);

    // delete old connection items
    sg_ClearConnectionItems();

    // create new connection items
    for (int i = NUM_FAVORITES - 1; i >= 0; --i)
    {
        sg_GenerateConnectionItem(sg_holder->GetFavorite(i), i);
    }
}

static void sg_GenerateConnectionItem(gServerFavorite &fav, int index)
{
    tASSERT(sg_connectionMenu);

    if (!fav.isEmpty())
    {
        tString serverName = fav.name_;
        nServerInfo *server = fav.server_;
        if (server)
        {
            serverName = server->GetName();
            serverName << " 0xRESETT("
                       << server->Users()
                       << "/"
                       << server->MaxUsers()
                       << ")\n";
        }
        else
        {
            serverName = tColoredString::RemoveColors(serverName);
        }

        tOutput serveNameOutput;
        serveNameOutput << serverName;

        tOutput lastPollHelp;
        lastPollHelp.SetTemplateParameter(1, serverName);
        lastPollHelp.SetTemplateParameter(2, fav.lastRefreshTimeStr());
        sg_AddBookmarkString("menu_last_poll_help", lastPollHelp);

        tNEW(gFavoriteMenuItem)(sg_connectionMenu, serveNameOutput, lastPollHelp, sg_Connect, index, &fav);
    }
}

//! TODO for 3.0 or 3.1: phase out this legacy support
static tString sg_customServerName("");
static tConfItemLine sg_serverName_ci("CUSTOM_SERVER_NAME", sg_customServerName);
static int sg_clientPort = 4534;
static tConfItem<int> sg_cport("CLIENT_PORT", sg_clientPort);

static bool sg_bookmarksAutoPoll = true;
static tConfItem<bool> sg_bookmarksAutoPollConf("BOOKMARKS_AUTO_POLL", sg_bookmarksAutoPoll);
//! transfer old custom server name to favorite
static void sg_TransferCustomServer()
{
    if (sg_customServerName != "")
    {
        // add custom connect server to favorites
        gServerInfoFavorite server(sg_customServerName, sg_bookmarksAutoPoll);
        gServerFavorites::AddFavorite(&server);

        // clear custom connect server
        sg_customServerName = "";
    }
}

#include <unordered_map>
#include <string>
#include <utility>

struct PairHash {
public:
    template <typename T1, typename T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

void sg_LinkFavoritesToServers(bool pollMaster = false)
{
    nServerInfo::GetFromMaster(0, "", false);
    nServerInfo *server = nServerInfo::GetFirstServer();

    std::unordered_map<std::pair<tString, int>, gServerFavorite *, PairHash> favoriteMap;

    for (int i = NUM_FAVORITES - 1; i >= 0; --i)
    {
        gServerFavorite &fav = sg_holder->GetFavorite(i);
        favoriteMap[std::make_pair(fav.address_, fav.port_)] = &fav;
    }

    while (server)
    {
        auto favItem = favoriteMap.find(std::make_pair(server->GetConnectionName(), static_cast<int>(server->GetPort())));
        if (favItem != favoriteMap.end())
        {
            if (pollMaster)
            {
                server->SetQueryType(nServerInfo::QUERY_ALL);
                server->QueryServer();
                server->ClearInfoFlags();
            }
            favItem->second->server_ = server;
        }
        server = server->Next();
    }
    if (pollMaster) {
    sn_Receive();
    sn_SendPlanned();
    }
}



static void sg_FavoritesMenu(INTFUNCPTR connect, gServerFavoritesHolder &holder, bool pingMaster = false)
{
    sg_Connect = connect;
    sg_holder = &holder;

    sg_TransferCustomServer();

    uMenu net_menu(sg_GetBookmarkString("menu"));
    sg_connectionMenu = &net_menu;

    uMenuItemFunction edit(&net_menu, sg_GetBookmarkString("menu_edit"), sg_GetBookmarkString("menu_edit_help"), &sg_EditServers);
    uMenuItemFunction add(&net_menu, sg_GetBookmarkString("menu_add"), sg_GetBookmarkString("menu_add_help"), &sg_AddBookmarkMenu);

    sg_connectionMenuItemKeep = &edit;
    sg_connectionMenuItemAddKeep = &add;

    sg_LinkFavoritesToServers(pingMaster);
    sg_GenerateConnectionItems();
    net_menu.Enter();
    sg_ClearConnectionItems();

    sg_connectionMenuItemKeep = NULL;
    sg_connectionMenu = NULL;
}

// *********************************************************************************************
// *
// *	FavoritesMenu
// *
// *********************************************************************************************
//!
//!
// *********************************************************************************************

void gServerFavorites::FavoritesMenu(void)
{
    sg_languageIDPrefix = "$bookmarks_";
    sg_FavoritesMenu(&sg_ConnectFavorite, sg_favoriteHolder, sg_bookmarksAutoPoll);
}

void gServerFavorites::FavoritesMenuForceQuery(bool query)
{
    sg_languageIDPrefix = "$bookmarks_";
    sg_FavoritesMenu(&sg_ConnectFavorite, sg_favoriteHolder, query);
}

void favoritesMenuLaunch(std::istream &s)
{
    gServerFavorites::FavoritesMenu();
}
static tConfItemFunc HelperMenuConf = HelperCommand::tConfItemFunc("BOOKMARKS_MENU_LAUNCH", &favoritesMenuLaunch);

// *********************************************************************************************
// *
// *	AlternativesMenu
// *
// *********************************************************************************************
//!
//!
// *********************************************************************************************

void gServerFavorites::AlternativesMenu(void)
{
    // load default subcultures
    nServerInfo::DeleteAll();
    nServerInfo::Load(tDirectories::Config(), "subcultures.srv");

    // add them
    nServerInfo *run = nServerInfo::GetFirstServer();
    while (run)
    {
        if (!sg_masterHolder.IsFavorite(run))
        {
            sg_masterHolder.AddFavorite(run);
        }

        run = run->Next();
    }

    nServerInfo::DeleteAll();

    sg_languageIDPrefix = "$masters_";
    sg_FavoritesMenu(&sg_AlternativeMaster, sg_masterHolder);
}

// *********************************************************************************************
// *
// *	CustomConnectMenu
// *
// *********************************************************************************************
//!
//!
// *********************************************************************************************

void gServerFavorites::CustomConnectMenu(void)
{
    sg_TransferCustomServer();

    uMenu net_menu("$network_custjoin_text");
    sg_connectionMenu = &net_menu;

    gServerFavorite &fav = sg_favoriteHolder.GetFavorite(-1);

    // create menu entries
    sg_languageIDPrefix = "$bookmarks_";
    sg_Connect = &sg_ConnectFavorite;
    gCustomConnectEntries submenu(fav, &net_menu);

    net_menu.Enter();
}

// *********************************************************************************************
// *
// *	AddFavorite
// *
// *********************************************************************************************
//!
//!		@param	server	 the server to add to the favorites
//!     @return true if successful, false if favorite list is full
//!
// *********************************************************************************************

bool gServerFavorites::AddFavorite(nServerInfoBase const *server)
{
    return sg_favoriteHolder.AddFavorite(server);
}

// *********************************************************************************************
// *
// *	IsFavorite
// *
// *********************************************************************************************
//!
//!		@param	server	server to check whether it is bookmarked
//!		@return		    true if the server is in the list of favorites
//!
// *********************************************************************************************

bool gServerFavorites::IsFavorite(nServerInfoBase const *server)
{
    return sg_favoriteHolder.IsFavorite(server);
}
