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

#include "gMenus.h"
#include "ePlayer.h"
#include "rScreen.h"
#include "nConfig.h"
#include "rConsole.h"
#include "tToDo.h"
#include "rGL.h"
#include "eTimer.h"
#include "eFloor.h"
#include "rRender.h"
#include "rModel.h"
#include "gGame.h"
#include "gCycle.h"
#include "gHud.h"
#include "tRecorder.h"
#include "rSysdep.h"

#include <sstream>
#include <set>

extern tString sg_playingBackVersion;

static bool o28 = false, o29 = false;

bool sg_ShowSpecialMenu() { return !(o28 || o29); }
bool sg_ShowConfigMenu()  { return !(o28 || o29); }
bool sg_ShowSBFilterItem(){ return !(o28 || o29); }

static tConfItem<int>   tm0("TEXTURE_MODE_0",rTextureGroups::TextureMode[0]);
static tConfItem<int>   tm1("TEXTURE_MODE_1",rTextureGroups::TextureMode[1]);
static tConfItem<int>   tm2("TEXTURE_MODE_2",rTextureGroups::TextureMode[2]);
static tConfItem<int>   tm3("TEXTURE_MODE_3",rTextureGroups::TextureMode[3]);

uMenu sg_screenMenu("$display_settings_menu");

static uMenu defaul("Are you sure?");

//uMenuItemSubmenu defaulm(&sg_screenMenu, &defaul, "$graphics_load_defaults_help");
uMenuItemFunction defaulm(&sg_screenMenu, "$graphics_load_defaults_text", "$graphics_load_defaults_help", [](){ static bool b = false; if( !b ) { defaul.ReverseItems(); b = true; } defaul.Enter(); });

static uMenuItemFunction defaul_ok
(&defaul,"Apply Defaults",
 "$graphics_load_defaults_help",
 [](){ sr_LoadDefaultConfig(); defaul.Exit(); });

uMenu screen_menu_detail("$detail_settings_menu");
uMenu screen_menu_tweaks("$performance_tweaks_menu");
uMenu screen_menu_prefs("$preferences_menu");
uMenu hud_prefs("$hud_menu");

static void sg_ScreenModeMenu();

static uMenuItemSubmenu smt(&sg_screenMenu,&screen_menu_tweaks,
                            "$performance_tweaks_menu_help");
static uMenuItemSubmenu smd(&sg_screenMenu,&screen_menu_detail,
                            "$detail_settings_menu_help");
static uMenuItemSubmenu smp(&sg_screenMenu,&screen_menu_prefs,
                            "$preferences_menu_help");
static uMenuItemFunction smm(&sg_screenMenu,"$screen_mode_menu",
                             "$screen_mode_menu_help", sg_ScreenModeMenu );

static uMenuItemSubmenu huds(&screen_menu_prefs,&hud_prefs,
                             "$hud_menu_help");


static tConfItemLine c_ext("GL_EXTENSIONS",gl_extensions);
static tConfItemLine c_ver("GL_VERSION",gl_version);
static tConfItemLine c_rEnd("GL_RENDERER",gl_renderer);
static tConfItemLine c_vEnd("GL_VENDOR",gl_vendor);
// static tConfItemLine a_ver("ARMAGETRON_VERSION",sn_programVersion);

static std::deque<tString> sg_consoleHistory; // global since the class doesn't live beyond the execution of the command
static int sg_consoleHistoryMaxSize=10; // size of the console history
static tSettingItem< int > sg_consoleHistoryMaxSizeConf("HISTORY_SIZE_CONSOLE",sg_consoleHistoryMaxSize);

class ArmageTron_feature_menuitem: public uMenuItemSelection<int>{
    void NewChoice(uSelectItem<bool> *){}
    void NewChoice(char *,bool ){}
public:
    ArmageTron_feature_menuitem(uMenu *m,char const * tit,char const * help,int &targ)
            :uMenuItemSelection<int>(m,tit,help,targ){
        uMenuItemSelection<int>::NewChoice(
            "$feature_disabled_text",
            "$feature_disabled_help",
            rFEAT_OFF);
        uMenuItemSelection<int>::NewChoice(
            "$feature_default_text",
            "$feature_default_help",
            rFEAT_DEFAULT);
        uMenuItemSelection<int>::NewChoice(
            "$feature_enabled_text",
            "$feature_enabled_help",
            rFEAT_ON);
    }

    ~ArmageTron_feature_menuitem(){}
};


class ArmageTron_texmode_menuitem: public uMenuItemSelection<int>{
    void NewChoice(uSelectItem<bool> *){}
    void NewChoice(char *,bool ){}
public:
    ArmageTron_texmode_menuitem(uMenu *m,char const * tit,int &targ,
                                bool font=false)
            :uMenuItemSelection<int>
    (m,tit,"$texture_menuitem_help",targ){

        if(!font)
            uMenuItemSelection<int>::NewChoice
            ("$texture_off_text","$texture_off_help",-1);
#ifndef DEDICATED
        uMenuItemSelection<int>::NewChoice
        ("$texture_nearest_text","$texture_nearest_help",GL_NEAREST);

        uMenuItemSelection<int>::NewChoice
        ("$texture_bilinear_text","$texture_bilinear_help",GL_LINEAR);

        if(!font)
        {
            uMenuItemSelection<int>::NewChoice
            ("$texture_mipmap_nearest_text",
             "$texture_mipmap_nearest_help",
             GL_NEAREST_MIPMAP_NEAREST);
            uMenuItemSelection<int>::NewChoice
            ("$texture_mipmap_bilinear_text",
             "$texture_mipmap_bilinear_help",
             GL_LINEAR_MIPMAP_NEAREST);
            uMenuItemSelection<int>::NewChoice
            ("$texture_mipmap_trilinear_text",
             "$texture_mipmap_trilinear_help",
             GL_LINEAR_MIPMAP_LINEAR);
        }
    #endif
    }

    ~ArmageTron_texmode_menuitem(){}
};

extern bool sg_TeamNumCount;

static tConfItem<bool>    ab("ALPHA_BLEND",sr_alphaBlend);
static tConfItem<bool>    ss("SMOOTH_SHADING",sr_smoothShading);
static tConfItem<bool>    to("TEXT_OUT",sr_textOut);
static tConfItem<bool>    fps("SHOW_FPS",sr_FPSOut);
static tConfItem<bool>    sta("SHOW_TEAMS_ALIVE",sg_TeamNumCount);
static tConfItem<bool>    pbt("SHOW_RECORDING_TIME",sr_RecordingTimeOut);
// tConfItem<> ("",&);
static tConfItem<int> fm("FLOOR_MIRROR",sr_floorMirror);
static tConfItem<int> fd("FLOOR_DETAIL",sr_floorDetail);
static tConfItem<bool> hr("HIGH_RIM",sr_highRim);
static tConfItem<bool> dt("DITHER",sr_dither);
static tConfItem<bool> us("UPPER_SKY",sr_upperSky);
static tConfItem<bool> ls("LOWER_SKY",sr_lowerSky);
static tConfItem<bool> wos("SKY_WOBBLE",sr_skyWobble);
static tConfItem<bool> ip("INFINITY_PLANE",sr_infinityPlane);

extern bool sg_axesIndicator;

static tConfItem<bool> lm("LAG_O_METER",sr_laggometer);
static tConfItem<bool> ai("AXES_INDICATOR",sg_axesIndicator);
bool sg_predictObjectsCmd = false;
static tConfItem<bool> po("PREDICT_OBJECTS",sg_predictObjectsCmd);
static tConfItem<bool> t32("TEXTURES_HI",sr_texturesTruecolor);

