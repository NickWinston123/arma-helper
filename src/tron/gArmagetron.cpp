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

#include "gStuff.h"
#include "tSysTime.h"
#include "tDirectories.h"
#include "tLocale.h"
#include "rViewport.h"
#include "rConsole.h"
#include "gGame.h"
#include "gLogo.h"
#include "eSound.h"
#include "rScreen.h"
#include "rSysdep.h"
#include "uInputQueue.h"
// #include "eTess.h"
#include "rTexture.h"
#include "tConfiguration.h"
#include "tRandom.h"
#include "tRecorder.h"
#include "tCommandLine.h"
#include "tToDo.h"
#include "eAdvWall.h"
#include "eGameObject.h"
#include "uMenu.h"
#include "ePlayer.h"
#include "ePlayerStats.h"
#include "gLanguageMenu.h"
#include "gAICharacter.h"
#include "gCycle.h"
#include "gMenus.h"
// #include <unistd>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "nNetwork.h"
#include "nServerInfo.h"
#include "nSocket.h"

#ifndef DEDICATED
#include "rRender.h"
#include "rSDL.h"
#endif
#include "eChatCommands.h"


bool tryConnectLastServer = false;
// data structure for command line parsing
class gMainCommandLineAnalyzer : public tCommandLineAnalyzer
{
public:
    bool fullscreen_;
    bool windowed_;
    bool use_directx_;
    bool dont_use_directx_;

    gMainCommandLineAnalyzer()
    {
        windowed_ = false;
        fullscreen_ = false;
        use_directx_ = false;
        dont_use_directx_ = false;
    }

private:
    bool DoAnalyze(tCommandLineParser &parser, int pass) override
    {
        if (pass > 0)
            return false;

        if (parser.GetSwitch("-fullscreen", "-f"))
        {
            fullscreen_ = true;
        }
        else if (parser.GetSwitch("-window", "-w") || parser.GetSwitch("-windowed"))
        {
            windowed_ = true;
        }
        else if (parser.GetSwitch("--exportallcfg"))
        {
            tConfItemBase::ExportAll();
            exit(0);
        }
#ifdef WIN32
        else if (parser.GetSwitch("+directx"))
        {
            use_directx_ = true;
        }
        else if (parser.GetSwitch("-directx"))
        {
            dont_use_directx_ = true;
        }
#endif
        else
        {
            return false;
        }

        return true;
    }

    void DoHelp(std::ostream &s) override
    { //
#ifndef DEDICATED
        s << "-f, --fullscreen             : start in fullscreen mode\n";
        s << "-w, --window, --windowed     : start in windowed mode\n\n";
#ifdef WIN32
        s << "+directx, -directx           : enable/disable usage of DirectX for screen\n"
          << "                               initialisation under MS Windows\n\n";
        s << "\n\nYes, I know this looks ugly. Sorry about that.\n";
#endif
        s << "--exportallcfg               : print all configs and settings that can be saved to standard output\n";
#endif
    }
};

static gMainCommandLineAnalyzer commandLineAnalyzer;

extern bool sr_useDirectX; // rScreen.cpp
#ifdef WIN32
static tConfItem<bool> udx("USE_DIRECTX", "makes use of the DirectX input "
                                          "fuctions; causes some graphic cards to fail to work (VooDoo 3,...)",
                           sr_useDirectX);
#endif

extern void exit_game_objects(eGrid *grid);

enum gConnection
{
    gLeave,
    gDialup,
    gISDN,
    gDSL
    // gT1
};

