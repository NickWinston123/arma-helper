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

// I don't know if these includes are needed, I'm brute-forcing it :) - Dave
#include "gMenus.h"
#include "ePlayer.h"
#include "eTeam.h"
#include "rScreen.h"
#include "nConfig.h"
#include "rConsole.h"
#include "gHudMap.h"
#include "tToDo.h"
#include "rGL.h"
#include "eTimer.h"
#include "eFloor.h"
#include "rRender.h"
#include "rModel.h"
#include "gGame.h"
#include "tRecorder.h"

#include "eChatBot.h"
#include "ePlayerStats.h"

#include "rRender.h"
#include <math.h>
#include "gCycle.h"
#include "gHelper/gHelperHud.h"
#include "gHelper/gHelperUtilities.h"
#include <time.h>

#include "gHud.h"
#include "gSensor.h"

bool hud_WallTime = true, hud_WallTimeLabel = true, hud_WallTimeShowForInfinite = true;
REAL hud_WallTimeLocX = 0.90, hud_WallTimeLocY = -0.9, hud_WallTimeSize = .13;
REAL subby_SpeedGaugeSize = .175, subby_SpeedGaugeLocX = 0.0, subby_SpeedGaugeLocY = -0.9;
REAL subby_BrakeGaugeSize = .175, subby_BrakeGaugeLocX = 0.48, subby_BrakeGaugeLocY = -0.9;
REAL subby_RubberGaugeSize = .175, subby_RubberGaugeLocX = -0.48, subby_RubberGaugeLocY = -0.9;
bool subby_RubberMeterColorChange = true;
bool subby_ShowHUD = true, subby_ShowSpeedFastest = true, subby_ShowScore = true, subby_ShowAlivePeople = true, subby_ShowPing = true, subby_ShowSpeedMeter = true, subby_ShowBrakeMeter = true, subby_ShowRubberMeter = true;
bool hudShowTime = false;
bool hudShowTime24hour = false;
REAL subby_ScoreLocX = -0.95, subby_ScoreLocY = -0.85, subby_ScoreSize = .13;
REAL subby_FastestLocX = -0.2, subby_FastestLocY = -0.95, subby_FastestSize = .13;
REAL subby_AlivePeopleLocX = .45, subby_AlivePeopleLocY = -0.95, subby_AlivePeopleSize = .13;
REAL subby_PingLocX = .80, subby_PingLocY = -0.95, subby_PingSize = .13;

bool showPosition = true;
REAL subby_CoordLocX = -0.557, subby_CoordLoxY = -0.75, subby_CoordSize = 0.13;

bool hud_showInteract = true;
REAL hud_InteractLocX = -0.6, hud_InteractLocY = -0.95, hud_InteractSize = .10;

bool sg_TeamNumCount = false;

REAL max_player_speed = 0;

namespace
{
    static tConfigMigration migrate([](tString const &savedInVersion)
                                    {
    // defaults for subby_FastestSize and subby_FastestLocX changed on 0.2.9.1
    if(tConfigMigration::SavedBefore(savedInVersion, "0.2.9.1_alpha"))
    {
        if(subby_FastestSize < subby_AlivePeopleSize)
            subby_FastestSize = .13; // was .12
        if(subby_FastestLocX > -.1)
            subby_FastestLocX = -.2; // was -0.0
    } });
}

#ifndef DEDICATED