static tConfItem<bool> kwa("KEEP_WINDOW_ACTIVE",sr_keepWindowActive);
#ifdef USE_HEADLIGHT
static tConfItem<bool> chl("HEADLIGHT",headlights);
#endif

#ifndef SDL_OPENGL
#ifndef DIRTY
#define DIRTY
#endif
#endif

bool operator < ( rScreenSize const & a, rScreenSize const & b )
{
    return a.Compare(b) < 0;
}

class gResMenEntry
{
    uMenuItemSelection<rScreenSize> res_men; // menu item
    std::set< rScreenSize > sizes;           // set of already added modes

    // adds a single custom screen resolution
    void NewChoice( rScreenSize const & size )
    {
        if ( sizes.find( size ) == sizes.end() )
        {
            sizes.insert(size);
        }
    }

    // adds a single predefined resolution
    void NewChoice( rResolution res )
    {
        rScreenSize size( res );
        NewChoice( size );
    }

public:
    gResMenEntry( uMenu & screen_menu_mode, rScreenSize& res, const tOutput& text, const tOutput& help, bool addFixed )
            :res_men
            (&screen_menu_mode,
             text,
             help,
             res)
    {
#ifndef DEDICATED
        // fetch valid screen modes from SDL
        SDL_Rect **modes;
        modes=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_OPENGL);

        // Check is there are any modes available
        int i;
        if(modes == 0 || modes == (SDL_Rect **)-1)
        {
            // add all fixed resolutions
            for ( i = ArmageTron_Custom; i>=0; --i )
            {
                NewChoice( rResolution(i) );
            }
        }
        else
        {
            // add custom resolution
            NewChoice( ArmageTron_Custom );

            // add desktop resolution
            if ( sr_DesktopScreensizeSupported() && !addFixed )
                NewChoice( ArmageTron_Desktop );

            // the maximal allowed screen size
            rScreenSize maxSize(0,0);

            // fill in available modes (avoid duplicates)
            for(i=0;modes[i];++i)
            {
                // add mode (if it's new)
                rScreenSize size(modes[i]->w, modes[i]->h);
                NewChoice( size );
                if ( maxSize.width < size.width )
                    maxSize.width = size.width;
                if ( maxSize.height < size.height )
                    maxSize.height = size.height;
            }

            // add fixed resolutions (as window sizes)
            if ( addFixed )
            {
                for ( i = ArmageTron_Custom; i>=ArmageTron_Min; --i )
                {
                    rScreenSize size( static_cast< rResolution >(i) );

                    // only add those that fit the maximal resolution
                    if ( maxSize.height >= size.height && maxSize.width >= size.width )
                        NewChoice( size );
                }
            }
        }

        // insert sorted resolutions into menu
        for( std::set< rScreenSize >::iterator iter = sizes.begin(); iter != sizes.end(); ++iter )
            {
                rScreenSize const & size = *iter;

                std::stringstream s;
                if ( size.width + size.height > 0 )
                    s << size.width << " x " << size.height;
                else
                    s << tOutput("$screen_size_desktop");

                res_men.NewChoice( s.str().c_str(), "", size );
            }

#endif
    }
};

static void sg_ScreenModeMenu()
{
    uMenu screen_menu_mode("$screen_mode_menu");

    uMenuItemFunction appl
    (&screen_menu_mode,
     "$screen_apply_changes_text",
     "$screen_apply_changes_help",
     &sr_ReinitDisplay);

    uMenuItemToggle kwa_t(
        &screen_menu_mode,
        "$screen_keep_window_active_text",
        "$screen_keep_window_active_help",
        sr_keepWindowActive);

    uMenuItemToggle ie_t
    (&screen_menu_mode,
     "$screen_check_errors_text",
     "$screen_check_errors_help",
     currentScreensetting.checkErrors);


#ifdef SDL_OPENGL
#ifdef DIRTY
    uMenuItemToggle sdl_t
    (&screen_menu_mode,
     "$screen_use_sdl_text",
     "$screen_use_sdl_help",
     currentScreensetting.useSDL);
#endif
#endif

    uMenuItemToggle gm(
        &screen_menu_mode,
        "$screen_grab_mouse_text",
        "$screen_grab_mouse_help",
        su_mouseGrab);


    /* massive troll
    uMenuItemSelection<rColorDepth> zd_t
    (&screen_menu_mode,
     "$screen_zdepth_text",
     "$screen_zdepth_help",
     currentScreensetting.zDepth);

    uSelectEntry<rColorDepth> zd_16(zd_t,"$screen_zdepth_16_text","$screen_zdepth_16_help",ArmageTron_ColorDepth_16);
    uSelectEntry<rColorDepth> zd_d(zd_t,"$screen_zdepth_desk_text","$screen_zdepth_desk_help",ArmageTron_ColorDepth_Desktop);
    uSelectEntry<rColorDepth> zd_32(zd_t,"$screen_zdepth_32_text","$screen_zdepth_32_help",ArmageTron_ColorDepth_32);
    */

    uMenuItemSelection<rColorDepth> cd_t
    (&screen_menu_mode,
     "$screen_colordepth_text",
     "$screen_colordepth_help",
     currentScreensetting.colorDepth);

    // uMenuItemSelection<REAL> b_t
    // (&screen_menu_mode,
    //  "Brightness",
    //  "Brightness",
    //  currentScreensetting.brightness);

    uSelectEntry<rColorDepth> cd_16(cd_t,"$screen_colordepth_16_text","$screen_colordepth_16_help",ArmageTron_ColorDepth_16);
    uSelectEntry<rColorDepth> cd_d(cd_t,"$screen_colordepth_desk_text","$screen_colordepth_desk_help",ArmageTron_ColorDepth_Desktop);
    uSelectEntry<rColorDepth> cd_32(cd_t,"$screen_colordepth_32_text","$screen_colordepth_32_help",ArmageTron_ColorDepth_32);

    uMenuItemToggle fs_t
    (&screen_menu_mode,
     "$screen_fullscreen_text",
     "$screen_fullscreen_help",
     currentScreensetting.fullscreen);


    gResMenEntry res( screen_menu_mode, currentScreensetting.res, "$screen_resolution_text", "$screen_resolution_help", false );
    gResMenEntry winsize( screen_menu_mode, currentScreensetting.windowSize, "$window_size_text", "$window_size_help", true );

    /*
    uMenuItemSelection<rResolution> res_men
    (&screen_menu_mode,
     "$screen_resolution_text",
     "$screen_resolution_help",
     currentScreensetting.res);


    uSelectEntry<rResolution> a(res_men,"320x200","",ArmageTron_320_200);
     static uSelectEntry<rResolution> b(res_men,"320x240","",ArmageTron_320_240);
    static uSelectEntry<rResolution> c(res_men,"400x300","",ArmageTron_400_300);
    static uSelectEntry<rResolution> d(res_men,"512x384","",ArmageTron_512_384);
    static uSelectEntry<rResolution> e(res_men,"640x480","",ArmageTron_640_480);
    static uSelectEntry<rResolution> f(res_men,"800x600","",ArmageTron_800_600);
    static uSelectEntry<rResolution> g(res_men,"1024x768","",ArmageTron_1024_768);
    static uSelectEntry<rResolution> h(res_men,"1280x1024","",ArmageTron_1280_1024);
    static uSelectEntry<rResolution> i(res_men,"1600x1200","",ArmageTron_1600_1200);
    static uSelectEntry<rResolution> j(res_men,"2048x1572","",ArmageTron_2048_1572);
    static uSelectEntry<rResolution> jj(res_men,
    				    "$screen_custom_text",
    				    "$screen_custom_help"
    				    ,ArmageTron_Custom);
    */

    screen_menu_mode.Enter();
}