// initial setup menu
void sg_StartupPlayerMenu()
{
    uMenu firstSetup("$first_setup", false);
    firstSetup.SetBot(-.2);

    uMenuItemExit e2(&firstSetup, "$menuitem_accept", "$menuitem_accept_help");

    ePlayer *player = ePlayer::PlayerConfig(0);
    tASSERT(player);

    gConnection connection = gDSL;

    uMenuItemSelection<gConnection> net(&firstSetup, "$first_setup_net", "$first_setup_net_help", connection);
    if (!st_FirstUse)
    {
        net.NewChoice("$first_setup_leave", "$first_setup_leave_help", gLeave);
        connection = gLeave;
    }
    net.NewChoice("$first_setup_net_dialup", "$first_setup_net_dialup_help", gDialup);
    net.NewChoice("$first_setup_net_isdn", "$first_setup_net_isdn_help", gISDN);
    net.NewChoice("$first_setup_net_dsl", "$first_setup_net_dsl_help", gDSL);

    tString keyboardTemplate("keys_cursor.cfg");
    uMenuItemSelection<tString> k(&firstSetup, "$first_setup_keys", "$first_setup_keys_help", keyboardTemplate);
    if (!st_FirstUse)
    {
        k.NewChoice("$first_setup_leave", "$first_setup_leave_help", tString(""));
        keyboardTemplate = "";
    }
    k.NewChoice("$first_setup_keys_cursor", "$first_setup_keys_cursor_help", tString("keys_cursor.cfg"));
    k.NewChoice("$first_setup_keys_wasd", "$first_setup_keys_wasd_help", tString("keys_wasd.cfg"));
    k.NewChoice("$first_setup_keys_zqsd", "$first_setup_keys_zqsd_help", tString("keys_zqsd.cfg"));
    k.NewChoice("$first_setup_keys_cursor_single", "$first_setup_keys_cursor_single_help", tString("keys_cursor_single.cfg"));
    // k.NewChoice( "$first_setup_keys_both", "$first_setup_keys_both_help", tString("keys_twohand.cfg") );
    k.NewChoice("$first_setup_keys_x", "$first_setup_keys_x_help", tString("keys_x.cfg"));

    tColor leave(0, 0, 0, 0);
    tColor color(1, 0, 0);
    uMenuItemSelection<tColor> c(&firstSetup,
                                 "$first_setup_color",
                                 "$first_setup_color_help",
                                 color);

    if (!st_FirstUse)
    {
        color = leave;
        c.NewChoice("$first_setup_leave", "$first_setup_leave_help", leave);
    }

    c.NewChoice("$first_setup_color_red", "", tColor(1, 0, 0));
    c.NewChoice("$first_setup_color_blue", "", tColor(0, 0, 1));
    c.NewChoice("$first_setup_color_green", "", tColor(0, 1, 0));
    c.NewChoice("$first_setup_color_yellow", "", tColor(1, 1, 0));
    c.NewChoice("$first_setup_color_orange", "", tColor(1, .5, 0));
    c.NewChoice("$first_setup_color_purple", "", tColor(.5, 0, 1));
    c.NewChoice("$first_setup_color_magenta", "", tColor(1, 0, 1));
    c.NewChoice("$first_setup_color_cyan", "", tColor(0, 1, 1));
    c.NewChoice("$first_setup_color_white", "", tColor(1, 1, 1));
    c.NewChoice("$first_setup_color_dark", "", tColor(0, 0, 0));

    if (st_FirstUse)
    {
        for (int i = tRandomizer::GetInstance().Get(4); i >= 0; --i)
        {
            c.LeftRight(1);
        }
    }

    uMenuItemString n(&firstSetup,
                      "$player_name_text",
                      "$player_name_help",
                      player->name, 16);

    uMenuItemExit e(&firstSetup, "$menuitem_accept", "$menuitem_accept_help");

    firstSetup.Enter();

    // apply network rates
    switch (connection)
    {
    case gDialup:
        sn_maxRateIn = 600;
        sn_maxRateOut = 400;
        break;
    case gISDN:
        sn_maxRateIn = 800;
        sn_maxRateOut = 800;
        break;
    case gDSL:
        sn_maxRateIn = 6400;
        sn_maxRateOut = 1600;
        break;
    case gLeave:
        break;
    }

    // store color
    if (!(color == leave))
    {
        player->rgb[0] = int(color.r_ * 15);
        player->rgb[1] = int(color.g_ * 15);
        player->rgb[2] = int(color.b_ * 15);
    }

    // load keyboard layout
    if (keyboardTemplate.Len() > 1)
    {
        std::ifstream s;
        if (tConfItemBase::OpenFile(s, keyboardTemplate, tConfItemBase::Config))
        {
            tCurrentAccessLevel level(tAccessLevel_Owner, true);
            tConfItemBase::ReadFile(s);
        }
    }
}

