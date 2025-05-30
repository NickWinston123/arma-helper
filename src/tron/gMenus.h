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

#ifndef gMenus_H
#define gMenus_H

#include "uMenu.h"

class ePlayer;

extern std::deque<tString> se_consoleHistory; 
extern tString se_consoleHistoryFileName;
extern void  sg_PlayerMenu(), sg_SpecialMenu(), sg_ConfigMenu();
extern uMenu sg_screenMenu;
void         sg_ConsoleInput(ePlayer* player = nullptr); // let the player enter one line of console input
extern ePlayer* sn_consoleUser();
extern ePlayer* sn_consoleUser(ePlayer *conUser);
extern void ConTabCompletition(tString &strString, int &cursorPos, bool changeLast);
#endif