static uMenuItemSelection<int> mfm
(&screen_menu_detail,
 "$detail_floor_mirror_text",
 "$detail_floor_mirror_help",
 sr_floorMirror);
static uSelectEntry<int> mfma(mfm,"$detail_floor_mirror_off_text","$detail_floor_mirror_off_help",rMIRROR_OFF);


static uSelectEntry<int> mfmb(mfm,"$detail_floor_mirror_obj_text",
                              "$detail_floor_mirror_obj_help",
                              rMIRROR_OBJECTS);

static uSelectEntry<int> mfmc(mfm,"$detail_floor_mirror_ow_text",
                              "$detail_floor_mirror_ow_help",
                              rMIRROR_WALLS);

static uSelectEntry<int> mfme(mfm,"$detail_floor_mirror_ev_text","$detail_floor_mirror_ev_help",rMIRROR_ALL);

static uMenuItemToggle fs_dither
(&screen_menu_detail,"$detail_dither_text",
 "$detail_dither_help",
 sr_dither);

#ifndef DEDICATED
// from gWall.cpp
extern bool sg_simpleTrail;
static uMenuItemToggle sgm_simpleTrail
(&screen_menu_detail,
 "$detail_simple_trail_text",
 "$detail_simple_trail_help",
 sg_simpleTrail);
#endif

static uMenuItemSelection<int> mfd
(&screen_menu_detail,
 "$detail_floor_text",
 "$detail_floor_help",
 sr_floorDetail);

static uSelectEntry<int> mfda(mfd,"$detail_floor_no_text",
                              "$detail_floor_no_help",
                              rFLOOR_OFF);
static uSelectEntry<int> mfdb(mfd,"$detail_floor_grid_text",
                              "$detail_floor_grid_help",
                              rFLOOR_GRID);
static uSelectEntry<int> mfdc(mfd,"$detail_floor_tex_text",
                              "$detail_floor_tex_help",
                              rFLOOR_TEXTURE);
static uSelectEntry<int> mfdd(mfd,"$detail_floor_2tex_text",
                              "$detail_floor_2tex_help",
                              rFLOOR_TWOTEXTURE);

extern bool sg_zoneAlphaBlend;
uMenuItemToggle azbm
(&screen_menu_detail,"Zone Alpha Blend:",
 "$zone_alpha_toggle_help",sg_zoneAlphaBlend);

static uMenuItemToggle  abm
(&screen_menu_detail,"$detail_alpha_text",
 "$detail_alpha_help",
 sr_alphaBlend);

static uMenuItemToggle  ssm
(&screen_menu_detail,"$detail_smooth_text",
 "$detail_smooth_help",
 sr_smoothShading);

extern bool crash_sparks;		// from gCycle.cpp
extern bool white_sparks;		// from gSparks.cpp
static tConfItem<bool> cs2("SPARKS",crash_sparks);
static tConfItem<bool> wsp("WHITE_SPARKS",white_sparks);

extern bool sg_crashExplosion;   // from gExplosion.cpp
static tConfItem<bool> crexp("EXPLOSION",sg_crashExplosion);

#ifndef DEDICATED
// both from ePlayer.cpp
static tConfItem<bool> se_highlightMyNameConf("HIGHLIGHT_NAME", se_highlightMyName);

static tConfItem<bool> se_tabCompletionConf("TAB_COMPLETION", se_tabCompletion);
static tConfItem<bool> se_tabCompletionColors("TAB_COMPLETION_WITH_COLORS", se_tabCompletionWithColors);
static tConfItem<bool> se_autoComplColor("AUTO_COMPLETE_WITH_COLOR", se_tabCompletionWithColors);

extern void sg_ColorMenu(), se_ForcePlayerColorMenu();

void sg_SpecialMenu()
{
    uMenu menu("$special_setup_menu_text");

    uMenuItemFunction opc(&menu, "Override Players' Colors", "", se_ForcePlayerColorMenu);

    uMenuItemFunction cm(&menu, "My Color Configuration", "", sg_ColorMenu);

    uMenuItemToggle hlm(&menu, "$highlight_name_menu_text", "$highlight_name_menu_help", se_highlightMyName);
    uMenuItemToggle tcwc(&menu, "$tab_completion_with_colors_menu_text", "$tab_completion_with_colors_menu_help", se_tabCompletionWithColors);
    uMenuItemToggle tc(&menu, "$tab_completion_menu_text", "$tab_completion_menu_help", se_tabCompletion);

    uMenuItemToggle hcw(&menu, "$hide_cycles_walls_menu_text", "$hide_cycles_walls_menu_help", sg_HideCyclesWalls);
    uMenuItemToggle hc(&menu, "$hide_cycles_menu_text", "$hide_cycles_menu_help", sg_HideCycles);

    menu.Enter();
}


class gMenuItemSubmenu: public uMenuItemSubmenu
{
    uMenu * submenu;
public:
    bool disabled;

    gMenuItemSubmenu(uMenu *M, uMenu *s, const tOutput& help):
        uMenuItemSubmenu( M, s, help )
    {
        submenu = s;
        this->disabled = false;
    }
    virtual ~gMenuItemSubmenu(){}

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0)
    {
        if( disabled )
        {
            alpha *= 0.5;
        }

        DisplayTextSpecial(x,y,submenu->title,selected,alpha,0);
    }

    virtual bool IsSelectable()
    {
        return !disabled;
    }
};


extern tString settingsDownloadCfg, st_configChanged;
extern nMessage * se_NewChatMessage( ePlayerNetID const * player, tString const & message );
extern tString st_AddToUserExt( tArray<tString> commands );
static tString sg_c_setting, sg_c_svr_sets, sg_c_svr_cfg_in;
void sg_ConfigMenu()
{
    uMenu menu("$config_setup_menu_text");

    uMenu add_to_cfg("Add Persistant Setting Value");

    uMenuItemFunction atc_b(&add_to_cfg,
        "Save",
        "",
        []()
        {
            tArray< tString > commands; commands.Insert( sg_c_setting );

            auto outln = st_AddToUserExt( commands ).Split("\n");
            tString out;

            for(int i=0;i<outln.Len();++i)
            {
                if( outln[i].StartsWith("Done") )
                {
                    sg_c_setting = "";
                    continue;
                }
                out << outln[i] << "\n";
            }

            uMenu::Message( tString("Done"), tString( out ), 12 );
        });

    uMenuItemString atc_s(&add_to_cfg,
        "Setting",
        "The setting to save",
        sg_c_setting);

    uMenuItemSubmenu cm(&menu,&add_to_cfg,
                        "persist setting to config that will be auto updated on change");

    uMenuItemFunction sac(&menu, "$config_save_all_text", "$config_save_all_help", &tConfItemBase::WriteAllToFile);
    uMenuItemFunction lac(&menu, "$config_load_all_text", "$config_load_all_help", &st_LoadConfig);




    uMenu dlsets("Save Server Settings");
    uMenuItemFunction gfs(&dlsets, "Save", "", [](){ se_NewChatMessage(se_GetLocalPlayer(), tString("/dlsettings"))->BroadCast(); });
    uMenuItemString gfs_s(&dlsets,
        "Filename",
        "",
        settingsDownloadCfg);


    uMenu dlcfg("Download Public Config File");
    uMenuItemFunction sfc(&dlcfg, "Download", "download public config from server", []()
    {
        tString cmd("/dlcfg ");
        cmd << sg_c_svr_cfg_in;

        se_NewChatMessage(se_GetLocalPlayer(), cmd)->BroadCast();
    });
    uMenuItemString sfc_s(&dlcfg,
        "Filename",
        "",
        sg_c_svr_cfg_in);


    gMenuItemSubmenu dcsm( &menu, &dlcfg,  "" );
    gMenuItemSubmenu dssm( &menu, &dlsets, "" );

    if(!(
        sn_GetNetState() == nCLIENT && sn_Connections[0].version.Max() == 18 && se_GetLocalPlayer()
    ))
    {
        dcsm.disabled = true;
        dssm.disabled = true;
    }

    uMenu sccM("$config_save_changed_text");
    uMenuItemFunction sccf(&sccM, "Save", "", &tConfItemBase::WriteChangedToFile);
    uMenuItemString scc_s(&sccM,
        "Filename",
        "",
        st_configChanged);

    uMenuItemSubmenu scc_m(&menu, &sccM, "$config_save_changed_help");

    /*
    uMenuItemFunction suc(&menu, "$config_user_save_text", "$config_user_save_help", &st_SaveConfig);
    uMenuItemFunction luc(&menu, "$config_user_load_text", "$config_user_load_help", &st_LoadUserConfig);
    */

    menu.Enter();
}