#ifndef DEDICATED
static void welcome()
{
    bool textOutBack = sr_textOut;
    sr_textOut = false;

#ifdef DEBUG_XXXX
    {
        for (int i = 20; i >= 0; i--)
        {
            sr_ClearGL();
            {
                rTextField c(-.8, .6, .1, .1);
                tString s;
                s << ColorString(1, 1, 1);
                s << "Test";
                s << ColorString(1, 0, 0);
                s << "bla bla blubb blaa blaa blubbb blaaa blaaa blubbbb blaaaa blaaaa blubbbbb blaaaaa blaaaaa blubbbbbb blaaaaaa\n";
                c << s;
            }
            sr_SwapGL();
        }
    }
#endif

    REAL timeout = tSysTimeFloat() + .2;
    SDL_Event tEvent;

    if (st_FirstUse)
    {
        sr_LoadDefaultConfig();
        textOutBack = sr_textOut;
        sr_textOut = false;
        gLogo::SetBig(false);
        gLogo::SetSpinning(true);
    }
    else
    {
        bool showSplash = true;
#ifdef DEBUG
        showSplash = false;
#endif

        // disable splash screen when recording (it's annoying)
        static const char *splashSection = "SPLASH";
        if (tRecorder::IsRunning())
        {
            showSplash = false;

            // but keep it for old recordings where the splash screen was always active
            if (!tRecorder::Playback(splashSection, showSplash))
                showSplash = tRecorder::IsPlayingBack();
        }
        tRecorder::Record(splashSection, showSplash);

        if (tryConnectLastServer)
            showSplash = false;

        if (showSplash)
        {
            timeout = tSysTimeFloat() + 6;

            uInputProcessGuard inputProcessGuard;
            while ((!su_GetSDLInput(tEvent) || tEvent.type != SDL_KEYDOWN) &&
                   tSysTimeFloat() < timeout)
            {
                if (sr_glOut)
                {
                    sr_ResetRenderState(true);
                    rViewport::s_viewportFullscreen.Select();

                    rSysDep::ClearGL();

                    uMenu::GenericBackground();

                    rSysDep::SwapGL();
                }

                tAdvanceFrame();
            }
        }

        // catch some keyboard input
        {
            uInputProcessGuard inputProcessGuard;
            while (su_GetSDLInput(tEvent))
                ;
        }

        sr_textOut = textOutBack;
        return;
    }

    if (sr_glOut)
    {
        rSysDep::ClearGL();
        rFont::s_defaultFont.Select();
        rFont::s_defaultFontSmall.Select();
        gLogo::Display();
        rSysDep::ClearGL();
    }
    rSysDep::SwapGL();

    sr_textOut = textOutBack;
    sg_StartupLanguageMenu();

    sr_textOut = textOutBack;
    sg_StartupPlayerMenu();

    st_FirstUse = false;

    sr_textOut = textOutBack;
    uMenu::Message(tOutput("$welcome_message_heading"), tOutput("$welcome_message"), 300);

    // start a first single player game
    auto speedFactor = sg_currentSettings->speedFactor;
    auto autoNum = sg_currentSettings->autoNum;
    auto sizeFactor = sg_currentSettings->sizeFactor;
    auto wallsLength = sg_currentSettings->wallsLength;
    auto rubber = sg_rubberCycle;
    auto delayCycle = sg_delayCycle;
    sg_currentSettings->speedFactor = -2;
    sg_currentSettings->autoNum = 0;
    sg_currentSettings->sizeFactor -= 2;
    sg_currentSettings->wallsLength = 400;
    sg_rubberCycle = 5;
    sg_delayCycle = 0.05;
    sr_textOut = textOutBack;
    sg_SinglePlayerGame();
    sg_currentSettings->autoNum = autoNum;
    sg_currentSettings->speedFactor = speedFactor;
    sg_currentSettings->sizeFactor = sizeFactor;
    sg_currentSettings->wallsLength = wallsLength;
    sg_rubberCycle = rubber;
    sg_delayCycle = delayCycle;

    sr_textOut = textOutBack;
    uMenu::Message(tOutput("$welcome_message_2_heading"), tOutput("$welcome_message_2"), 300);

    sr_textOut = textOutBack;
}
#endif