void GLmeter_subby(float value, float max, float locx, float locy, float size, const char *t, bool displayvalue = true, bool reverse = false, REAL r = .5, REAL g = .5, REAL b = 1)
{

#ifndef DEDICATED
    if (!sr_glOut)
        return;
#endif

    tString title(t);

    float x, y;
    char string[50];
    value > max ? value = max : 1;
    value < 0 ? value = 0 : 1;
    x = (reverse ? -1 : 1) * cos(M_PI * value / max);
    y = sin(M_PI * value / max);
    if (y < size * .24)
        displayvalue = false; // dont display text when at minimum

        /* Draws an ugly background on the gauge
            BeginQuads();
            Color(1.,1.,1.,.8);
            Vertex(locx-size-.04,locy-.04,0);
            Vertex(locx-size-.04,locy+size+.04,0);
            Vertex(locx+size+.04,locy+size+.04,0);
            Vertex(locx+size+.04,locy-.04,0);

            Color(.1,.1,.1,.8);
            Vertex(locx-size-.02,locy-.02,0);
            Vertex(locx-size-.02,locy+size+.02,0);
            Vertex(locx+size+.02,locy+size+.02,0);
            Vertex(locx+size+.02,locy-.02,0);

            RenderEnd();*/

#if 1
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2);
#endif

    glDisable(GL_TEXTURE_2D);
    Color(r, g, b);
    BeginLines();
    Vertex(-.1 * x * size + locx, .1 * y * size + locy, 0);
    Vertex(-x * size + locx, y * size + locy, 0);
    RenderEnd();

#if 1
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1);
#endif

    rTextField min_t(-size - (0.1 * size) + locx, locy, .12 * size, .24 * size);
    rTextField max_t(+size + (0.1 * size) + locx, locy, .12 * size, .24 * size);
    min_t << "0xcccccc" << (reverse ? max : 0);
    max_t << "0xcccccc" << (reverse ? 0 : max);

    if (displayvalue)
    {
        rTextField speed_t(-x * 1.45 * size + locx, y * 1.35 * size + locy, .12 * size, .24 * size);
        sprintf(string, "%.1f", value);
        speed_t << "0xccffff" << (string);
    }
    int length = title.Len();
    if (length > 0)
    {
        rTextField titletext(locx - ((.15 * size * (length - 1.5)) / 2.0), locy, .12 * size, .24 * size); // centre  -1.0 for null char and -.5 for half a char width = -1.5
        titletext << "0xff3333" << title;
    }
}

static REAL sg_hudCacheThreshold = .0001;
static tSettingItem<REAL> sg_minDropIntervalConf("HUD_CACHE_THRESHOLD", sg_hudCacheThreshold);

static bool sg_HudHideWhileChatting = true;
static tConfItem<bool> sg_HudHideWhileChattingConf("HUD_HIDE_WHILE_CHATTING", sg_HudHideWhileChatting);

gGLMeter::gGLMeter()
    : oldTime_(-100), oldRel_(-100)
{
}

void gGLMeter::Display(float value, float max, float locx, float locy, float size, const char *t, bool displayvalue, bool reverse, REAL r, REAL g, REAL b)
{
    REAL rel = value / max;
    REAL time = se_GameTime();
    REAL change = rel - oldRel_;

    // see if the gauge change is enough to warrant an update
    if (change * change * (time - oldTime_) > sg_hudCacheThreshold * sg_hudCacheThreshold || time < oldTime_)
    {
        list_.Clear();
    }

    if (!list_.Call())
    {
        oldRel_ = rel;
        oldTime_ = time;

        rDisplayListFiller filler(list_);
        GLmeter_subby(value, max, locx, locy, size, t, displayvalue, reverse, r, g, b);
    }
}

// caches stuff

// template <typename T1, typename T2>
// bool gTextCache<T1, T2>::Call( T1 propa, T2 propb )
// {
//     if ( propa != propa_ || propb != propb_ )
//     {
//         propa_ = propa;
//         propb_ = propb;
//         list_.Clear();
//         return false;
//     }
//     else
//     {
//         return list_.Call();
//     }
// }

static int alivepeople, alivemates, thetopscore, hudfpscount;