//extern bool png_screenshot;		// from rSysdep.cpp
//static tConfItem<bool> pns("PNG_SCREENSHOT",png_screenshot);

static uMenuItemToggle  t32b
(&screen_menu_detail,"$detail_text_truecolor_text",
 "$detail_text_truecolor_help"
 ,sr_texturesTruecolor);


static ArmageTron_texmode_menuitem tmm0(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[0],
                                        rTextureGroups::TextureMode[0]);

static ArmageTron_texmode_menuitem tmm1(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[1],
                                        rTextureGroups::TextureMode[1]);

static ArmageTron_texmode_menuitem tmm2(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[2],
                                        rTextureGroups::TextureMode[2]);

static ArmageTron_texmode_menuitem tmm3(&screen_menu_detail,
                                        rTextureGroups::TextureGroupDescription[3],
                                        rTextureGroups::TextureMode[3],true);


uMenuItemToggle bpt2
(&screen_menu_prefs,"$misc_recording_time_text",
 "$misc_recording_time_help",sr_RecordingTimeOut);


class uMenuItemBrightness: public uMenuItemReal
{
public:
    uMenuItemBrightness( uMenu * m, const tOutput &title, const tOutput &help, REAL &targ )
        :uMenuItemReal( m, title, help, targ, 0, 4, 0.1)
    {
    };

    void Render(REAL x, REAL y, REAL alpha, bool selected)
    {
        DisplayText(x-.02,y,title,selected,alpha,1);

        tString s; s << roundf( target * 100 ) << "%";
        DisplayText(x+.02,y,s,selected,alpha,-1);
    }
};

extern float sr_brightness;
uMenuItemBrightness brightnessConf( &screen_menu_prefs, "Brightness", "", sr_brightness );

static uMenuItemToggle s2
(&screen_menu_prefs,"$pref_highrim_text",
 "$pref_highrim_help",sr_highRim);

static uMenuItemToggle us2
(&screen_menu_prefs,"$pref_uppersky_text",
 "$pref_uppersky_help",
 sr_upperSky);

static uMenuItemToggle ls2
(&screen_menu_prefs,"$pref_lowersky_text",
 "$pref_lowersky_help",
 sr_lowerSky);

uMenuItemToggle rtnc2
(&screen_menu_prefs,"Show Teams Alive Count:",
 "",sg_TeamNumCount);

uMenuItemToggle fps2
(&screen_menu_prefs,"$misc_fps_text",
 "$misc_fps_help",sr_FPSOut);

#ifdef USE_HEADLIGHT
static uMenuItemToggle uchl(&screen_menu_prefs,"$pref_headlight_text","$pref_headlight_help",
                            headlights);
#endif


static uMenuItemToggle ws2
(&screen_menu_prefs,"$pref_skymove_text",
 "$pref_skymove_help",
 sr_skyWobble);

static uMenuItemToggle crexp2
(&screen_menu_prefs,"$pref_explosion_text",
 "$pref_explosion_help",
 sg_crashExplosion);

static uMenuItemToggle cs
(&screen_menu_prefs,"$pref_sparks_text",
 "$pref_sparks_help",
 crash_sparks);

static uMenuItemSelection<rDisplayListUsage> dl
(&screen_menu_tweaks,"$tweaks_displaylists_text",
 "$tweaks_displaylists_help", sr_useDisplayLists);
static uSelectEntry<rDisplayListUsage> dl_off(dl,"$tweaks_displaylists_off_text","$tweaks_displaylists_off_help",rDisplayList_Off);
static uSelectEntry<rDisplayListUsage> dl_cac(dl,"$tweaks_displaylists_cac_text","$tweaks_displaylists_cac_help",rDisplayList_CAC);
static uSelectEntry<rDisplayListUsage> dl_cae(dl,"$tweaks_displaylists_cae_text","$tweaks_displaylists_cae_help",rDisplayList_CAE);

static uMenuItemToggle infp
(&screen_menu_tweaks,"$tweaks_infinity_text",
 "$tweaks_infinity_help"
 ,sr_infinityPlane);

uMenuItemSelection<rSysDep::rSwapMode> swapMode
(&screen_menu_tweaks,
 "$swapmode_text",
 "$swapmode_help",
 rSysDep::swapMode_);

static uSelectEntry<rSysDep::rSwapMode> swapMode_fastest(swapMode,"$swapmode_fastest_text","$swapmode_fastest_help",rSysDep::rSwap_Fastest);
static uSelectEntry<rSysDep::rSwapMode> swapMode_glFlush(swapMode,"$swapmode_glflush_text","$swapmode_glflush_help",rSysDep::rSwap_glFlush);
static uSelectEntry<rSysDep::rSwapMode> swapMode_glFinish(swapMode,"$swapmode_glfinish_text","$swapmode_glfinish_help",rSysDep::rSwap_glFinish);

/*
uMenuItemSelection<int> targetFPS
(&screen_menu_tweaks,
 "$targetfps_text",
 "$targetfps_help",
rSysDep::swapMode_);

static uSelectEntry<rSysDep::rSwapMode> swapMode_150Hz(swapMode,"$swapmode_150hz_text","$swapmode_150hz_help",rSysDep::rSwap_150Hz);
static uSelectEntry<rSysDep::rSwapMode> swapMode_100Hz(swapMode,"$swapmode_100hz_text","$swapmode_100hz_help",rSysDep::rSwap_100Hz);
static uSelectEntry<rSysDep::rSwapMode> swapMode_80Hz(swapMode,"$swapmode_80hz_text","$swapmode_80hz_help",rSysDep::rSwap_80Hz);
static uSelectEntry<rSysDep::rSwapMode> swapMode_60Hz(swapMode,"$swapmode_60hz_text","$swapmode_60hz_help",rSysDep::rSwap_60Hz);
*/

class uMenuItemFPS: public uMenuItemInt
{
public:
    uMenuItemFPS( uMenu * m, const tOutput &title, const tOutput &help, int &targ )
        :uMenuItemInt( m, title, help, targ, 1, 8000)
    {
    };

    virtual void LeftRight( int dir )
    {
        static int fps[] = {
            24, 30, 40, 60, 120, 144, 240,
            300, 400, 500, 600, 800,
            1000, 2000, 4000, 8000,
        };
        static const size_t fpses = ( sizeof(fps) / sizeof(int) );

        size_t curr = 0;

        int lastdiff = 9999999;
        for(size_t i=0;i<fpses;++i)
        {
            int diff = abs( fps[i] - target );
            if( diff > lastdiff )
                break;

            curr = i;
            lastdiff = diff;
        }

        if( dir > 0 && curr < ( fpses - 1 ) )
            target = fps[curr + 1];
        else if( dir < 0 && curr > 0 )
            target = fps[curr - 1];
    };
};

