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

#include "tString.h"
#include "tConfiguration.h"

#include "tColor.h"

static REAL CTR(int x)
{
    return x / 255.0;
}

static char hex_array[] = "0123456789abcdef";

int hex_to_int(char c)
{
    int ret = 0;
    for (int i = 15; i >= 0; i--)
        if (hex_array[i] == c)
            ret = i;
    return ret;
}

// minimal tolerated values of font color before a white background is rendered
static REAL sr_minR = .5, sr_minG = .5, sr_minB = .5, sr_minTotal = .7;
tConfItem<REAL> sr_minRConf("FONT_MIN_R", sr_minR);
tConfItem<REAL> sr_minGConf("FONT_MIN_G", sr_minG);
tConfItem<REAL> sr_minBConf("FONT_MIN_B", sr_minB);
tConfItem<REAL> sr_minTotalConf("FONT_MIN_TOTAL", sr_minTotal);

// *******************************************************************************************
// *
// *	tColor
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

tColor::tColor()
    : r_(1), g_(1), b_(1), a_(1)
{
}

// *******************************************************************************************
// *
// *	tColor
// *
// *******************************************************************************************
//!
//!		@param	r	Red component of the tColor to create
//!		@param	g	Green component of the tColor to create
//!		@param	b	Blue component of the tColor to create
//!		@param	a	Alpha component of the tColor to create
//!
// *******************************************************************************************

tColor::tColor(REAL r, REAL g, REAL b, REAL a)
    : r_(r), g_(g), b_(b), a_(a)
{
}

tColor::tColor(int rgb[3], REAL a)
    : r_(rgb[0]), g_(rgb[1]), b_(rgb[2]), a_(a)
{
}

// *******************************************************************************************
// *
// *	tColor
// *
// *******************************************************************************************
//!
//!		@param	other	Color to compare with
//!     @ret    true if the colors are sufficiently equal
//!
// *******************************************************************************************
bool tColor::operator==(const tColor &other) const
{
    return (fabs(other.r_ - r_) < EPS && fabs(other.g_ - g_) < EPS &&
            fabs(other.b_ - b_) < EPS && fabs(other.a_ - a_) < EPS);
}

// *******************************************************************************************
// *
// *	tColor
// *
// *******************************************************************************************
//!
//!		@param	c	Color code string to read the color from
//!
// *******************************************************************************************

/*tColor::tColor( const tString * c )
:a_(1)
{
    FillFrom( c );
}*/

// *******************************************************************************************
// *
// *	tColor
// *
// *******************************************************************************************
//!
//!		@param	c	Color code string to read the color from
//!
// *******************************************************************************************

tColor::tColor(const char *c)
    : a_(1)
{
    FillFrom(c);
}

// *******************************************************************************************
// *
// *	FillFrom
// *
// *******************************************************************************************
//!
//!		@param	c	Color code string to read the color from
//!
// *******************************************************************************************

void tColor::FillFrom(const char *c)
{
    // check whether the passed string is too short
    for (int i = 0; i < 8; ++i)
    {
        if (!c[i])
        {
            r_ = g_ = b_ = 0;
            return;
        }
    }

    r_ = CTR(hex_to_int(c[2]) * 16 + hex_to_int(c[3]));
    g_ = CTR(hex_to_int(c[4]) * 16 + hex_to_int(c[5]));
    b_ = CTR(hex_to_int(c[6]) * 16 + hex_to_int(c[7]));
}

// strict checking: accept only 0-9 and a-f.  Network aware config item is in nNetwork.cpp.
bool st_verifyColorCodeStrictly = 0;

static bool st_verifyColorChar(int c)
{
    if (st_verifyColorCodeStrictly)
    {
        // really check for valid hexcodes
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    }
    else
    {
        // be content with ASCII.
        return (0 < c) && (c <= 0x7f);
    }
}

// *******************************************************************************************
// *
// *	VerifyColorCode
// *
// *******************************************************************************************
//!
//!		@param	c	Color code string to read the color from
//!
// *******************************************************************************************

bool tColor::VerifyColorCode(const char *c)
{
    for (int i = 2; i < 8; ++i)
    {
        if (!st_verifyColorChar(c[i]))
        {
            return false;
        }
    }
    return true;
}

// *******************************************************************************************
// *
// *	VerifyColorCode
// *
// *******************************************************************************************
//!
//!		@param	c	Color code string to read the color from
//!
// *******************************************************************************************

bool tColor::VerifyColorCode(const wchar_t *c)
{
    for (int i = 2; i < 8; ++i)
    {
        if (!st_verifyColorChar(c[i]))
        {
            return false;
        }
    }
    return true;
}

// *******************************************************************************************
// *
// *	tColor
// *
// *******************************************************************************************
//!
//!		@param	c	Color code string to read the color from
//!
// *******************************************************************************************

bool tColor::IsDark(void)
{
    return (r_ < sr_minR && g_ < sr_minG && b_ < sr_minB) || r_ + g_ + b_ < sr_minTotal;
}
