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

#include "rFont.h"
#include "tSysTime.h"
#include "tConfiguration.h"
#include "rConsole.h"
#include "rScreen.h"
#include "tDirectories.h"
#include "tRecorder.h"
#include <iostream>


rConsole::rConsole()
        :currentTop(0),currentIn(0),
        lastCustomTimeout(-20),height(8),timeout(2),
        fullscreen(0),autoDisplayAtSwap(1),
autoDisplayAtNewline(0){
    RegisterBetterConsole(this);
}

static int sr_rows = 5;
static tConfItem<int> sr_rowsConf("CONSOLE_ROWS",sr_rows);

static int sr_maxRows = 19;
static tConfItem<int> sr_maxRowsConf("CONSOLE_ROWS_MAX",sr_maxRows);

int rConsole::MaxHeight(){
    int x=int(1.7/rCHEIGHT_CON)-1;;
    if (x>sr_maxRows)
        return sr_maxRows;
    else
        return x;
}

int rConsole::Height(){
    if (fullscreen)
        return MaxHeight();
    else if (rSmallConsoleCallback::SmallColsole())
        return sr_rows;
    else
        return height > sr_rows ? height : sr_rows;

}
REAL rConsole::Timeout(){
    if (fullscreen)
        return(100000000.0);
    else
        return timeout;
}


void rConsole::SetHeight(int h,bool stop_scroll){
    height=h;
    if (stop_scroll)
        lastCustomTimeout=tSysTimeFloat()-30;
}

void rConsole::SetTimeout(REAL to){timeout=to;}


//#ifdef DEBUG
//#define MAXBACK 10
//#define BACKEXTRA 5
//#else
#define MAXBACK 300
#define BACKEXTRA 100
//#endif

REAL rCWIDTH_CON=REAL(16/640.0);
REAL rCHEIGHT_CON=REAL(32/480.0);

static bool sr_consoleLog = false;
static tConfItem<bool> sr_consoleLogConf("CONSOLE_LOG", sr_consoleLog);

bool sr_consoleLogLimited = false;
static tConfItem<bool> sr_consoleLogLimitedConf("CONSOLE_LOG_LIMITED", sr_consoleLogLimited);

REAL sr_consoleLogLimitedSize = 50;
static tConfItem<REAL> sr_consoleLogLimitedSizeConf("CONSOLE_LOG_LIMITED_SIZE", sr_consoleLogLimitedSize);

bool sr_consoleTimeStamp = false;
static tConfItem<bool> sr_consoleTimeStampConf("CONSOLE_TIMESTAMP", sr_consoleTimeStamp);

static bool sr_consoleLogColor = false;
static tConfItem<bool> sr_consoleLogColorConf("CONSOLE_LOG_COLOR", sr_consoleLogColor);

static bool sr_consoleLogColorTimestamp = false;
static tConfItem<bool> sr_consoleLogColorTimestampConf("CONSOLE_LOG_COLOR_DECORATE_TIMESTAMP", sr_consoleLogColorTimestamp);