extern int sr_maxFPS;
uMenuItemFPS targetFPS
(&screen_menu_tweaks,
 "Target FPS",
 "The framerate the game tries to keep at or around",
sr_maxFPS);

tCONFIG_ENUM( rSysDep::rSwapMode );

static tConfItem< rSysDep::rSwapMode > swapModeCI("SWAP_MODE", rSysDep::swapMode_ );

static tConfItem<REAL> sgs("HUD_SPEED_GAUGE_SIZE",subby_SpeedGaugeSize);
static tConfItem<REAL> sgx("HUD_SPEED_GAUGE_LOCX",subby_SpeedGaugeLocX);
static tConfItem<REAL> sgy("HUD_SPEED_GAUGE_LOCY",subby_SpeedGaugeLocY);

static tConfItem<bool> wt("HUD_WALL_TIME",hud_WallTime);
static tConfItem<bool> wtl("HUD_WALL_TIME_LABEL",hud_WallTimeLabel);
static tConfItem<REAL> wts("HUD_WALL_TIME_SIZE",hud_WallTimeSize);
static tConfItem<REAL> wtx("HUD_WALL_TIME_LOCX",hud_WallTimeLocX);
static tConfItem<REAL> wty("HUD_WALL_TIME_LOCY",hud_WallTimeLocY);


static tConfItem<REAL> bgs("HUD_BRAKE_GAUGE_SIZE",subby_BrakeGaugeSize);
static tConfItem<REAL> bgx("HUD_BRAKE_GAUGE_LOCX",subby_BrakeGaugeLocX);
static tConfItem<REAL> bgy("HUD_BRAKE_GAUGE_LOCY",subby_BrakeGaugeLocY);

static tConfItem<REAL> rgs("HUD_RUBBER_GAUGE_SIZE",subby_RubberGaugeSize);
static tConfItem<REAL> rgx("HUD_RUBBER_GAUGE_LOCX",subby_RubberGaugeLocX);
static tConfItem<REAL> rgy("HUD_RUBBER_GAUGE_LOCY",subby_RubberGaugeLocY);

static tConfItem<bool> showh("HUD_SHOW",subby_ShowHUD);
static tConfItem<bool> showf("HUD_SHOW_FASTEST",subby_ShowSpeedFastest);
static tConfItem<bool> shows("HUD_SHOW_SCORE",subby_ShowScore);
static tConfItem<bool> showae("HUD_SHOW_ALIVE",subby_ShowAlivePeople);
static tConfItem<bool> showp("HUD_SHOW_PING",subby_ShowPing);
static tConfItem<bool> showsm("HUD_SHOW_SPEED",subby_ShowSpeedMeter);
static tConfItem<bool> showbm("HUD_SHOW_BRAKE",subby_ShowBrakeMeter);
static tConfItem<bool> showrm("HUD_SHOW_RUBBER",subby_ShowRubberMeter);
static tConfItem<bool> showtim("HUD_SHOW_TIME",hudShowTime);
static tConfItem<bool> show24("HUD_SHOW_TIME_24",hudShowTime24hour);
static tConfItem<bool> showpos("HUD_SHOW_POSITION",showPosition);

static tConfItem<REAL> scorex("HUD_SCORE_LOCX",subby_ScoreLocX);
static tConfItem<REAL> scorey("HUD_SCORE_LOCY",subby_ScoreLocY);
static tConfItem<REAL> scores("HUD_SCORE_SIZE",subby_ScoreSize);

static tConfItem<REAL> fastx("HUD_FASTEST_LOCX",subby_FastestLocX);
static tConfItem<REAL> fasty("HUD_FASTEST_LOCY",subby_FastestLocY);
static tConfItem<REAL> fasts("HUD_FASTEST_SIZE",subby_FastestSize);

static tConfItem<REAL> aex("HUD_ALIVE_LOCX",subby_AlivePeopleLocX);
static tConfItem<REAL> aey("HUD_ALIVE_LOCY",subby_AlivePeopleLocY);
static tConfItem<REAL> aes("HUD_ALIVE_SIZE",subby_AlivePeopleSize);

static tConfItem<REAL> px("HUD_PING_LOCX",subby_PingLocX);
static tConfItem<REAL> py("HUD_PING_LOCY",subby_PingLocY);
static tConfItem<REAL> ps("HUD_PING_SIZE",subby_PingSize);

static tConfItem<REAL> positionx("HUD_POSITION_LOCX",subby_CoordLocX);
static tConfItem<REAL> positiony("HUD_POSITION_LOCY",subby_CoordLoxY);
static tConfItem<REAL> positions("HUD_POSITION_SIZE",subby_CoordSize);

uMenuItemToggle hud3
(&hud_prefs,"$pref_showfastest_text",
 "$pref_showfastest_help",subby_ShowSpeedFastest);

uMenuItemToggle mud4
(&hud_prefs,"$pref_showscore_text",
 "$pref_showscore_help",subby_ShowScore);

uMenuItemToggle hud5
(&hud_prefs,"$pref_showenemies_text",
 "$pref_showenemies_help",subby_ShowAlivePeople);

uMenuItemToggle hud6
(&hud_prefs,"$pref_showping_text",
 "$pref_showping_help",subby_ShowPing);

uMenuItemToggle hud7
(&hud_prefs,"$pref_showspeed_text",
 "$pref_showspeed_help",subby_ShowSpeedMeter);

uMenuItemToggle hud8
(&hud_prefs,"$pref_showbrake_text",
 "$pref_showbrake_help",subby_ShowBrakeMeter);

uMenuItemToggle hud9
(&hud_prefs,"$pref_showrubber_text",
 "$pref_showrubber_help",subby_ShowRubberMeter);

uMenuItemToggle hud10
(&hud_prefs,"$pref_showtime_text",
 "$pref_showtime_help",hudShowTime);

uMenuItemToggle hud11
(&hud_prefs,"$pref_show24hour_text",
 "$pref_show24hour_help",hudShowTime24hour);

extern int simplemapmode;
uMenuItemToggle hud12
(&hud_prefs,"Show Minimap",
 "",*(reinterpret_cast<bool *>(&simplemapmode)));

uMenuItemToggle hud2
(&hud_prefs,"$pref_showhud_text",
 "$pref_showhud_help",subby_ShowHUD);

static tConfItem<bool> WRAP("WRAP_MENU",uMenu::wrap);

#include <deque>
#include <algorithm>


REAL sg_consoleTabCompletionMaxPossibilities = 50;
static tConfItem< REAL > sg_consoleTabCompletionMaxPossibilitiesConf("CONSOLE_TAB_COMPLETION_MAX_POSSIBILITIES",sg_consoleTabCompletionMaxPossibilities);


static tString Simplify(const tString &str) {
    return str.ToLower();
}

static void ShowPossibilities(const std::deque<tString> &results, const tString &word) {
    if (results.size() > sg_consoleTabCompletionMaxPossibilities) {
        con << "Too many results found (" << results.size()
            << " > " << sg_consoleTabCompletionMaxPossibilities
            << "). Type more characters.\n";
    } else {
        con << "Possibilities:\n";
        int len = word.Len();
        for (const auto &result : results) {
            int pos = result.ToLower().StrPos(word.ToLower());
            con << result.SubStr(0, pos)
                << "0xff8888"
                << result.SubStr(pos, len)
                << "0xffffff"
                << result.SubStr(pos + len)
                << " ";
        }
        con << "\n";
    }
}

