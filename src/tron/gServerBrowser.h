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

#ifndef ArmageTron_ServerBrowser_H
#define ArmageTron_ServerBrowser_H

class nServerInfo;
class nServerInfoBase;

class gServerBrowser
{
public:
    static void BrowseFavorites();                                                // browse favorite servers
    static void BrowseMaster();                                                   // browse servers from the central master servers
    static void BrowseSpecialMaster(nServerInfoBase *master, char const *prefix); // browse servers from a special master server
    static void BrowseLAN();                                                      // browse servers in the LAN
    static void BrowseServers();                                                  // browse the servers currently in the list
    static void ConfigurationMenu();                                              // browser configuration menu

    static nServerInfoBase *CurrentMaster(); // the currently active master server

    static int lowPort, highPort;
};

#endif
