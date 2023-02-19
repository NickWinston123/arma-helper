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

#include "rDisplayList.h"
#ifndef ARMAGETRON_HUD_H
#define ARMAGETRON_HUD_H

std::string getTimeString(bool hudShowTime24hour);

class gGLMeter
{
public:
    gGLMeter();
    void Display(float value,float max, float locx, float locy, float size, const char * t,bool displayvalue = true, bool reverse = false, REAL r=.5, REAL g=.5, REAL b=1);
private:
    REAL oldTime_;      // last rendered game time
    REAL oldRel_;       // last rendered gauge position
    rDisplayList list_; // caching display list
};

template <typename T1, typename T2>
class gTextCache
{
    public:
    gTextCache() : propa_{}, propb_{} {}
    bool Call(T1 propa, T2 propb) {
    if ( !(propa == propa_) || !(propb == propb_) )
    {
        propa_ = propa;
        propb_ = propb;
        list_.Clear();
        return false;
    }
    else
    {
        return list_.Call();
    }
    };
    
    rDisplayList list_;

private:
    T1 propa_;
    T2 propb_;

};


extern REAL subby_SpeedGaugeSize, subby_SpeedGaugeLocX, subby_SpeedGaugeLocY;
extern REAL subby_BrakeGaugeSize, subby_BrakeGaugeLocX, subby_BrakeGaugeLocY;
extern REAL subby_RubberGaugeSize, subby_RubberGaugeLocX, subby_RubberGaugeLocY;
extern bool subby_ShowHUD, subby_ShowSpeedFastest, subby_ShowScore, subby_ShowAlivePeople, subby_ShowPing, subby_ShowSpeedMeter, subby_ShowBrakeMeter, subby_ShowRubberMeter;
extern REAL subby_ScoreLocX, subby_ScoreLocY, subby_ScoreSize;
extern REAL subby_FastestLocX, subby_FastestLocY, subby_FastestSize;
extern REAL subby_AlivePeopleLocX, subby_AlivePeopleLocY, subby_AlivePeopleSize;
extern REAL subby_PingLocX, subby_PingLocY, subby_PingSize;
extern bool hudShowTime;
extern bool hudShowTime24hour;
extern bool showPosition;
extern REAL subby_CoordLocX, subby_CoordLoxY, subby_CoordSize;
#endif