tConsole & rConsole::DoPrint(const tString &s){
    bool print_to_stdout=false;
#ifdef DEBUG
    print_to_stdout=true;
#endif
    bool swap = false;

    if (!sr_screen)
        print_to_stdout=true;
    if (print_to_stdout)
    {
        std::cout << tColoredString::RemoveColorsLoose(s);
        std::cout.flush();
    }

    if(!tRecorder::IsPlayingBack() && sr_consoleLog) 
    {
        FileManager fileManager(tString("consolelog.txt"), tDirectories::Log());

        tString finalLine;
        finalLine << st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ") << tColoredString::RemoveColorsLoose(s);
        fileManager.Write(finalLine);
    }
       

    if(!tRecorder::IsPlayingBack() && sr_consoleLogLimited) 
    {
        FileManager fileManager(tString("consolelog-limited.txt"), tDirectories::Log());

        tString finalLine;
        finalLine << st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ") << tColoredString::RemoveColorsLoose(s);
        fileManager.Write(finalLine);
    }
    
    
    if(!tRecorder::IsPlayingBack() && sr_consoleLogColor) 
    {

        FileManager fileManager(tString("consolelog-color.txt"), tDirectories::Log());

        tString finalLine;

        if(sr_consoleLogColorTimestamp)
            finalLine << st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ");
            
        finalLine << s;

        fileManager.Write(finalLine);
        
    }

    if (sr_screen){
        tString message;

        if (sr_consoleTimeStamp)
            message = st_GetCurrentTime("%H:%M:%S| ") << s;   
        else 
            message = s;
        const char *c=message;
        while (*c!=0){
            lines[currentIn] << *c;
            if (*c=='\n'){
                if (currentIn<=currentTop+1)
                    lastTimeout=tSysTimeFloat()+4;
                currentIn++;
                if (autoDisplayAtNewline && !rNoAutoDisplayAtNewlineCallback::NoAutoDisplayAtNewline() && (sr_textOut ||
                        rForceTextCallback::ForceText()))
                    swap = true;

                if (currentIn >= MAXBACK+BACKEXTRA){
                    for(int i=0;i<MAXBACK;i++)
                        lines[i]=lines[i+BACKEXTRA];

                    for(int j=lines.Len()-1;j>=MAXBACK;j--)
                        lines[j].Clear();

                    currentIn-=BACKEXTRA;
                    currentTop-=BACKEXTRA;
                    if (currentTop<0)
                        currentTop=0;
                }
            }
            c++;
        }

        if (rSmallConsoleCallback::SmallColsole() || lastCustomTimeout<tSysTimeFloat()-15)
            while ((currentIn-currentTop) > Height())
                currentTop++;
    }

    if (swap)
        DisplayAtNewline();

    return *this;
}

// moves to the end, showing the last lines
void rConsole::End(int last)
{
    currentTop = currentIn - last;
    if ( currentTop < 0 )
    {
        currentTop = 0;
    }
}

void rConsole::Scroll(int dir){
    rCenterDisplayCallback::CenterDisplay();

    currentTop+=dir*10;
    lastCustomTimeout=tSysTimeFloat();
    if (currentTop<0)
        currentTop=0;

    if (currentTop>currentIn-10)
        lastCustomTimeout=tSysTimeFloat()-10;

    if (currentTop>currentIn){
        currentTop=currentIn;
        lastCustomTimeout=tSysTimeFloat()-20;
    }
}

tString rConsole::ColorString(REAL r, REAL g, REAL b) const{
    tColoredString ret;
    ret << tColoredString::ColorString(r,g,b);
    return ret;
}

rConsole sr_con;

// ---------------------------------------------------

static tCallbackOr *tCallbackOr_anchor;

rForceTextCallback::rForceTextCallback(BOOLRETFUNC *f)
        :tCallbackOr(tCallbackOr_anchor, f){}

bool rForceTextCallback::ForceText(){
    return Exec(tCallbackOr_anchor);
}

static tCallbackOr *SmallColsole_anchor;

rSmallConsoleCallback::rSmallConsoleCallback(BOOLRETFUNC *f)
        :tCallbackOr(SmallColsole_anchor, f){}

bool rSmallConsoleCallback::SmallColsole(){
    return Exec(SmallColsole_anchor);
}


static tCallback *CenterDisplay_anchor;

rCenterDisplayCallback::rCenterDisplayCallback(VOIDFUNC *f)
        :tCallback(CenterDisplay_anchor, f){}

void rCenterDisplayCallback::CenterDisplay(){
    Exec(CenterDisplay_anchor);
}

static tCallbackOr *NoAutoDisplayAtNewline_anchor;

rNoAutoDisplayAtNewlineCallback::rNoAutoDisplayAtNewlineCallback(BOOLRETFUNC *f)
        :tCallbackOr(NoAutoDisplayAtNewline_anchor, f){}

bool rNoAutoDisplayAtNewlineCallback::NoAutoDisplayAtNewline(){
    return Exec(NoAutoDisplayAtNewline_anchor);
}





