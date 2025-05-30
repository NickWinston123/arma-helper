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

#ifndef ArmageTron_FONT_H
#define ArmageTron_FONT_H

#include "rSDL.h"

#include "defs.h"
#include "rTexture.h"
#include "tString.h"
#include "tColor.h"

class rFont:public rFileTexture{
    int offset;
    REAL cwidth;
    REAL cheight;
    REAL onepixel;
    int borderExtension{1}; // does the left side pixel border of a character get moved half a pixel left or right?
    rFont *lowerPart;
public:
    rFont(const char *fileName,int Offset=0,REAL CWidth=(1/16.0),
          REAL CHeight=(1/8.0),REAL onepixel=1/256.0, int borderExtension = 1, rFont *lower=NULL);
    rFont(const char *fileName, rFont *lower);
    virtual ~rFont();

#ifndef DEDICATED
    // displays c
    void Render(unsigned char c,REAL left,REAL top,REAL right,REAL bot);
#endif
    static rFont s_defaultFont,s_defaultFontSmall;

protected:
    virtual void ProcessImage(SDL_Surface *);       //!< process the surface before uploading it to GL
    virtual void OnSelect( bool enforce );
};


// **********************************************+

// maybe make this a child of std::ostream...
class rTextField{
    tString buffer;       // buffer where we store stuff before we print it
    int  width;          // width in characters
    int  parIndent;      // number of spaces to insert after automatic newline
    REAL left,top;       // top left corner of the console
    REAL cwidth,cheight; // character dimensions
    rFont *F;             // the font
    int  x,y,realx;      // current cursor position

    tColor color_;               //!< current color
    static tColor defaultColor_; //!< default color
    static tColor blendColor_;   //!< color all other colors are multiplied with

    int cursor; // display mode of the cursor; 0: disabled, 1: low, 2: high
    int cursorPos; // position of the cursor (number of chars to come)

    REAL cursor_x,cursor_y; // position on the screen

    void FlushLine(int len,bool newline=true);
    void FlushLine(bool newline=true);
public:
#define  rCWIDTH_NORMAL  (16/640.0)
#define  rCHEIGHT_NORMAL (32/480.0)

    rTextField(REAL Left,REAL Top,
               REAL Cwidth=rCWIDTH_NORMAL,REAL Cheight=rCHEIGHT_NORMAL,
               rFont *f=&rFont::s_defaultFont);

    // all the basic code assumes a 4:3 screen. We won't fix that here on 0.2.9.
    // instead, users need to either multiply their witdh (and maybe left) with
    // AspectWidthMultiplier() or their height (and Top) with AspectHeightMultiplier().
    // Pick one for each context and stick with it. The Width modification should be
    // the default.
    static REAL AspectWidthMultiplier();
    static REAL AspectHeightMultiplier();

    // puts x/y right into the middle of a screen pixel given Width or Height.
    static REAL Pixelize(REAL xy, int WidthHeight);

    virtual ~rTextField(); // for future extensions (buffered console?)

    REAL GetCWidth() const {
        return cwidth;
    }
    REAL GetCHeight() const {
        return cheight;
    }

    void SetTop( REAL t ){
        top = t;
    }

    void SetLeft( REAL l ){
        top = l;
    }

    REAL GetTop() const{
        return top;
    }
    REAL GetBottom() const{
        return top - cheight * y;
    }


    REAL GetLeft() const{
        return left;
    }

    void SetWidth(int w){
        width=w;
    }

    int GetWidth() const {
        return width;
    }

    void SetIndent(int i){
        parIndent=i;
    }

    int GetIndent() const {
        return parIndent;
    }

    void SetCursor(int c,int p){
        cursor=c;
        cursorPos=p;
    }

    void ResetColor(){
        FlushLine(false);
        color_ = defaultColor_;
    }

    // rTextField & operator<<(unsigned char c);

    enum ColorMode {
        COLOR_IGNORE,   // ignore color codes, printing everything verbatim
        COLOR_USE,      // normal mode: hide color codes and use them
        COLOR_SHOW      // use color codes, but print them as well
    };

    rTextField & StringOutput(const char *c, ColorMode colorMode = COLOR_USE );

    int Lines(){
        return y;
    }

    inline rTextField & SetColor( tColor const & color );	//!< Sets current color
    inline tColor const & GetColor( void ) const;	//!< Gets current color
    inline rTextField const & GetColor( tColor & color ) const;	//!< Gets current color
    static void SetDefaultColor( tColor const & defaultColor );	//!< Sets default color
    static tColor const & GetDefaultColor( void );	//!< Gets default color
    static void GetDefaultColor( tColor & defaultColor );	//!< Gets default color
    static void SetBlendColor( tColor const & blendColor );	//!< Sets color all other colors are multiplied with
    static tColor const & GetBlendColor( void );	//!< Gets color all other colors are multiplied with
    static void GetBlendColor( tColor & blendColor );	//!< Gets color all other colors are multiplied with

    static float GetTextLength (const char * str, float height, bool stripColors=false, bool useNewline=true, float *resultingHeight=0); //!< Predict the dimenstions of a string
private:
    inline void WriteChar(unsigned char c); //!< writes a single character as it is, no automatic newline breaking
};

template<class T> rTextField & operator<<(rTextField &c,const T &x){
    tColoredString out;
    out << x;
    return c.StringOutput(out);
}

void DisplayText(REAL x,REAL y,REAL w,REAL h,const char *text,int center=0,
                 int cursor=0,int cursorPos=0, rTextField::ColorMode colorMode = rTextField::COLOR_USE );

void DisplayTextAutoWidth(REAL x,REAL y,const char *text,REAL h=rCHEIGHT_NORMAL,int center=0,
                 int cursor=0,int cursorPos=0, rTextField::ColorMode colorMode = rTextField::COLOR_USE );

void DisplayTextAutoHeight(REAL x,REAL y,const char *text,REAL w=rCWIDTH_NORMAL,int center=0,
                 int cursor=0,int cursorPos=0, rTextField::ColorMode colorMode = rTextField::COLOR_USE );

// *******************************************************************************************
// *
// *	GetColor
// *
// *******************************************************************************************
//!
//!		@return		current color
//!
// *******************************************************************************************

tColor const & rTextField::GetColor( void ) const
{
    return this->color_;
}

// *******************************************************************************************
// *
// *	GetColor
// *
// *******************************************************************************************
//!
//!		@param	color	current color to fill
//!		@return		A reference to this to allow chaining
//!
// *******************************************************************************************

rTextField const & rTextField::GetColor( tColor & color ) const
{
    color = this->color_;
    return *this;
}

// *******************************************************************************************
// *
// *	SetColor
// *
// *******************************************************************************************
//!
//!		@param	color	current color to set
//!		@return		A reference to this to allow chaining
//!
// *******************************************************************************************

rTextField & rTextField::SetColor( tColor const & color )
{
    this->color_ = color;
    return *this;
}

#endif