void cleanup(eGrid *grid)
{
    static bool reentry = true;
    if (reentry)
    {
        reentry = false;
        su_contInput = false;

        exit_game_objects(grid);
        /*
          for(int i=MAX_PLAYERS-1;i>=0;i--){
          if (playerConfig[i])
          destroy(playerConfig[i]->cam);
          }


          gNetPlayerWall::Clear();

          eFace::Clear();
          eEdge::Clear();
          ePoint::Clear();

          eFace::Clear();
          eEdge::Clear();
          ePoint::Clear();

          eGameObject::DeleteAll();


        */

#ifdef POWERPAK_DEB
        if (pp_out)
        {
            PD_Quit();
            PP_Quit();
        }
#endif
        nNetObject::ClearAll();

        if (sr_glOut)
        {
            rITexture::UnloadAll();
        }

        se_SoundExit();
        sr_glOut = false;
        sr_ExitDisplay();

#ifndef DEDICATED
        sr_RendererCleanup();
#endif
    }
}

#ifndef DEDICATED
static bool sg_active = true;
static void sg_DelayedActivation()
{
    Activate(sg_active);
}

int filter(const SDL_Event *tEvent)
{
    // recursion avoidance
    static bool recursion = false;
    if (!recursion)
    {
        class RecursionGuard
        {
        public:
            RecursionGuard(bool &recursion)
                : recursion_(recursion)
            {
                recursion = true;
            }

            ~RecursionGuard()
            {
                recursion_ = false;
            }

        private:
            bool &recursion_;
        };

        RecursionGuard guard(recursion);

        // boss key or OS X quit command
        if ((tEvent->type == SDL_KEYDOWN && tEvent->key.keysym.sym == 27 &&
             tEvent->key.keysym.mod & KMOD_SHIFT) ||
            (tEvent->type == SDL_KEYDOWN && tEvent->key.keysym.sym == 113 &&
             tEvent->key.keysym.mod & KMOD_META) ||
            (tEvent->type == SDL_QUIT))
        {
            // sn_SetNetState(nSTANDALONE);
            // sn_Receive();

            // register end of recording
            tRecorder::Record("END");

            st_SaveConfig();
            uMenu::quickexit = uMenu::QuickExit_Total;
            return false;
        }

        if (tEvent->type == SDL_MOUSEMOTION)
            if (tEvent->motion.x == sr_screenWidth / 2 && tEvent->motion.y == sr_screenHeight / 2)
                return 0;
        if (su_mouseGrab &&
            tEvent->type != SDL_MOUSEBUTTONDOWN &&
            tEvent->type != SDL_MOUSEBUTTONUP &&
            ((tEvent->motion.x >= sr_screenWidth - 10 || tEvent->motion.x <= 10) ||
             (tEvent->motion.y >= sr_screenHeight - 10 || tEvent->motion.y <= 10)))
            SDL_WarpMouse(sr_screenWidth / 2, sr_screenHeight / 2);

        if (tEvent->type == SDL_VIDEORESIZE && !currentScreensetting.fullscreen)
        {
            st_ToDo(rCallbackBeforeScreenModeChange::Exec);
            sr_screenWidth = tEvent->resize.w;
            sr_screenHeight = tEvent->resize.h;

            // Alright, SDL1 requires setting the video mode after a resize
            // color depth seems to be ignored, but we'll specify it anyway
            sr_screen = SDL_SetVideoMode(
                sr_screenWidth, sr_screenHeight,
                (currentScreensetting.colorDepth) ? 24 : 16,
                ((currentScreensetting.useSDL) ? SDL_OPENGL : (SDL_DOUBLEBUF | SDL_SWSURFACE)) | SDL_RESIZABLE);
            if (sr_screen == NULL)
            {
                // if it does fail we'll just disable resizing...
                // TODO
                sr_ReinitDisplay();
            }
            st_ToDo(rCallbackAfterScreenModeChange::Exec);
        }

        // fetch alt-tab

        if (tEvent->type == SDL_ACTIVEEVENT)
        {
            // Jonathans fullscreen bugfix.
#ifdef MACOSX
            if (currentScreensetting.fullscreen ^ lastSuccess.fullscreen)
                return false;
#endif
            int flags = SDL_APPINPUTFOCUS;
            if (tEvent->active.state & flags)
            {
                // con << tSysTimeFloat() << " " << "active: " << (tEvent->active.gain ? "on" : "off") << "\n";
                sg_active = tEvent->active.gain;
                st_ToDo(sg_DelayedActivation);
            }

            // reload GL stuff if application gets reactivated
            if (tEvent->active.gain && tEvent->active.state & SDL_APPACTIVE)
            {
                // just treat it like a screen mode change, gets the job done
                st_ToDo(rCallbackBeforeScreenModeChange::Exec);
                st_ToDo(rCallbackAfterScreenModeChange::Exec);
            }
            return false;
        }

        if (su_prefetchInput)
        {
            return su_StoreSDLEvent(*tEvent);
        }
    }

    return 1;
}
#endif