static void tank_display_hud(ePlayerNetID *me)
{
    static int fps = 60;
    static REAL lastTime = 0;

    if (se_mainGameTimer &&
        se_mainGameTimer->speed > .9 &&
        se_mainGameTimer->speed < 1.1 &&
        se_mainGameTimer->IsSynced())
    {
        REAL newtime = tSysTimeFloat();
        REAL ts = newtime - lastTime;
        int newfps = static_cast<int>(se_AverageFPS());
        if (fabs((newfps - fps) * ts) > 4)
        {
            fps = newfps;
            lastTime = newtime;
        }

        Color(1, 1, 1);
        rTextField c(-.9, -.6);

        // signed short int playerID = -1;
        //	  unsigned short int alivepeople2 = 0;
        unsigned short int max = se_PlayerNetIDs.Len();
        //	  signed short int scores[16];
        //	  signed short int teamID = -1;

        bool dohudcrap = false;
        if (hudfpscount >= fps)
        {
            hudfpscount = 0;
            dohudcrap = true;
        }
        else
        {
            hudfpscount++;
        }

        // Calculation, enemies (people not on me's team) alive
        if (dohudcrap)
        {
            alivepeople = 0;
            alivemates = 0;
            if (me)
            {
                for (unsigned short int i = 0; i < max; i++)
                {
                    ePlayerNetID *p = se_PlayerNetIDs(i);
                    if (p->Object() && p->Object()->Alive())
                    {
                        if (p->CurrentTeam() != me->CurrentTeam())
                        {
                            alivepeople++;
                        }
                        else
                        {
                            alivemates++;
                        }
                    }
                }
            }
        }

        /*	  int max = teams.Len();
              for(int i=0;i<max;i++){
                eTeam *t = teams(i);
              }*/

        if (me)
        {
            // Calculation, top player score
            if (dohudcrap)
            {
                //	    unsigned short int maxscore = 0;
                thetopscore = 0;
                for (unsigned short int i = 0; i < max; i++)
                {
                    ePlayerNetID *p2 = se_PlayerNetIDs(i);
                    //	  if (p2->TotalScore() > p->TotalScore() || p2->TotalScore() > thetopscore){
                    if (p2->TotalScore() > thetopscore)
                    {
                        thetopscore = p2->TotalScore();
                    }
                }
            }

            eCoord pcpos; // = 0 implied
            if (me->Object() && me->Object()->Alive())
            {
                pcpos = me->Object()->Position();
            }

            tString line1, line2, line3;

            /*    line1 << "HUD: " << p->name;
              line1.SetPos(22, true);
                    line1 << "FPS: " << fps;
              line1.SetPos(34, true);
              line1 << "ENEMIES: " << alivepeople << "\n";

                 line2 << "SCORE: " << p->TotalScore() << ", " << thetopscore;
              line2.SetPos(22, true);
                 line2 << "PING: " << int(p->ping*1000) << "\n";

              line3 << "LOCATION: " << int(pcpos.x);
              line3 << ", " << int(pcpos.y);
              line3.SetPos(22, true);
                // line3 << "RUBBER: " << int(p->rubberstatus*100);


             c <<line1 << line2; */
        }
    }
}