static void FindConfigItems(const tString &word, std::deque<tString> &results) {
    tConfItemBase::tConfItemMap const &itemsMap = tConfItemBase::GetConfItemMap();
    for (const auto &entry : itemsMap) {
        tConfItemBase *item = entry.second;
        if (item->GetTitle().ToLower().StartsWith(word.ToLower())) {
            results.push_back(item->GetTitle());
        }
    }
}

static tString FindClosestMatch(const tString &word, const std::deque<tString> &items) {
    tString closestMatch;
    int maxCommonPrefix = 0;
    for (const auto &itemName : items) {
        int commonPrefix = 0;
        for (int i = 0; i < itemName.Len() && i < word.Len(); ++i) {
            if (tolower(itemName(i)) == tolower(word(i))) {
                commonPrefix++;
            } else {
                break;
            }
        }
        if (commonPrefix > maxCommonPrefix) {
            maxCommonPrefix = commonPrefix;
            closestMatch = itemName;
        }
    }
    return closestMatch;
}

static void ConTabCompletition(tString &strString, int &cursorPos, bool changeLast) {
    static tString oldString;
    static int cfgPos;
    static int lastPos;
    static bool showResults; // Added a new variable to track whether to show the results
    if (changeLast) {
        strString = oldString;
        cfgPos++;
        cursorPos = lastPos;
    } else {
        oldString = strString;
        lastPos = cursorPos;
        cfgPos = 0;
        showResults = true; // Set showResults to true when a new string is typed
    }

    tArray<tString> msgsExt = strString.Split(" ");
    tString newString;
    int cusPos = 0;

    for (int i = 0; i < msgsExt.Len(); i++) {
        tString word = msgsExt[i];

        cusPos += word.Len() - 1;

        if (cusPos == cursorPos) {
            // Find possible words
            tString simplifiedWord = Simplify(word);
            std::deque<tString> results;
            FindConfigItems(simplifiedWord, results);

            if (results.size() > 1) {
                // Use cfgPos to select the next match in the list
                tString match = results.at(cfgPos % results.size());

                if (match == simplifiedWord) {
                    if (showResults) {
                        ShowPossibilities(results, word);
                        showResults = false;
                    }
                    cursorPos = cusPos + 1;
                } else {
                    if (showResults) {
                        ShowPossibilities(results, word);
                        showResults = false;
                    }
                    word = match;
                    cursorPos = cusPos + match.Len() + 1;
                }
            } else if (!results.empty()) {
                word = results.front() + " ";
                cursorPos = cusPos + word.Len() + 1;
            } else {
                cfgPos = -1;
            }
        }

        cusPos++;

        if ((i + 1) == msgsExt.Len())
            newString << word;
        else
            newString << word << " ";
    }

    strString = newString;
}

class gMemuItemConsole: uMenuItemStringWithHistory{
public:
    gMemuItemConsole(uMenu *M,tString &c):
    uMenuItemStringWithHistory(M,"Con:","", c, 1024, sg_consoleHistory, sg_consoleHistoryMaxSize) {}

    virtual ~gMemuItemConsole(){}

    //virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0);

    virtual bool Event(SDL_Event &e){
        if (e.type==SDL_KEYDOWN &&
                (e.key.keysym.sym==SDLK_KP_ENTER || e.key.keysym.sym==SDLK_RETURN)){

            con << tColoredString::ColorString(.5,.5,1) << " > " << *content << '\n';

            // direct commands are executed at owner level
            tCurrentAccessLevel level( tAccessLevel_Owner, true );

            // pass the console command to the configuration system
            std::stringstream s(&((*content)[0]));
            tConfItemBase::LoadAllFromMenu( s );

            MyMenu()->Exit();
            return true;
        }
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_TAB)
        {
            if ((*content != "") && se_tabCompletion)
            {
                tString strString;
                strString << *content;

                static tString lastContent;
                bool changeLast = (lastContent == strString);

                ConTabCompletition(strString, cursorPos, changeLast);
                *content = strString;

                lastContent = strString;
            }
            return true;
        }
        else if (e.type==SDL_KEYDOWN &&
                 uActionGlobal::IsBreakingGlobalBind(e.key.keysym.sym))
            return su_HandleEvent(e, true);
        else
            return uMenuItemStringWithHistory::Event(e);
    }
};

void do_con(){
    su_ClearKeys();

    se_ChatState( ePlayerNetID::ChatFlags_Console, true );
    sr_con.SetHeight(20,false);
    se_SetShowScoresAuto(false);
    tString c;

    uMenu con_menu("",false);
    gMemuItemConsole s(&con_menu,c);
    con_menu.SetCenter(-.75);
    con_menu.SetBot(-2);
    con_menu.SetTop(-.7);
    con_menu.Enter();

    se_ChatState( ePlayerNetID::ChatFlags_Console, false );

    se_SetShowScoresAuto(true);
    sr_con.SetHeight(7,false);
}
#endif

void sg_ConsoleInput(){
#ifndef DEDICATED
    st_ToDoOnce(&do_con);
#endif
}















class ArmageTron_viewport_menuitem:public uMenuItemInt{
public:
    ArmageTron_viewport_menuitem(uMenu *m):
            uMenuItemInt(m,"$viewport_menu_title",
                         "$viewport_menu_help",
                         rViewportConfiguration::next_conf_num,
                 0,rViewportConfiguration::s_viewportNumConfigurations-1){
        m->RequestSpaceBelow(.9);
    }

    virtual REAL SpaceRight(){return 1;}

    virtual void RenderBackground(){
        uMenuItem::RenderBackground();

        if (rViewportConfiguration::next_conf_num<0) rViewportConfiguration::next_conf_num=0;
        if (rViewportConfiguration::next_conf_num>=rViewportConfiguration::s_viewportNumConfigurations)
            rViewportConfiguration::next_conf_num=rViewportConfiguration::s_viewportNumConfigurations-1;

        tString  titles[MAX_VIEWPORTS];

        for(int i=MAX_VIEWPORTS-1;i>=0;i--)
            titles[i] << i+1;
#ifndef DEDICATED
        rViewportConfiguration::DemonstrateViewport(titles);
#endif
    }

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0){
        if (rViewportConfiguration::next_conf_num<0) rViewportConfiguration::next_conf_num=0;
        if (rViewportConfiguration::next_conf_num>=rViewportConfiguration::s_viewportNumConfigurations)
            rViewportConfiguration::next_conf_num=rViewportConfiguration::s_viewportNumConfigurations-1;

        tOutput disp;

        disp << "$viewport_conf_text";
        disp.AddSpace();
        disp << rViewportConfiguration::s_viewportConfigurationNames[rViewportConfiguration::next_conf_num];
        DisplayText(x-.02,y,disp,selected,alpha);
    }

};



class ArmageTronPlayer_to_viewport_menuitem:public uMenuItemInt{
    int    vp;
public:
    ArmageTronPlayer_to_viewport_menuitem(uMenu *m,int v):
            uMenuItemInt(m,"",
                         "$viewport_assign_help",
                         s_newViewportBelongsToPlayer[v],
                 0,MAX_PLAYERS-1),vp(v){
        m->RequestSpaceBelow(.9);
    }

    virtual REAL SpaceRight(){return 1;}

    virtual void LeftRight(int x){
        rViewport::SetDirectionOfCorrection(vp,x);
        target=(target + MAX_PLAYERS + x) % MAX_PLAYERS;
    }