// from game.C
void Update_netPlayer();

void sg_SetIcon()
{
#ifndef DEDICATED
    rSurface tex("textures/icon.png");
    //    SDL_Surface *tex=IMG_Load( tDirectories::Data().GetReadPath( "textures/icon.png" ) );

    if (tex.GetSurface())
        SDL_WM_SetIcon(tex.GetSurface(), NULL);
#endif
}

class gAutoStringArray
{
public:
    ~gAutoStringArray()
    {
#ifdef HAVE_CLEARENV
        // Optional. Systems that don't have this function better make copies of putenv() arguments.
        clearenv();
#endif

        for (std::vector<char *>::iterator i = strings.begin(); i != strings.end(); ++i)
        {
            free(*i);
        }
    }

    char *Store(char const *s)
    {
        char *ret = strdup(s);
        strings.push_back(ret);
        return ret;
    }

private:
    std::vector<char *> strings; // the stored raw C strings
};

// wrapper for putenv, taking care of the peculiarity that the argument
// is kept in use for the rest of the program's lifetime
void sg_PutEnv(char const *s)
{
    static gAutoStringArray store;
    putenv(store.Store(s));
}

namespace
{
    tString sn_configurationSavedInVersion{"0.2.8"};
    tConfItem<tString> sn_configurationSavedInVersionConf("SAVED_IN_VERSION", sn_configurationSavedInVersion);
}

tString sg_playingBackVersion;
extern void sg_MenusForVersion(tString version);