static void display_hud_subby(ePlayer *player)
{
    if (!player)
        return;
    ePlayerNetID *me = player->netPlayer;

    if (se_mainGameTimer &&
        se_mainGameTimer->speed > .9 &&
        se_mainGameTimer->speed < 1.1 &&
        se_mainGameTimer->IsSynced())
    {
        Color(1, 1, 1);
        rTextField c(-.9, -.85);
        rTextField t(.6, .98);

        // t << "0xffffffGame Time:" << int (se_GameTime())<< " s";
        if (subby_ShowHUD)
        {
            // static int max=0,lastmax=0;
            // static int imax=-1,lastimax=0;
            // static REAL imax=-1;
            char fasteststring[50];
            // REAL distance;
            float myping;
            static REAL max = max_player_speed;
            static tString name;
            static tString ultiname;
            // static bool  wrotefastest = false;
            // static bool wrote10 =false;
            static bool belowzero = false;
            static REAL ultimax = 0;
            // static REAL timelast;
            static REAL myscore, topscore;
            // tString realname = ePlayer::PlayerConfig(0)->Name();

            // const tString& myname = me->name;

            if (me)
                myscore = me->TotalScore();

            const eGameObject *ret = NULL;
            if (player->cam)
            {
                ret = player->cam->Center();

                static bool firsttime = true;

                if (se_GameTime() < 0 || !firsttime)
                {
                    firsttime = false;
                    if ((se_GameTime() < 0))
                    {
                        belowzero = true;
                        if (se_GameTime() < -1)
                        {
                            tOutput message;
                            //  message<< "Last Game Fastest: " << name << " with a top speed of " << max << "\n";
                            //  con << "testcon";
                            //  sn_CenterMessage(message);
                        }
                        else
                        {
                            // timelast = se_GameTime();
                            max = 0;
                            // wrotefastest = false;
                            // wrote10 =false;
                        }
                    }
                    else if (se_GameTime() > 0 && belowzero)
                    {
                        belowzero = false;
                    }
                }   

                topscore = 0;
                for (int i = 0; i < se_PlayerNetIDs.Len(); i++)
                {
                    ePlayerNetID *p = se_PlayerNetIDs[i];
                    p->TotalScore() > topscore ? topscore = p->TotalScore() : 1;

                    if (gCycle *h = (gCycle *)(p->Object()))
                    {

                        // change player so we always see the gauges of the cycle that is watched
                        if (h == ret)
                            me = p;

                        //   distance = h->distance;
                        //   c << p-> name << " " << int((h ->Speed())) << " ";
                        if (se_playerStats) 
                        {
                            REAL currentPlayerSpeed = h->Speed();
                            PlayerData &stats = ePlayerStats::getStats(p->GetName());
                            
                            if (stats.fastest_speed < currentPlayerSpeed)
                                stats.fastest_speed = currentPlayerSpeed;

                            if (stats.thisSession().fastest_speed < currentPlayerSpeed)
                                stats.thisSession().fastest_speed = currentPlayerSpeed;
                        }


                        if (h->Speed() > max)
                        {
                            max = (float)h->Speed(); // changed to float for more accuracy in reporting top speed
                            name = p->GetName();
                            // imax = i;
                        }
                    }
                }

                if (me != NULL)
                {
                    static gGLMeter meter2[MAX_PLAYERS];

                    gCycle *h = dynamic_cast<gCycle *>(me->Object());
                    if (h && (!player->netPlayer || (!player->netPlayer->IsChatting() || !sg_HudHideWhileChatting)) && se_GameTime() > -2)
                    {

                        gHelperHudBase::Render();

                        // myscore=p->TotalScore();
                        myping = me->ping;
                        static gTextCache<REAL> cacheArray[MAX_PLAYERS];
                        gTextCache<REAL> &cache = cacheArray[player->ID()];
                        gTextCache<std::string> cacheStr;

                        if (subby_ShowSpeedMeter)
                        {
                            static gGLMeter meter[MAX_PLAYERS];
                            meter[player->ID()].Display(h->Speed(), h->MaximalSpeed(), subby_SpeedGaugeLocX, subby_SpeedGaugeLocY, subby_SpeedGaugeSize, "Speed"); // easy to use configurable meters
                        }
                        if (subby_ShowRubberMeter)
                        {
                            static gGLMeter meter[MAX_PLAYERS];
                            if (!subby_RubberMeterColorChange)
                                meter[player->ID()].Display(h->GetRubber(), sg_rubberCycle, subby_RubberGaugeLocX, subby_RubberGaugeLocY, subby_RubberGaugeSize, " Rubber Used");
                            else
                                meter[player->ID()].Display(h->GetRubber(), sg_rubberCycle, subby_RubberGaugeLocX, subby_RubberGaugeLocY, subby_RubberGaugeSize, " Rubber Used", true, false, (h->GetRubber() / sg_rubberCycle), 1 - (h->GetRubber() / sg_rubberCycle), 0);
                            if (gCycle::RubberMalusActive())
                            {
                                static gGLMeter meter2[MAX_PLAYERS];
                                meter2[player->ID()].Display(100 / (1 + h->GetRubberMalus()), 100, subby_RubberGaugeLocX, subby_RubberGaugeLocY, subby_RubberGaugeSize, "", true, false, 1, .5, .5);
                            }
                        }
                        if (subby_ShowBrakeMeter)
                        {
                            static gGLMeter meter[MAX_PLAYERS];
                            meter[player->ID()].Display(h->GetBrakingReservoir(), 1.0, subby_BrakeGaugeLocX, subby_BrakeGaugeLocY, subby_BrakeGaugeSize, " Brakes");
                        }

                        if (hud_WallTime && h) // owner
                        {
                            gCycle *owner_ = h;
                            eCamera *cam = player->cam;

                            if (cam)
                            {
                                eCoord direction = owner_->Direction();
                                if (cam->glancingLeft)
                                {
                                    direction = owner_->Direction().Turn(eCoord(0, 1));
                                }
                                else if (cam->glancingRight)
                                {
                                    direction = owner_->Direction().Turn(eCoord(0, -1));
                                }

                                gSensor sensor(owner_, owner_->Position(), direction);
                                sensor.detect(10000);

                                if (sensor.ehit && sensor.wallOwner && sensor.wallOwner != owner_)
                                {
                                    REAL remainingTime = gCycle::timeBeforeWallRemoval(sensor.wallOwner);
                                    if (hud_WallTimeShowForInfinite || remainingTime > 0)
                                    {
                                        tString message;
                                        if (hud_WallTimeLabel)
                                            message << "0xbf9d50Wall Time: ";

                                        message << "0xffffff" << customRound(remainingTime, 2);

                                        rTextField wallTime(hud_WallTimeLocX - ((.15 * hud_WallTimeSize * (message.Len() - 1.5)) / 2.0),
                                                            hud_WallTimeLocY,
                                                            .15 * hud_WallTimeSize,
                                                            .3 * hud_WallTimeSize);

                                        wallTime << message;
                                    }
                                }
                            }
                        }

                        //  bool displayfastest = true;// put into global, set via menusytem... subby to do.make sr_DISPLAYFASTESTout

                        if (subby_ShowSpeedFastest)
                        {
                            static gTextCache<REAL> cacheArray[MAX_PLAYERS];
                            gTextCache<REAL> &cache = cacheArray[player->ID()];
                            if (!cache.Call(max, 0))
                            {
                                rDisplayListFiller filler(cache.list_);

                                float size = subby_FastestSize;
                                tColoredString message, messageColor;
                                messageColor << "0xbf9d50";

                                sprintf(fasteststring, "%.1f", max);
                                message << "  Fastest: " << name << " " << fasteststring;
                                message.RemoveHex(); // cheers tank;
                                int length = message.Len();

                                rTextField speed_fastest(subby_FastestLocX - ((.15 * size * (length - 1.5)) / 2.0), subby_FastestLocY, .15 * size, .3 * size);
                                /*   rTextField speed_fastest(.7-((.15*size*(length-1.5))/2.0),.65,.15*size,.3*size); */

                                speed_fastest << messageColor << message;
                            }
                        }

                        if (hud_showInteract)
                        {
                            static gTextCache<tString> cacheArray[MAX_PLAYERS];
                            gTextCache<tString> &cache = cacheArray[player->ID()];

                            static tString lastHunter;
                            tString hunterName;
                            if (h->GetPlayerHuntedBy())
                            {
                                hunterName = h->GetPlayerHuntedBy()->GetName();
                            }
                            else
                            {
                                hunterName = "0x808080<SUICIDE>";
                            }

                            if (!cache.Call(hunterName, lastHunter))
                            {
                                rDisplayListFiller filler(cache.list_);

                                static tColoredString messageColor("0x58bf90");

                                tColoredString message;
                                message << "  Interact: " << hunterName;

                                float size = hud_InteractSize;
                                int length = message.Len();

                                rTextField meter(hud_InteractLocX - ((.15 * size * (length - 1.5)) / 2.0), hud_InteractLocY, .15 * size, .3 * size);
                                meter << messageColor << message;
                            }
                        }

                        // */

                        if (subby_ShowScore)
                        {
                            static gTextCache<REAL> cacheArray[MAX_PLAYERS];
                            gTextCache<REAL> &cache = cacheArray[player->ID()];
                            if (!cache.Call(topscore, myscore))
                            {
                                rDisplayListFiller filler(cache.list_);

                                tString colour;
                                if (myscore == topscore)
                                {
                                    colour = "0xff9d50";
                                }
                                else if (myscore > topscore)
                                {
                                    colour = "0x11ff11";
                                }
                                else
                                {
                                    colour = "0x11ffff";
                                }

                                float size = subby_ScoreSize;
                                rTextField score(subby_ScoreLocX, subby_ScoreLocY, .15 * size, .3 * size);
                                score << " Scores\n";
                                score << "0xefefef"
                                      << "Me:  Top:\n";
                                score << colour << myscore << "     0xffff00" << topscore;
                            }
                        }

                        if (subby_ShowAlivePeople)
                        {
                            static gTextCache<REAL> cacheArray[MAX_PLAYERS];
                            gTextCache<REAL> &cache = cacheArray[player->ID()];
                            if (!cache.Call(alivepeople, alivemates))
                            {
                                rDisplayListFiller filler(cache.list_);

                                tString message;
                                message << "Enemies: " << alivepeople << " Friends: " << alivemates;
                                int length = message.Len();
                                float size = subby_AlivePeopleSize;
                                rTextField enemies_alive(subby_AlivePeopleLocX - ((.15 * size * (length - 1.5)) / 2.0), subby_AlivePeopleLocY, .15 * size, .3 * size);
                                enemies_alive << "0xfefefe" << message;
                            }
                        }

                        if (subby_ShowPing)
                        {
                            static gTextCache<REAL> cacheArray[MAX_PLAYERS];
                            gTextCache<REAL> &cache = cacheArray[player->ID()];
                            if (!cache.Call(0, myping))
                            {
                                rDisplayListFiller filler(cache.list_);

                                tString message;
                                message << "Ping: " << int(myping * 1000) << " ms";
                                int length = message.Len();
                                float size = subby_PingSize;
                                rTextField ping(subby_PingLocX - ((.15 * size * (length - 1.5)) / 2.0), subby_PingLocY, .15 * size, .3 * size);
                                ping << "0xfefefe" << message;
                            }
                        }
#ifdef DEBUG
                        if (showPosition)
                        {
                            tString output;
                            float posX = h->Position().x;
                            float posY = h->Position().y;
                            output << "Position X: " << posX << "\n";
                            output << "Position Y: " << posY;
                            int length = output.Len();
                            float size = subby_CoordSize;
                            rTextField location(subby_CoordLocX - ((.15 * size * (length - 1.5)) / 2.0), subby_CoordLoxY, .15 * size, .3 * size);
                            location << output;
                        }
#endif
                    }
                }
            }
        }
    }
    tank_display_hud(me); // call tank's version
}