    virtual void RenderBackground(){
        rViewport::CorrectViewport(vp, MAX_PLAYERS);

        uMenuItem::RenderBackground();

        tString titles[MAX_VIEWPORTS];
        for(int i=MAX_VIEWPORTS-1;i>=0;i--){
            titles[i] << s_newViewportBelongsToPlayer[i]+1;
            titles[i] << " (";
            titles[i] << ePlayer::PlayerConfig(s_newViewportBelongsToPlayer[i])->Name();
            titles[i] << ")";
        }
#ifndef DEDICATED
        rViewportConfiguration::DemonstrateViewport(titles);
#endif
    }

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0){

        tOutput disp;

        disp.SetTemplateParameter(1, vp +1);
        disp.SetTemplateParameter(2, s_newViewportBelongsToPlayer[vp]+1);
        disp.SetTemplateParameter(3, ePlayer::PlayerConfig(s_newViewportBelongsToPlayer[vp])->Name());
        disp << "$viewport_belongs_text";

        DisplayText(x-.02,y,disp,selected,alpha);
    }

};

#include "rSysdep.h"
extern void Render(int);



REAL sg_playerColorMenuMax = 15;
static tConfItem< REAL > sg_playerColorMenuMaxConf("PLAYER_COLOR_MENU_MAX",sg_playerColorMenuMax);

class ArmageTron_color_menuitem:public uMenuItemInt{
protected:
    int *rgb;
    unsigned short me;
public:
    ArmageTron_color_menuitem(uMenu *m,const char *tit,
                              const char *help, int *RGB,int Me)
            :uMenuItemInt(m,tit,help,RGB[Me],0,sg_playerColorMenuMax),
    rgb(RGB),me(Me) {
        m->RequestSpaceBelow(.2);
    }

    ~ArmageTron_color_menuitem(){}

    virtual REAL SpaceRight(){return .2;}


    virtual void RenderBackground(){
        //    static int count=0;
        /*
        while(rgb[0]+rgb[1]+rgb[2]<13){
          if (rgb[count]<15)
        rgb[count]++;
          count++;
          if (count>2)
        count=0;
        }
        */
#ifndef DEDICATED
        uMenuItem::RenderBackground();
        if (!sr_glOut)
            return;
        REAL r = rgb[0]/15.0;
        REAL g = rgb[1]/15.0;
        REAL b = rgb[2]/15.0;

        REAL sr = r, sg = g, sb = b;

        se_MakeColorValid(r, g, b, 1.0f);
        RenderEnd();
        glColor3f(r, g, b);
        //glRectf(.8,-.8,.98,-.71);
        glRectf(.8,-.8,.98,-.98);

        while( sr > 1.f ) sr -= 1.f;
        while( sg > 1.f ) sg -= 1.f;
        while( sb > 1.f ) sb -= 1.f;

        glColor3f(sr, sg, sb);
        glRectf(-.8,-.8,-.98,-.98);
#endif
    }

};



void sg_PlayerMenu(int Player){
    tOutput name;
    name.SetTemplateParameter(1, Player+1);

    name << "$player_menu_text";


    uMenu playerMenu(name);

    uMenu camera_menu("$player_camera_text");
    uMenu chat_menu("$player_chat_text");
    uMenu chat_menu2("List chats only");
    //  name.Clear();
    chat_menu.SetCenter(-.5);
    chat_menu2.SetCenter(-.5);

    uMenuItemString *ic[MAX_INSTANT_CHAT];
    uMenuItemInput * ici[MAX_INSTANT_CHAT];
    uMenuItemDivider * icd[MAX_INSTANT_CHAT];

    uMenuItemString * ic2[MAX_INSTANT_CHAT];

    ePlayer *p = ePlayer::PlayerConfig(Player);
    if (!p)
        return;

    int i;
    for(i=MAX_INSTANT_CHAT-1;i>=0;i--){
        tOutput name;
        name.SetTemplateParameter(1, i+1);
        name << "$player_chat_chat";

        ici[i] = new uMenuItemInput( &chat_menu, p->se_instantChatAction[ i ], Player + 1 );

        ic[i]=new uMenuItemString
              (&chat_menu,name,
               "$player_chat_chat_help",
               p->instantChatString[i]);

        ic2[i]=new uMenuItemString
              (&chat_menu2,name,
               "$player_chat_chat_help",
               p->instantChatString[i]);

        icd[i] = new uMenuItemDivider( &chat_menu );
    }

    uMenuItemToggle al
    (&playerMenu,"$player_autologin_text",
     "$player_autologin_help",
     p->autoLogin);

    uMenuItemString gid(&playerMenu,
                      "$player_global_id_text",
                      "$player_global_id_help",
                      p->globalID, 400);
    gid.SetColorMode( rTextField::COLOR_IGNORE );

    uMenuItemToggle st
    (&playerMenu,"$player_stealth_text",
     "$player_stealth_help",
     p->stealth);

    uMenuItemToggle sp
    (&playerMenu,"$player_spectator_text",
     "$player_spectator_help",
     p->spectate);

    uMenuItemToggle pnt
    (&playerMenu,"$player_name_team_text",
     "$player_name_team_help",
     p->nameTeamAfterMe);

    uMenuItemInt npt
    (&playerMenu,"$player_num_per_team_text",
     "$player_num_per_team_help",
     p->favoriteNumberOfPlayersPerTeam, 1, 16, 1);


    //Color Customization in the player menu
    uMenuItemSelection<int> se_cnr(&playerMenu,"Name Color","Name Color",p->colorNameCustomization);
    se_cnr.NewChoice("None",     "No color name customization", ColorNameCustomization::OFF_NAME);
    se_cnr.NewChoice("Gradient", "Gradient name customization", ColorNameCustomization::GRADIENT_NAME);
    se_cnr.NewChoice("Rainbow",  "Rainbow name customization",  ColorNameCustomization::RAINBOW_NAME);
    se_cnr.NewChoice("Shift",    "Shift name customization",    ColorNameCustomization::SHIFT_NAME);

    //Color Customization in the player menu
    uMenuItemSelection<int> se_cr(&playerMenu,"$player_color_randomization_text","$player_color_randomization_help",p->colorCustomization);
    se_cr.NewChoice("$player_color_randomization_none_text",      "$player_color_randomization_none_help",      ColorCustomization::OFF);
    se_cr.NewChoice("$player_color_randomization_random_text",    "$player_color_randomization_random_help",    ColorCustomization::RANDOM);
    se_cr.NewChoice("$player_color_randomization_unique_text",    "$player_color_randomization_unique_help",    ColorCustomization::UNIQUE);
    se_cr.NewChoice("$player_color_randomization_rainbow_text",   "$player_color_randomization_rainbow_help",   ColorCustomization::RAINBOW);
    se_cr.NewChoice("$player_color_randomization_crossfade_text", "$player_color_randomization_crossfade_help", ColorCustomization::CROSSFADE);

    // HACK: backup color before clamping by menuitems
    int r = p->rgb[0], g = p->rgb[1], b = p->rgb[2];

    ArmageTron_color_menuitem B(&playerMenu,"$player_blue_text",
                                "$player_blue_help",
                                p->rgb,2);

    ArmageTron_color_menuitem G(&playerMenu,"$player_green_text",
                                "$player_green_help",
                                p->rgb,1);

    ArmageTron_color_menuitem R(&playerMenu,"$player_red_text",
                                "$player_red_help",
                                p->rgb,0);

    // now, restore backed up non-clamped values.
    p->rgb[0] = r; p->rgb[1] = g; p->rgb[2] = b;


    uMenuItemSubmenu chm(&playerMenu,&chat_menu,
                         "$player_chat_chat_help");
    uMenuItemSubmenu chm2(&chat_menu,&chat_menu2,
                         "$player_chat_chat_help");

    uMenuItemSubmenu cm(&playerMenu,&camera_menu,
                        "$player_camera_help");

    uMenuItemFunctionInt icc(&playerMenu,"$player_camera_input_text",
                             "$player_camera_input_help",
                             &su_InputConfigCamera,Player);

    uMenuItemFunctionInt inc(&playerMenu,"$player_input_text",
                             "$player_input_help",
                             &su_InputConfig,Player);


    camera_menu.SetCenter(.3);

    uMenuItemToggle cam_glance
    (&camera_menu,
     "$camera_smart_glance_custom_text",
     "$camera_smart_glance_custom_help",
     p->smartCustomGlance);

    uMenuItemToggle cis(&camera_menu,
                        "$player_camera_autoin_text",
                        "$player_camera_autoin_help",
                        p->autoSwitchIncam);

    uMenuItemToggle cim(&camera_menu,
                        "$player_camera_wobble_text",
                        "$player_camera_wobble_help",
                        p->wobbleIncam);

    uMenuItemToggle cic(&camera_menu,
                        "$player_camera_center_int_text",
                        "$player_camera_center_int_help",
                        p->centerIncamOnTurn);

    uMenuItemToggle al_s
    (&camera_menu,
     "$player_camera_smartcam_text",
     "$player_camera_smartcam_help",
     p->allowCam[CAMERA_SMART]);

    uMenuItemToggle al_f
    (&camera_menu,
     "$player_camera_fixed_text",
     "$player_camera_fixed_help",
     p->allowCam[CAMERA_FOLLOW]);


    uMenuItemToggle al_fr
    (&camera_menu,
     "$player_camera_free_text",
     "$player_camera_free_help",
     p->allowCam[CAMERA_FREE]);

    uMenuItemToggle al_c
    (&camera_menu,
     "$player_camera_custom_text",
     "$player_camera_custom_help",
     p->allowCam[CAMERA_CUSTOM]);

    uMenuItemToggle al_sc
    (&camera_menu,
     "$player_camera_server_custom_text",
     "$player_camera_server_custom_help",
     p->allowCam[CAMERA_SERVER_CUSTOM]);

    uMenuItemToggle al_i
    (&camera_menu,
     "$player_camera_incam_text",
     "$player_camera_incam_help",
     p->allowCam[CAMERA_IN]);


    uMenuItemInt cam_fov
    (&camera_menu,
     "$player_camera_fov_text",
     "$player_camera_fov_help",
     p->FOV,30,160,5);

    uMenuItemSelection<eCamMode> cam_s
    (&camera_menu,
     "$player_camera_initial_text",
     "$player_camera_initial_help",
     p->startCamera);

    cam_s.NewChoice("$player_camera_initial_scust_text","$player_camera_initial_scust_help",CAMERA_SERVER_CUSTOM);
    cam_s.NewChoice("$player_camera_initial_cust_text","$player_camera_initial_cust_help",CAMERA_CUSTOM);
    cam_s.NewChoice("$player_camera_initial_int_text","$player_camera_initial_int_help",CAMERA_IN);
    cam_s.NewChoice("$player_camera_initial_smrt_text","$player_camera_initial_smrt_help",CAMERA_SMART);
    cam_s.NewChoice("$player_camera_initial_ext_text","$player_camera_initial_ext_help",CAMERA_FOLLOW);
    cam_s.NewChoice("$player_camera_initial_free_text","$player_camera_initial_free_help",CAMERA_FREE);

    uMenuItemString tn(&playerMenu,
                      "$player_teamname_text",
                      "$player_teamname_help",
                      p->teamname, 16);

    uMenuItemColorLine n(&playerMenu, p->name, 16);

    playerMenu.Enter();

    for(i=MAX_INSTANT_CHAT-1; i>=0; i--)
        delete ic[i];


    // request network synchronisation if the server can handle it
    static nVersionFeature inGameRenames( 5 );
    if ( inGameRenames.Supported() )
    {
        ePlayerNetID::Update();
        ePlayer::SendAuthNames();
    }

    /*
    for (i=MAX_PLAYERS-1; i>=0; i--)
    {
        if (ePlayer::PlayerIsInGame(i))
        {
            ePlayer* p = ePlayer::PlayerConfig(i);
            if (p->netPlayer)
                p->netPlayer->RequestSync();
        }
    }
    */
}