int main(int argc, char **argv)
{
    // std::cout << "enter\n";
    //   net_test();

    bool dedicatedServer = false;

    //  std::cout << "Running " << argv[0] << "...\n";

    // tERR_MESSAGE( "Start!" );

    try
    {
        tCommandLineData commandLine;
        commandLine.programVersion_ = &sn_programVersion;

        // analyse command line
        // tERR_MESSAGE( "Analyzing command line." );
        if (!commandLine.Analyse(argc, argv))
            return 0;

        {
            // embed version in recording
            const char *versionSection = "VERSION";
            tString version(sn_programVersion);
            tRecorder::Playback(versionSection, version);
            tRecorder::Record(versionSection, version);
#ifndef DEDICATED
            if (version != sn_programVersion)
            {
#ifdef DEBUG
                tERR_WARN("Recording from a different version, consider at high risk of desync.");
#endif
                tRecorder::ActivateProbablyDesyncedPlayback();
            }
#endif
            sg_MenusForVersion(version);
        }

        {
            // read/write server/client mode from/to recording
            const char *dedicatedSection = "DEDICATED";
            if (!tRecorder::PlaybackStrict(dedicatedSection, dedicatedServer))
            {
#ifdef DEDICATED
                dedicatedServer = true;
#endif
            }
            tRecorder::Record(dedicatedSection, dedicatedServer);
        }

        // tERR_MESSAGE( "Initializing player data." );
        ePlayer::Init();

        // tERR_MESSAGE( "Loading configuration." );
        tLocale::Load("languages.txt");

        VariableCreatorLoader();

        st_LoadConfig();

        if (sg_commandWatch)
            CommandWatchLoader();
        
        LoadChatCommandConfCommands();
        CommandShortcutLoader();


        if (sn_updateVersionOverrideValue >= 0)
        {
            updateVersionOverride(sn_updateVersionOverrideValue);
        }

        if (sn_bannedWatch)
            FileManager(tString("banned.txt"),tDirectories::Var()).Clear();

        uMenuItemStringWithHistory::LoadHistoryFromFile(se_consoleHistoryFileName, se_consoleHistory);
        uMenuItemStringWithHistory::LoadHistoryFromFile(se_chatHistoryFileName, se_chatHistory);

        tryConnectLastServer = sg_connectToLastServerOnStart && !sg_lastServerStr.empty();

        if (se_playerStats)
            ePlayerStats::loadStatsFromDB();

        // migrate user configuration from previous versions
        if (sn_configurationSavedInVersion != sn_programVersion)
        {
            if (st_FirstUse)
            {
                sn_configurationSavedInVersion = "0.0";
            }

            tConfigMigration::Migrate(sn_configurationSavedInVersion);
        }
        if (tConfigMigration::SavedBefore(sn_configurationSavedInVersion, sn_programVersion))
            sn_configurationSavedInVersion = sn_programVersion;

        // record and play back the recording debug level
        tRecorderSyncBase::GetDebugLevelPlayback();

        if (commandLineAnalyzer.fullscreen_)
            currentScreensetting.fullscreen = true;
        if (commandLineAnalyzer.windowed_)
            currentScreensetting.fullscreen = false;
        if (commandLineAnalyzer.use_directx_)
            sr_useDirectX = true;
        if (commandLineAnalyzer.dont_use_directx_)
            sr_useDirectX = false;

        gAICharacter::LoadAll(tString("aiplayers.cfg"));

        sg_LanguageInit();
        atexit(tLocale::Clear);

        static eLadderLogWriter sg_encodingWriter("ENCODING", true);
        sg_encodingWriter << st_internalEncoding;
        sg_encodingWriter.write();

        if (commandLine.Execute())
        {
            gCycle::PrivateSettings();

            {
                std::ifstream t;

                if (!tDirectories::Config().Open(t, "settings.cfg"))
                {
                    //		#ifdef WIN32
                    //                    tERR_ERROR( "Data files not found. You have to run Armagetron from its own directory." );
                    //		#else
                    tERR_ERROR("Configuration files not found. Check your installation.");
                    //		#endif
                }
            }

            {
                std::ofstream s;
                if (!tDirectories::Var().Open(s, "scorelog.txt", std::ios::app))
                {
                    char const *error = "var directory not writable or does not exist. It should reside inside your user data directory and should have been created automatically on first start, but something must have gone wrong."
#ifdef WIN32
                                        " You can access your user data directory over one of the start menu entries we installed."
#else
                                         " Your user data directory is subdirectory named .armagetronad in your home directory."
#endif
                        ;

                    tERR_ERROR(error);
                }
            }

            {
                std::ifstream t;

                if (tDirectories::Data().Open(t, "moviepack/settings.cfg"))
                {
                    sg_moviepackInstalled = true;
                }
            }

#ifndef DEDICATED
            sr_glOut = 1;
#endif

            // std::cout << "checked mp\n";

            // while DGA mouse is buggy in XFree 4.0:
#ifdef linux
            // Sam 5/23 - Don't ever use DGA, we don't need it for this game.
            if (!getenv("SDL_VIDEO_X11_DGAMOUSE"))
            {
                sg_PutEnv("SDL_VIDEO_X11_DGAMOUSE=0");
            }
#endif

#ifdef WIN32
            // disable DirectX by default; it causes problems with some boards.
            if (!getenv("SDL_VIDEODRIVER"))
            {
                if (sr_useDirectX)
                {
                    // NO. Causes crashes with SDL1.2 on Windows on window deactivation/alt-tabbing away
                    // sg_PutEnv( "SDL_VIDEODRIVER=directx" );
                }
                else
                {
                    sg_PutEnv("SDL_VIDEODRIVER=windib");
                }
            }
#endif

            // atexit(ANET_Shutdown);

#ifndef WIN32
#ifdef DEBUG
#define NOSOUND
#endif
#endif

#ifndef DEDICATED
            if (
#ifndef NOSOUND
#ifndef DEFAULT_SDL_AUDIODRIVER
                SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 &&
#endif
#endif
                SDL_Init(SDL_INIT_VIDEO) < 0)
            {
                tERR_ERROR("Couldn't initialize SDL: " << SDL_GetError());
            }
            atexit(SDL_Quit);

            sr_glRendererInit();

            SDL_SetEventFilter(&filter);

            // std::cout << "set filter\n";

            sg_SetIcon();

            tConsole::RegisterMessageCallback(&uMenu::Message);
            tConsole::RegisterIdleCallback(&uMenu::IdleInput);

#ifndef NOSOUND
            se_SoundInit();
            atexit(se_SoundExit);
#ifndef DEBUG
            // double sound initialisation for dodgy cards
            se_SoundExit();
            se_SoundInit();
#endif
#endif

            if (sr_InitDisplay())
            {

                try
                {
                    // std::cout << "init disp\n";

                    // std::cout << "init sound\n";

                    welcome();
                    // std::cout << "atexit\n";

                    sr_con.autoDisplayAtSwap = false;

                    se_SoundPause(false);

                    // std::cout << "sound started\n";

                    gLogo::SetBig(false);
                    gLogo::SetSpinning(true);

                    sn_bigBrotherString = renderer_identification + "VER=" + sn_programVersion + "\n\n";

                    InitHelperItems();
                   
                    if (tryConnectLastServer)
                        ConnectToLastServer();

                                        
                    if (sg_playerWatchServerDisconnectedWatch)
                        sg_scheduleDisconnectedFromServerCheckCheck();
                
                    MainMenu();

                    // remove all players
                    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
                        se_PlayerNetIDs(i)->RemoveFromGame();

                    nNetObject::ClearAll();

                    rITexture::UnloadAll();
                }
                catch (tException const &e)
                {
                    gLogo::SetDisplayed(true);
                    uMenu::SetIdle(NULL);
                    sr_con.autoDisplayAtSwap = false;

                    // inform user of generic errors
                    tConsole::Message(e.GetName(), e.GetDescription(), 20);
                }

                sr_ExitDisplay();
                sr_RendererCleanup();

                // std::cout << "exit\n";

                st_SaveConfig();

                SaveVariableCreatorVariablesToFile();

                uMenuItemStringWithHistory::SaveHistoryToFile(se_consoleHistoryFileName,se_consoleHistory);
                uMenuItemStringWithHistory::SaveHistoryToFile(se_chatHistoryFileName,se_chatHistory);
                // std::cout << "saved\n";

                //    cleanup(grid);
                SDL_QuitSubSystem(SDL_INIT_VIDEO);
            }
            se_SoundExit();
            SDL_Quit();
#else
            sr_glOut = 0;

            //  nServerInfo::TellMasterAboutMe();

            while (!uMenu::quickexit)
                sg_HostGame();
#endif
            nNetObject::ClearAll();
            nServerInfo::DeleteAll();
        }

        ePlayer::Exit();

        //	tLocale::Clear();
    }
    catch (tCleanQuit const &e)
    {
        return 0;
    }
    catch (tException const &e)
    {
        try
        {
            st_PresentError(e.GetName(), e.GetDescription());
        }
        catch (...)
        {
        }

        return 1;
    }
#ifdef _MSC_VER
#pragma warning(disable : 4286)
    // GRR. Visual C++ dones not handle generic exceptions with the above general statement.
    // A specialized version is needed. The best part: it warns about the code below being redundant.
    catch (tGenericException const &e)
    {
        try
        {
            st_PresentError(e.GetName(), e.GetDescription());
        }
        catch (...)
        {
        }

        return 1;
    }
#endif
    catch (...)
    {
        return 1;
    }

    return 0;
}

#ifdef DEDICATED
// settings missing in the dedicated server
static void st_Dummy(std::istream &s)
{
    tString rest;
    rest.ReadLine(s);
}
static tConfItemFunc st_Dummy10("MASTER_QUERY_INTERVAL", &st_Dummy);
static tConfItemFunc st_Dummy11("MASTER_SAVE_INTERVAL", &st_Dummy);
static tConfItemFunc st_Dummy12("MASTER_IDLE", &st_Dummy);
static tConfItemFunc st_Dummy13("MASTER_PORT", &st_Dummy);
#endif
