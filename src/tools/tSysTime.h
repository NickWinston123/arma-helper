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

#ifndef ArmageTron_SysTime_H
#define ArmageTron_SysTime_H
#include <string>
bool tTimerIsAccurate();                      //! returns true if a timer with more than millisecond accuracy is available
double tSysTimeFloat();                       //! returns the current frame's time ( from the playback )
double tRealSysTimeFloat();                   //! returns the current frame's time ( from the real system )
void tAdvanceFrame( int usecdelay = 0);       //! andvances one frame: updates the system time
void tDelay( int usecdelay );                 //! delays for the specified number of microseconds
void tDelayForce( int usecdelay );            //! delays for the specified number of microseconds, even when playing back
std::string getTimeString(bool hudShowTime24hour = false);
std::string getTimeStringBase(struct tm &thistime, bool hudShowTime24hour = false);
std::string getTimeZone(const struct tm& timeInfo);

struct tm getCurrentLocalTime();
time_t convertToTimeT(struct tm date);
time_t getDifferenceInSeconds(struct tm date1, struct tm date2) ;
time_t getStartTimeOfDay();
time_t getStartOfCurrentHour();

 double getSteadyTime();

#endif