VOIDFUNC viewport_menu_x;






// from gGame.C
//extern int pingCharity;


void sg_PlayerMenu(){
    uMenu Player_men("$player_mainmenu_text");


    uMenuItemFunction vp_selec(&Player_men,
                               "$viewport_assign_text",
                               "$viewport_assign_help",
                               &viewport_menu_x);

    ArmageTron_viewport_menuitem vp(&Player_men);
    uMenuItemFunctionInt  *names[MAX_PLAYERS];

    int i;

    for(i=MAX_PLAYERS-1;i>=0;i--){
        tOutput title;
        title.SetTemplateParameter(1, i+1);
        title << "$player_menu_text";

        tOutput help;
        help.SetTemplateParameter(1, i+1);
        help << "$player_menu_help";

        names[i]=new uMenuItemFunctionInt(&Player_men,
                                          title,
                                          help,
                                          sg_PlayerMenu,i);
    }


    Player_men.Enter();

    //  ePlayerNetID::Update();
    for(i=MAX_VIEWPORTS-1;i>=0;i--){
        delete names[i];
    }
}




void viewport_menu_x(void){
    uMenu sg_PlayerMenu("$viewport_assign_text");

    ArmageTronPlayer_to_viewport_menuitem *select[MAX_VIEWPORTS];

    int i;
    for(i=rViewportConfiguration::s_viewportConfigurations[rViewportConfiguration::next_conf_num]->num_viewports-1;i>=0;i--){
        select[i]=new ArmageTronPlayer_to_viewport_menuitem(&sg_PlayerMenu,i);
    }

    // ArmageTron_viewport_menuitem vp(&sg_PlayerMenu);

    sg_PlayerMenu.Enter();

    for(i=rViewportConfiguration::s_viewportConfigurations[rViewportConfiguration::next_conf_num]->num_viewports-1;i>=0;i--){
        delete select[i];
    }
}


static uActionGlobal con_input( "CONSOLE_INPUT" );


static uActionGlobal screenshot( "SCREENSHOT" );

static uActionGlobal togglefullscreen( "TOGGLE_FULLSCREEN" );

static bool screenshot_func(REAL x){
    if (x>0){
#ifndef DEDICATED
        sr_screenshotIsPlanned=true;
#endif
    }

    return true;
}

static bool con_func(REAL x){
    if (x>0){
        sg_ConsoleInput();
    }

    return true;
}

static bool toggle_fullscreen_func( REAL x )
{
#ifndef DEDICATED
#ifdef DEBUG
    // don't toggle fullscreen while playing back in debug mode, that's annoying
    if ( tRecorder::IsPlayingBack() )
        return true;
#endif

    // only do anything if the application is active (work around odd bug)
    if ( x > 0 && ( SDL_GetAppState() & SDL_APPACTIVE ) )
    {
        currentScreensetting.fullscreen = !currentScreensetting.fullscreen;
        sr_ReinitDisplay();
    }
#endif

    return true;
}

static uActionGlobalFunc gaf_ss(&screenshot,&screenshot_func, true );
static uActionGlobalFunc gaf_md(&con_input,&con_func);
static uActionGlobalFunc gaf_tf(&togglefullscreen,&toggle_fullscreen_func, true );


void sg_MenusForVersion( tString version )
{
    if( tRecorder::IsPlayingBack() )
    {
        o28 = tConfigMigration::SavedBefore( version, "0.2.8.9" );
        o29 = version.StartsWith("0.2.9.0.1") || version.StartsWith("0.2.9.1");

        if( o28 || o29 )
        {

        }
    }
}