static void display_fps_subby()
{
    if (!(se_mainGameTimer &&
          se_mainGameTimer->speed > .9 &&
          se_mainGameTimer->speed < 1.1 &&
          se_mainGameTimer->IsSynced()))
        return;

    static int fps = 60;
    static REAL lastTime = 0;

    REAL newtime = tSysTimeFloat();
    REAL ts = newtime - lastTime;

    static gTextCache<REAL> cache;
    if (cache.Call(fps, (int)tSysTimeFloat()))
    {
        return;
    }
    if (tRecorder::IsRunning())
    {
        cache.list_.Clear();
    }
    rDisplayListFiller filler(cache.list_);

    float size = .15;
    rTextField c2(.9 - .2 * rTextField::AspectWidthMultiplier(), .85, .15 * size * rTextField::AspectWidthMultiplier(), .3 * size);

    if (sr_RecordingTimeOut && tRecorder::IsRunning())
    {
        std::stringstream time;
        time.precision(5);
        time << newtime;
        {
            c2 << "0xffffffT:" << time.str() << "\n";
        }
    }

    int newfps = static_cast<int>(se_AverageFPS());
    if (fabs((newfps - fps) * ts) > 4)
    {
        fps = newfps;
        lastTime = newtime;
    }

    if (sr_FPSOut)
    {
        c2 << "0xffffffFPS: " << fps;
    }

    if (sg_TeamNumCount)
    {
        static tString output("0");
        static REAL lasttime = 0;

        if (tSysTimeFloat() - lasttime > 0.32)
        {
            lasttime = tSysTimeFloat();

            eTeam *myTeam = se_GetLocalPlayer() ? se_GetLocalPlayer()->CurrentTeam() : 0x0;

            output = "";

            if (myTeam)
            {
                // list players own team first

                int alive = 0;
                for (int i = myTeam->players.Len() - 1; i >= 0; --i)
                {
                    if (myTeam->players(i)->Object() && myTeam->players(i)->Object()->Alive())
                    {
                        ++alive;
                    }
                }

                output << alive;
            }

            for (int t = eTeam::teams.Len() - 1; t >= 0; --t)
            {
                if (myTeam && myTeam == eTeam::teams(t))
                    continue;

                int total = 0, alive = 0;
                for (int i = eTeam::teams(t)->players.Len() - 1; i >= 0; --i)
                {
                    ++total;
                    if (eTeam::teams(t)->players(i)->Object() && eTeam::teams(t)->players(i)->Object()->Alive())
                    {
                        ++alive;
                    }
                }

                if (total == 0)
                    continue;

                if (output[0] != '\0')
                {
                    output << "v";
                }

                output << alive;
            }
        }

        c2 << "\n0x808080" << output;
    }

    // Show the time
    if (hudShowTime)
    {
        float size = .15;
        std::string theTime = getTimeString(hudShowTime24hour);

        rTextField the_time(.9 - .2 * rTextField::AspectWidthMultiplier(), .9, .15 * size * rTextField::AspectWidthMultiplier(), .3 * size);

        the_time << "0xffffff" << theTime;
    }
}

