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

#ifndef ArmageTron_consoleBase_H
#define ArmageTron_consoleBase_H

#include "tString.h"
#include "defs.h"
#include "tLinkedList.h"
#include "nNetwork.h"
class tOutput;

//! filter for console messages
class tConsoleFilter: public tListItem< tConsoleFilter >
{
public:
    tConsoleFilter();
    ~tConsoleFilter();

    //! filter an output line
    inline void FilterLine( tString& line )
    {
        DoFilterLine( line );
    }

    //! filter a localizable output paramenter
    inline void FilterElement( tString& element )
    {
        DoFilterElement( element );
    }

    inline int GetPriority() const
    {
        return DoGetPriority();
    }
private:
    // the functions that do the real work
    virtual void DoFilterLine	( tString& line 	);		//!< filter an output line
    virtual void DoFilterElement( tString& element 	);		//!< filter a localizable output paramenter

    virtual int DoGetPriority() const;						//!< return filter priority
};

// console class logging various messages
class tConsole
{
public:
    // callback for messages the user should read
    typedef bool MessageCallback(const tOutput& message, const tOutput& interpretation, REAL timeout);

    // idle callback; called from various spots when the progam is waiting.
    // return true if the waiting should be aborted, parameter is true if input should
    // actually be processed
    typedef bool IdleCallback( bool );

    virtual ~tConsole();

    tConsole & Print(tString const & s);

    template<class T> tConsole & operator<<(const T&x){
        tColoredString s;
        s << x;
        return Print(s);
    }


    tColoredString nMessageToString(nMessage& msg) {
        tColoredString s;
        s << "Descriptor: "  << msg.Descriptor() << "\n";
        s << "Sender ID: "   << msg.SenderID() << "\n";
        s << "Message ID: "  << msg.MessageID() << "\n";
        s << "Data Length: " << msg.DataLen() << "\n";

        s << "Data: ";
        for (int i=0; i<msg.DataLen(); i++)
            s << msg.Data(i) << " ";
        s << "\n";

        return s;
    }

    void nMessage(nMessage& msg) {
        Print(nMessageToString(msg));
    }

    void CenterDisplay(tString s,REAL timeout=5,REAL r=1,REAL g=1,REAL b=1);

    // give a message to the user
    static bool Message(const tOutput& message, const tOutput& interpetation, REAL timeout = -1);

    // idle around a bit (return value true: abort whatever you're doing),
    // input value determines whether input is actually processed
    static bool Idle( bool processInput );

    virtual tString ColorString(REAL r, REAL g, REAL b) const;

    static void RegisterMessageCallback(MessageCallback *callback);
    static void RegisterIdleCallback(IdleCallback *callback);

protected:
    static void RegisterBetterConsole(tConsole *better);

private:
    static tConsole *s_betterConsole;

    void PrintLine(tString const & s, int repetitions);
    virtual tConsole & DoPrint(const tString& s);
    virtual void DoCenterDisplay(const tString& s,REAL timeout=5,REAL r=1,REAL g=1,REAL b=1);
};

extern tConsole con;

#endif
