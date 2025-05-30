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

#ifndef ArmageTron_tCOLOR_H
#define ArmageTron_tCOLOR_H

#include "tString.h"
#include "defs.h"

//! rgba color represented by floats between 0 and 1
class tColor
{
public:
    tColor();                                   //!< Constructor
    tColor(REAL r, REAL g, REAL b, REAL alpha = 1); //!< Constructor
    tColor(const char *c);                      //!< Creates a tColor from a color code string
    tColor(int rgb[3], REAL alpha = 1);
                                                //    tColor( const tString * c );		//!< Creates a tColor from a color code string
    ~tColor() {}                                //!< Destructor

    void FillFrom(const char *c); //!< Fills this color object from a color code string

    bool operator==(const tColor &other) const; //!< comparison operator

    bool operator!=(const tColor &other) const {
        return !(*this == other);
    }
    
    static bool VerifyColorCode(const char *c);    // Verifys it's a valid color code
    static bool VerifyColorCode(const wchar_t *c); // Verifys it's a valid color code

    // the colors are public because they are independent of each other
    REAL r_, g_, b_, a_; //!< Color values

    bool IsDark(void); //!< Has this color to be rendered on a bright background ?

protected:
private:
};

#endif