int simplemapmode = 1;
static tConfItem<int> simplemapmode_con("HUD_MAP", simplemapmode);

extern bool stc_forbidHudMap;

static void drawMinimap()
{
    if (stc_forbidHudMap)
        return;

    sr_ResetRenderState(true);

    if (simplemapmode & 1)
    {
        gHudMapDrawConf c;

        c.cycleSize = 5;
        c.border = 10;

        c.x = .5;
        c.y = -1;
        c.w = .5;
        c.h = .5 * sr_screenWidth / sr_screenHeight;
        c.rw = .25 * sr_screenWidth;
        c.rh = .25 * sr_screenWidth;
        c.ix = 1;
        c.iy = 1; // iy was 0

        DrawMap(c);
    }

    if (simplemapmode & 2)
    {
        DrawMap(true, true, true,
                5.5, 10,
                -1, -1, 2, 2,
                sr_screenWidth, sr_screenHeight, .5, .5);
    }
}

static void display_hud_subby_all()
{
    sr_ResetRenderState(true);
    display_fps_subby();

    rViewportConfiguration *viewportConfiguration = rViewportConfiguration::CurrentViewportConfiguration();

    for (int viewport = viewportConfiguration->num_viewports - 1; viewport >= 0; --viewport)
    {
        // get the ID of the player in the viewport
        int playerID = sr_viewportBelongsToPlayer[viewport];

        // get the player
        ePlayer *player = ePlayer::PlayerConfig(playerID);

        // select the corrected viewport
        viewportConfiguration->Port(viewport)->CorrectAspectBottom().Select();

        // delegate
        display_hud_subby(player);
    }

    drawMinimap();
}

static rPerFrameTask dfps(&display_hud_subby_all);
#endif // DEDICATED
