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

#include "tMemManager.h"
#include "tString.h"
#include "tColor.h"
#include "tLocale.h"
#include "tConfiguration.h"
#include <ctype.h>
#include <time.h>
#include <string>
#include <iostream>

tString::tString(){
    operator[](0)='\0';
}

tString::tString(const char *x){
    operator[](0)='\0';
    operator+=(x);
}

tString::tString(std::string x){
    operator[](0)='\0';
    operator+=(x.c_str());
}


tString::tString(const tString &x)
        :tArray< char >()
{
    operator[](0)='\0';
    operator=(x);

    tASSERT( (*this) == x );
}

tString::tString(const tOutput &x){
    operator[](0)='\0';
    operator=(x);
}

// reads an escape sequence from a stream
// c: the character just read from the stream
// c2: eventually set to the second character read
// s: the stream to read from
static bool st_ReadEscapeSequence( char & c, char & c2, std::istream & s )
{
    c2 = '\0';

    // detect escaping
    if ( c == '\\' )
    {
        c = s.get();

        // nothing useful read?
        if ( s.eof() )
        {
            c = '\\';
            return false;
        }

        // interpret special escape sequences
        switch (c)
        {
        case 'n':
            // turn \n into newline
            c = '\n';
            return true;
        case '"':
        case ' ':
        case '\'':
        case '\\':
        case '\n':
            // include quoting character as literal
            return true;
        default:
            // take the whole \x sequence as it appeared.
            c2 = c;
            c = '\\';
            return false;
        }
    }

    return false;
}

// reads an escape sequence from a stream, putting back unread char
// c: the character just read from the stream
// s: the stream to read from
static bool st_ReadEscapeSequence( char & c, std::istream & s )
{
    char c2 = '\0';
    bool ret = st_ReadEscapeSequence( c, c2, s );
    if ( c2 && s.good() )
    {
        s.putback( c2 );
    }
    return ret;
}

void tString::ReadLine(std::istream &s, bool enableEscapeSequences ){
    char c=' ';
    int i=0;
    SetLen(0);

    // note: this is not the same as std::ws(s), we're stopping at newlines.
    while(c!='\n' && c!='\r' && isblank(c) &&  s.good() && !s.eof()){
        c=s.get();
    }
    if(s.good())
    {
        s.putback(c);
    }

    c='x';

    while( true )
    {
        c=s.get();

        // notice end of line or file
        if ( c=='\n' || c=='\r' || !s.good() || s.eof())
            break;

        if ( enableEscapeSequences )
        {
            char c2 = '\0';
            if ( st_ReadEscapeSequence( c, c2, s ) )
            {
                operator[](i++)=c;
                c = 'x';
                continue;
            }
            else if ( c2 )
            {
                operator[](i++)=c;
                operator[](i++)=c2;
                c = 'x';
                continue;
            }
        }
        operator[](i++)=c;
    }

    operator[](i)='\0';
}

tString & tString::operator<<(const char *c){
    return operator+=(c);
}

tString & tString::operator+=(const char *c){
    if (c){
        if ( Len() == 0 )
            operator[](0)='\0';

        int i=Len()-1;
        if (i<0) i=0;
        while (*c!='\0'){
            operator[](i)=*c;
            i++;
            c++;
        }
        operator[](i)='\0';
    }
    return *this;
}

tString & tString::operator=(const char *c){
    Clear();
    return operator+=(c);
}

tString & tString::operator=(const tOutput& o){
    Clear();
    return operator <<(o);
}

tString tString::operator+(const char *c) const{
    tString s(*this);
    return s+=c;
    //return s;
}

tString::operator const char *() const{
    if (Len())
        return &operator()(0);
    else
        return "";
}

tString & tString::operator<<(char c){
    return operator+=(c);
}

tString & tString::operator+=(char c){
    if ( Len() == 0 )
        operator[](0)='\0';

    int i=Len();
    if (i<=0) i=1;
    operator[](i-1)=c;
    operator[](i)='\0';
    return *this;
}

tString tString::operator+(char c) const{
    tString s(*this);
    return s+=c;
    //return s;
}

std::ostream & operator<< (std::ostream &s,const tString &x){
    if(x.Len())
        return s << &(x(0));
    else
        return s;
}

std::istream & operator>> (std::istream &s,tString &x){
    int i=0;
    x.SetLen(0);

    // eat whitespace
    std::ws(s);

    char c=s.get();

    // check if the string is quoted
    bool quoted = false;
    char quoteChar = c;   // if it applies, this is the quoting character
    if ( c == '"' || c == '\'' )
    {
        // yes, it is
        quoted = true;
        c = s.get();
    }

    bool lastEscape = false;
    while((quoted || !( isblank(c) || c == '\n' || c == '\r') ) && s.good() && !s.eof()){
        // read and interpret escape sequences
        bool thisEscape = false;
        if ( !lastEscape && !( thisEscape = st_ReadEscapeSequence( c, s ) ) && quoted && c == quoteChar )
        {
            // no escape, this string is quoted and the current character is an end quote. We're finished.
            c = s.get();
            break;
        }
        else
        {
            // append escaped or regular character
            x[i++]=c;
            c=s.get();
        }

        // lastEscape = thisEscape;
    }

    if(s.good())
    {
        s.putback(c);
    }
    x[i]='\0';
    return s;
}

#ifdef DEBUG
class tQuoteTester
{
    // checks that a specific input produces a specific output when read via <<
    static void Test( char const * in, char const * out )
    {
        std::stringstream s;
        s << in << " NEXTWORD";
        tString o;
        s >> o;
        tASSERT( o == out );
    }

public:
    tQuoteTester()
    {
        // basic stuff
        Test( "abc", "abc" );

        // quoted
        Test( "'abc'", "abc" );

        // escaped
        Test( "\\'a\\\"b", "'a\"b" );

        // chained backslashes
        Test( "\\\\", "\\" );
        Test( "\\\\\\\\", "\\\\" );
        Test( "'\\\\\\\\'", "\\\\" );
    }
};

static tQuoteTester tester;
#endif

// *******************************************************************************************
// *
// *   SetPos
// *
// *******************************************************************************************
//!
//!        @param  len the target length
//!        @param  cut if set, the string may be cut back if its current length is bigger than length
//!
// *******************************************************************************************

void tString::SetPos(int l, bool cut){
    int i;
    if ( l < Len() )
    {
        if ( cut )
        {
            if ( l > 0 )
            {
                SetLen( l - 1 );
                operator+=(' ');
            }
            else
            {
                SetLen( 0 );
            }
        }
        else
        {
            operator+=(' ');
        }
    }
    if( l == Len() && !cut)
    {
        operator+=(' ');
    }
    for(i=Len();i<l;i++)
        operator+=(' ');
}


//removed in favor of searching whole string...
/*void tString::RemoveStartColor(){
	tString oldname = *this;
//	unsigned short int colorcodelength = 0;
	tString newname = "";
	if (oldname.ge2("0x") == false){
		return;
	}
	newname.SetLen(14);
//	unsigned short int numcounter = 0;
	unsigned short int i = 0;
	for (i=2; i<oldname.Len(); i++){
		if (oldname(i) > 47 && oldname(i) < 58) {
//			std::cout << oldname(i) << std::endl;
			colorcodelength++;
		}
		else {
			break;
		}
	}
	unsigned short int c = 0;
	for (i=colorcodelength+2; i<oldname.Len(); i++) {
//		std::cout << oldname(i);
		newname(c) = oldname(i);
		c++;
	}
	for (i=8; i<oldname.Len(); i++) {
//		std::cout << oldname(i);
		newname(c) = oldname(i);
		c++;
	}
//	std::cout << std::endl << newname << std::endl;
	*this = newname;
}*/



//added by me (Tank Program)
//sees if a string starts with another string
//created for remote admin...

// *******************************************************************************************
// *
// *   StartsWith
// *
// *******************************************************************************************
//!
//!        @param  other  the string to compare the start with
//!        @return        true if this starts with other
//!
// *******************************************************************************************

bool tString::StartsWith( const tString & other ) const
{
    // const tString & rmhxt = *this;
    // rmhxt.RemoveHex();
    // *this = rmhxt;
    // other.RemoveHex();
    if (other.Len() > Len()) {
        return false;
    }
    for (int i=0; i<other.Len()-1; i++) {
        if (other(i) != (*this)(i)) {
            return false;
        }
    }

    return true;
}

// *******************************************************************************
// *
// *	StrPos
// *
// *******************************************************************************
//!
//!		@param	tofind	the string to find
//!		@return		    position of the start of the string or negative if not found
//!
// *******************************************************************************

int tString::StrRPos( const tString & tofind, int startPos) const
 {
    if (tofind.Len() > Len() || startPos >= Len() || startPos < 0) {
        return -1;
    }

    for (int i = startPos - tofind.Len(); i >= 0; i--) {
        if ((*this)(i) == tofind(0)) {
            bool found = true;
            for (int j = 0; j < tofind.Len(); j++) {
                if ((*this)(i + j) != tofind(j)) {
                    found = false;
                    break;
                }
            }
            if (found) return i;
        }
    }

    return -1;
}


int tString::StrRPos(const tString & tofind) const {
    if (tofind.Len() > Len()) {
        return -1;
    }

    for (int i = Len() - tofind.Len(); i >= 0; i--) {
        if ((*this)(i) == tofind(0)) {
            bool found = true;
            for (int j = 0; j < tofind.Len(); j++) {
                if ((*this)(i + j) != tofind(j)) {
                    found = false;
                    break;
                }
            }
            if (found) return i;
        }
    }

    return -1;
}

int tString::StrPos( const tString & tofind ) const
{
    if (tofind.Len() > Len()) {
        return -1;
    }
    for (int i=0; i<Len()-1; i++) {
        if ((*this)(i) == tofind(0)) {
            bool found = true;
            for (int j=0; j<tofind.Len()-1 && i+j < Len()-1; j++) {
                if ((*this)(i+j) != tofind(j))
                    found = false;
            }
            if (found == true)
                return i;
        }
    }

    return -1;
}

int tString::StrPos( int start, const tString & tofind ) const
{
    if (tofind.Len() > Len()) {
        return -1;
    }
    for (int i = start; i<Len()-1; i++) {
        if ((*this)(i) == tofind(0)) {
            bool found = true;
            for (int j=0; j<tofind.Len()-1; j++) {
                if ((*this)(i+j) != tofind(j))
                    found = false;
            }
            if (found == true)
                return i;
        }
    }

    return -1;
}

// *******************************************************************************
// *
// *	StrPos
// *
// *******************************************************************************
//!
//!		@param	tofind	the string to find
//!		@return		    position of the start of the string or negative if not found
//!
// *******************************************************************************

int tString::StrPos( const char * tofind ) const
{
    return StrPos( tString ( tofind ) );
}

int tString::StrPos( int start, const char * tofind ) const
{
    return StrPos( start, tString ( tofind ) );
}

tString tString::SubStr( const int start, int len) const
{
    tASSERT( start >= 0 );

    if (start > Len())
        return tString("");

    //if len < 0 or too long, take the whole string
    if ( len < 0 || (len + start) >= Len())
        len = Len() - start;

    tString toReturn("");

    for (int i=start; i<(len + start); i++) {
        toReturn << (*this)(i);
    }
    return  toReturn;
}


tString tString::SubStr( const int start ) const
{
    return SubStr (start, Len()-start-1 );
}

//based on GetInt
int tString::toInt( int pos ) const
{
    int ret = 0;
    int digit = 0;
    while ( pos < Len() && digit >= 0 && digit <= 9 )
    {
        ret = ret*10 + digit;
        digit = (*this)(pos) - '0';
        pos++;
    }

    // check whether the last character read was not whitespace or the end of the string
    if ( pos < Len() && !isblank((*this)(pos-1)) )
    {
        return 0;
    }

    return ret;
}

int tString::toInt() const {
    return toInt(0);
}

// *******************************************************************************************
// *
// *   StartsWith
// *
// *******************************************************************************************
//!
//!        @param  other  the string to compare the start with
//!        @return        true if this starts with other
//!
// *******************************************************************************************

bool tString::StartsWith( const char * other ) const
{
    return StartsWith( tString( other ) );
}

// *******************************************************************************************
// *
// *   EndsWith
// *
// *******************************************************************************************
//!
//!        @param  other  the string to compare the end with
//!        @return        true if this ends with other
//!
// *******************************************************************************************

bool tString::EndsWith(const tString & other ) const
{
    if (other.Len() > Len()) {
        return false;
    }
    tString thisString = Reverse();
    tString otherString(other);
    otherString = otherString.Reverse();

    // Haha, just use StartsWith to do the comparison :)
    return thisString.StartsWith(otherString);
    //return true;
}

// *******************************************************************************************
//!
//!        @param  other  the string to compare the end with
//!        @return        true if this ends with other
//!
// *******************************************************************************************

bool tString::EndsWith( const char* other) const {
    return EndsWith( tString(other) );
}

// *******************************************************************************
// *
// *	LTrim
// *
// *******************************************************************************
//!
//!		@return		the trimmed string
//!
// *******************************************************************************

tString tString::LTrim( void ) const
{
    tString toReturn;
    bool trim = true;

    for( size_t i = 0; i<Len(); i++ )
    {
        if( !isblank((*this)[i]) )
            trim = false;
        if( !trim)
            toReturn << (*this)[i];
    }
    return toReturn;
}

// *******************************************************************************
// *
// *	RTrim
// *
// *******************************************************************************
//!
//!		@return		the trimmed string
//!
// *******************************************************************************

tString tString::RTrim( void ) const
{
    int lastNonSpace = Len() - 2;
    while ( lastNonSpace >= 0 && ( (*this)[lastNonSpace] == 0 || isblank((*this)[lastNonSpace]) ) )
    {
        --lastNonSpace;
    }

    if ( lastNonSpace < Len() - 2 )
    {
        return SubStr( 0, lastNonSpace + 1 );
    }

    return *this;
}

// *******************************************************************************
// *
// *	Trim
// *
// *******************************************************************************
//!
//!		@return		the trimmed string
//!
// *******************************************************************************

tString tString::Trim( void ) const
{
    return LTrim().RTrim();
}

/*
bool tString::operator==(const tString &other) const
{
    if (other.Len() != Len())
        return false;
    for (int i= Len()-1; i>=0; i--)
        if (other(i) != (*this)(i))
            return false;

    return true;
}
*/

// Original ge2, didn't compile under linux, some stupid overload or something, so we now have ge2
/*bool tString::operator>=(const tString &other) const
{
	if (other.Len() > Len()) {
//		std::cout << "lenissue\n";
		return false;
	}
	for (int i=0; i<other.Len()-1; i++) {
		if (other(i) != (*this)(i)) {
//			std::cout << "matchissue: '" << other(i) << "' '" << (*this)(i) << "'\n";
			return false;
		}
//		else {
//			std::cout << other(i) << " " << (*this)(i) << std::endl;
//		}
	}

	return true;
} */

// char st_stringOutputBuffer[tMAX_STRING_OUTPUT];


/*
  void operator <<(tString &s,const char * c){
  std::stringstream S(st_stringOutputBuffer,tMAX_STRING_OUTPUT-1);
  S << c << '\0';
  s+=st_stringOutputBuffer;
  }

  void operator <<(tString &s,const unsigned char * c){
  std::stringstream S(st_stringOutputBuffer,tMAX_STRING_OUTPUT-1);
  S << c << '\0';
  s+=st_stringOutputBuffer;
  }

  void operator <<(tString &s,int c){
  std::stringstream S(st_stringOutputBuffer,tMAX_STRING_OUTPUT-1);
  S << c << '\0';
  s+=st_stringOutputBuffer;
  }

  void operator <<(tString &s,float c){
  std::stringstream S(st_stringOutputBuffer,tMAX_STRING_OUTPUT-1);
  S << c << '\0';
  s+=st_stringOutputBuffer;
  }
*/


std::stringstream& operator<<(std::stringstream& s,const tString &t)
{
    static_cast<std::ostream&>(s) << static_cast<const char *>(t);
    return s;
}

/*
std::stringstream& operator<<(std::stringstream& s, const int &t)
{
	static_cast<std::ostream&>(s) << static_cast<int >(t);
	return s;
}

std::stringstream& operator<<(std::stringstream& s, const float &t)
{
	static_cast<std::ostream&>(s) << static_cast<float>(t);
	return s;
}

std::stringstream& operator<<(std::stringstream& s, const short unsigned int &t)
{
	static_cast<std::ostream&>(s) << static_cast<int>(t);
	return s;
}

std::stringstream& operator<<(std::stringstream& s, const short int &t)
{
	static_cast<std::ostream&>(s) << static_cast<int>(t);
	return s;
}

std::stringstream& operator<<(std::stringstream& s, const unsigned int &t)
{
	static_cast<std::ostream&>(s) << static_cast<int>(t);
	return s;
}

std::stringstream& operator<<(std::stringstream& s, const unsigned long &t)
{
	static_cast<std::ostream&>(s) << static_cast<int>(t);
	return s;
}

std::stringstream& operator<<(std::stringstream& s, char t)
{
	static_cast<std::ostream&>(s) << t;
	return s;
}

std::stringstream& operator<<(std::stringstream& s, bool t)
{
	static_cast<std::ostream&>(s) << static_cast<int>(t);
	return s;
}


std::stringstream& operator<<(std::stringstream& s, const char * const &t)
{
	static_cast<std::ostream&>(s) << static_cast<const char *>(t);
	return s;
}
*/


tString & tString::operator=(const tString &s)
{
    // self copying is unsafe
    if ( &s == this )
        return *this;

    Clear();
    for (int i = s.Len()-1; i>=0; i--)
        operator[](i) = s(i);

    tASSERT( (*this) == s );

    return *this;
}


tString & tString::operator+=(const tString &s)
{
    if (Len() > 0 && operator()(Len()-1) == 0)
        SetLen(Len()-1);

    for (int i = 0; i< s.Len(); i++)
        operator[](Len()) = s(i);

    if (Len() > 0 && operator()(Len()-1) != 0)
        operator[](Len()) = 0;

    return *this;
}

/*

//static char* st_TempString = NULL;
#ifdef DEBUG
//static int   st_TempStringLength = 10;
#else
//static int   st_TempStringLength = 1000;
#endif

// static int

class tTempStringCleanup
{
public:
	~tTempStringCleanup()
		{
			if (st_TempString)
				free( st_TempString );

			st_TempString = NULL;
		}
};

static tTempStringCleanup cleanup;

char * tString::ReserveTempString()
{
	if (!st_TempString)
		st_TempString = reinterpret_cast<char*>( malloc(st_TempStringLength) );

	st_TempString[st_TempStringLength-1] = 0;
	st_TempString[st_TempStringLength-2] = 0;

	return st_TempString;
}


int    tString::TempStringLength()
{
	return st_TempStringLength;
}

void   tString::MakeTempStringLonger()
{
	free(st_TempString);
	st_TempString = NULL;
	st_TempStringLength *= 2;
}

*/

// exctact the integer at position pos plus 2^16 ( or the character ) [ implementation detail of CompareAlphanumerical ]
static int GetInt( const tString& s, int& pos )
{
    int ret = 0;
    int digit = 0;
    while ( pos < s.Len() && digit >= 0 && digit <= 9 )
    {
        ret = ret*10 + digit;
        digit = s[pos] - '0';
        pos++;
    }

    if ( ret > 0 )
    {
        return ret + 0x10000;
    }
    else
    {
        return digit + '0';
    }
}

// *******************************************************************************************
// *
// *   CompareAlphaNumerical
// *
// *******************************************************************************************
//!
//!        @param  a   first string to compare
//!        @param  b   second string to compare
//!        @return     -1 if b is bigger than a, +1 if a is bigger than b, 0 if they are equal
//!
// *******************************************************************************************

int tString::CompareAlphaNumerical( const tString& a, const tString &b)
{
    int apos = 0;
    int bpos = 0;

    while ( apos < a.Len() && bpos < b.Len() )
    {
        int adigit = GetInt( a,apos );
        int bdigit = GetInt( b,bpos );
        if ( adigit < bdigit )
            return 1;
        else if ( adigit > bdigit )
            return -1;
    }

    if ( a.Len() - apos < b.Len() - bpos )
        return 1;
    else if ( a.Len() - apos > b.Len() - bpos )
        return 1;
    else
        return 0;
}

// *******************************************************************************************
// *
// *	Compare
// *
// *******************************************************************************************
//!
//!     @param	other       the string to compare with
//!     @param  ignoreCase
//!     @return             negative if *this is lexicograhically less that other, 0 if they are equal, positive otherwise
//!
// *******************************************************************************************

int tString::Compare( const char* other, bool ignoreCase ) const
{
    if ( ignoreCase ) {
        return strcasecmp( *this, other );
    }
    else {
        return Compare( other );
    }
}

// *******************************************************************************************
// *
// *	Compare
// *
// *******************************************************************************************
//!
//!     @param	other 	the string to compare with
//!     @return         negative if *this is lexicograhically less that other, 0 if they are equal, positive otherwise
//!
// *******************************************************************************************

int tString::Compare( const char* other ) const
{
    if ( !other )
        return 1;
    if(strcmp( *this, other ) == 0)
       return 0;
    else
       return strcmp( *this, other );
}


// *******************************************************************************************
// *
// *	operator ==
// *
// *******************************************************************************************
//!
//!		@param	other	the string to compare with
//!		@return			true only if the two strings are equal
//!
// *******************************************************************************************

bool tString::operator ==( const char* other ) const
{
    return Compare( other ) == 0;
}

// *******************************************************************************************
// *
// *	operator !=
// *
// *******************************************************************************************
//!
//!		@param	other	the string to compare with
//!		@return			true only if the two strings are not equal
//!
// *******************************************************************************************

bool tString::operator !=( const char* other ) const
{
    return Compare( other ) != 0;
}



// *******************************************************************************************
// *
// *	operator <
// *
// *******************************************************************************************
//!
//!		@param	other	the string to compare with
//!		@return			true only if *this is lexicographically before other
//!
// *******************************************************************************************

bool tString::operator <( const char* other ) const
{
    return Compare( other ) < 0;
}

// *******************************************************************************************
// *
// *	operator>
// *
// *******************************************************************************************
//!
//!		@param	other	the string to compare with
//!		@return			true only if *this is lexicographically after other
//!
// *******************************************************************************************

bool tString::operator>( const char* other ) const
{
    return Compare( other ) > 0;
}

// *******************************************************************************************
// *
// *	operator <=
// *
// *******************************************************************************************
//!
//!		@param	other	the string to compare with
//!		@return			true only if *this is not lexicographically after other
//!
// *******************************************************************************************

bool tString::operator <=( const char* other ) const
{
    return Compare( other ) <= 0;
}

// *******************************************************************************************
// *
// *	operator >=
// *
// *******************************************************************************************
//!
//!		@param	other	the string to compare with
//!		@return			true only if *this is not lexicographically before other
//!
// *******************************************************************************************

bool tString::operator >=( const char* other ) const
{
    return Compare( other ) >= 0;
}

bool operator==( const char* first, const tString& second )
{
    return second == first;
}

bool operator!=( const char* first, const tString& second )
{
    return second != first;
}

bool operator<( const char* first, const tString& second )
{
    return second > first;
}

bool operator>( const char* first, const tString& second )
{
    return second < first;
}

bool operator<=( const char* first, const tString& second )
{
    return second >= first;
}

bool operator>=( const char* first, const tString& second )
{
    return second <= first;
}

bool operator==( const tString& first, const tString& second )
{
    return first.operator==( second );
}

bool operator!=( const tString& first, const tString& second )
{
    return first.operator!=( second );
}

bool operator<( const tString& first, const tString& second )
{
    return first.operator<( second );
}

bool operator>( const tString& first, const tString& second )
{
    return first.operator>( second );
}

bool operator<=( const tString& first, const tString& second )
{
    return first.operator<=( second );
}

bool operator>=( const tString& first, const tString& second )
{
    return first.operator>=( second );
}

// *******************************************************************************************
// *
// *	~tColoredString
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

tColoredString::~tColoredString( void )
{
}

// *******************************************************************************************
// *
// *	tColoredString
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

tColoredString::tColoredString( void )
{
}

// *******************************************************************************************
// *
// *	tColoredString
// *
// *******************************************************************************************
//!
//!		@param	other string to copy from
//!
// *******************************************************************************************

tColoredString::tColoredString( const tColoredString & other )
        :tString( other )
{
}

// *******************************************************************************************
// *
// *	tColoredString
// *
// *******************************************************************************************
//!
//!     @param  other string to copy from
//!
// *******************************************************************************************

tColoredString::tColoredString( const tString & other )
        :tString( other )
{
}

// *******************************************************************************************
// *
// *	tColoredString
// *
// *******************************************************************************************
//!
//!     @param  other C string to copy from
//!
// *******************************************************************************************

tColoredString::tColoredString( const char * other )
        :tString( other )
{
}

// *******************************************************************************************
// *
// *	tColoredString
// *
// *******************************************************************************************
//!
//!		@param	other	output to copy from
//!
// *******************************************************************************************

tColoredString::tColoredString( const tOutput & other )
        :tString( other )
{
}

// *******************************************************************************************
// *
// *	operator =
// *
// *******************************************************************************************
//!
//!		@param	c	C string to copy from
//!     @return     reference to this to chain assignments
//!
// *******************************************************************************************

tString & tColoredString::operator =( const char * c )
{
    tString::operator = ( c );
    return *this;
}

// *******************************************************************************************
// *
// *	operator =
// *
// *******************************************************************************************
//!
//!		@param	s	string to copy from
//!     @return     reference to this to chain assignments
//!
// *******************************************************************************************

tString & tColoredString::operator =( const tString & s )
{
    tString::operator = ( s );
    return *this;
}

// *******************************************************************************************
// *
// *	operator =
// *
// *******************************************************************************************
//!
//!		@param	s   output to copy from
//!		@return		reference to this to chain assignments
//!
// *******************************************************************************************

tString & tColoredString::operator =( const tOutput & s )
{
    tString::operator = ( s );
    return *this;
}

// *******************************************************************************************
// *
// *	HasColors
// *
// *******************************************************************************************
//!
//!		@param	c	C style string or tString to clear of color codes
//!		@return   	false/true - if colors (do not) exist
//!
// *******************************************************************************************

bool tColoredString::HasColors( const char *c )
{
    int len = strlen(c);

    // walk through string
    while (*c!='\0')
    {
        // skip color codes
        if (*c=='0' && len >= 2 && c[1]=='x')
        {
            return true;
        }
        else
        {
            c++;
            len--;
        }
    }

    return false;
}

// *******************************************************************************************
// *
// *	HasColors
// *
// *******************************************************************************************
//!
//!		@param	c	C style string or tString to clear of color codes
//!		@return   	string with lower cased color codes
//!
// *******************************************************************************************

tString tColoredString::LowerColors( const char *c )
{
    tString ret;
    int len = strlen(c);

    // walk through string
    while (*c!='\0')
    {
        // skip color codes
        if (*c=='0' && len >= 2 && c[1]=='x')
        {
            tString color_code;
            color_code << c[0] << c[1] << c[2] << c[3] << c[4] << c[5] << c[6] << c[7];
            if (color_code == "0xRESETT")
                ret << color_code;
            else
                ret << color_code.ToLower();

            c   += 8;
            len -= 8;
        }
        else
        {
            ret << (*(c++));
            len--;
        }
    }

    return ret;
}

// *******************************************************************************************
// *
// *	RemoveColors
// *
// *******************************************************************************************
//!
//!		@param	c	C style string or tString to clear of color codes
//!		@return   	cleared string
//!
// *******************************************************************************************

tString tColoredString::RemoveColors( const char * c, bool darkonly )
{
    // st_Breakpoint();
    tString ret;
    int len = strlen(c);
    bool removed = false;

    // walk through string
    while (*c!='\0'){
        // skip color codes
        if (*c=='0' && len >= 2 && c[1]=='x')
        {
            if( len >= 8 && darkonly )
            {
                tColor colorToFilter( c );
                if ( !colorToFilter.IsDark() || strncmp( c, "0xRESETT", 8 ) == 0 )
                    ret << c[0] << c[1] << c[2] << c[3] << c[4] << c[5] << c[6] << c[7];
                else
                    removed = true;

                c   += 8;
                len -= 8;
            }
            else if( len >= 8 )
            {
                c   += 8;
                len -= 8;
                removed = true;
            }
            else
            {
                // skip incomplete color codes, too
                return RemoveColors( ret, darkonly );
            }
            // st_Breakpoint();
        }
        else
        {
            ret << (*(c++));
            len--;
        }
    }

    // st_Breakpoint();

    return removed ? RemoveColors( ret, darkonly ) : ret;
}

// *******************************************************************************************
// *
// *	RemoveColors
// *
// *******************************************************************************************
//!
//!		@param	c	C style string to clear of color codes
//!		@return   	cleared string
//!
// *******************************************************************************************

tString tColoredString::RemoveColors( const char * c )
{
    return RemoveColors ( c, false );
}

// *******************************************************************************************
// *
// *	RemoveColorsLoose
// *
// *******************************************************************************************
//!
//!		@param	c	C style string or tString to clear of unescaped color codes
//!		@return   	cleared string
//!
// *******************************************************************************************

tString tColoredString::RemoveColorsLoose( const char * c )
{
    tString ret;
    int len = strlen(c);

    // walk through string
    while(*c!='\0')
    {
        if (*c=='0' && len >= 8 && c[1]=='x')
        {
            tString color; color << c[0] << c[1] << c[2] << c[3] << c[4] << c[5] << c[6] << c[7];
            if(color == "0xRESETT" || tColor::VerifyColorCode(color))
            {
                c += 8; len -= 8;
                continue;
            }
        }
        ret << (*(c++));
        --len;
    }
    return ret;
}

// *******************************************************************************************
// *
// *	RemoveBadColors
// *
// *******************************************************************************************
//!
//!		@param	c	C style string to process color codes in
//!		@return   	string with only valid colors
//!
// *******************************************************************************************
tString tColoredString::RemoveBadColors( const char * c )
{
    tString ret;
    int len = strlen(c);

    // walk through string
    while( (*c) != '\0' )
    {
        if( (*c) == '0' && len >= 8 && c[1] == 'x' )
        {
            tString color; color << c[0] << c[1] << c[2] << c[3] << c[4] << c[5] << c[6] << c[7];
            c += 8; len -= 8;
            if( color == "0xRESETT" || tColor::VerifyColorCode(color) )
                ret << color;
            else
                continue;
        }
        ret << (*(c++));
        --len;
    }
    return ret;
}

// *******************************************************************************************
// *
// *	EscapeBadColors
// *
// *******************************************************************************************
//!
//!		@param	c	C style string to process color codes in
//!		@param	lastKnownGoodColor	safe color
//!		@return   	string with only real colors
//!
// *******************************************************************************************
tString tColoredString::EscapeBadColors(const char *c,std::string lastKnownGoodColor)
{
    tString ret;
    int len = strlen(c);

    // walk through string
    while(*c!='\0')
    {
        if( (*c) == '0' && c[1] == 'x' )
        {
            if( len < 8 )
            {
                ret << (*(c++)); --len;
                ret << lastKnownGoodColor;
                continue;
            }
            tString color; color << c[0] << c[1] << c[2] << c[3] << c[4] << c[5] << c[6] << c[7];
            if(color == "0xRESETT" || tColor::VerifyColorCode(color))
            {
                lastKnownGoodColor = color;
                ret << color;
                c += 8; len -= 8;
            }
            else
            {
                ret << (*(c++)); --len;
                ret << lastKnownGoodColor;
                continue;
            }
        }
        ret << (*(c++));
        --len;
    }
    return ret;
}

// *******************************************************************************************
// *
// *	ReplaceBadColors
// *
// *******************************************************************************************
//!
//!		@param	c	C style string to process color codes in
//!		@return   	string with only valid colors
//!
// *******************************************************************************************
tString tColoredString::ReplaceBadColors( const char * c )
{
    tString ret;
    int len = strlen(c);

    // walk through string
    while( (*c) != '\0' )
    {
        if( (*c) == '0' && len >= 8 && c[1] == 'x' )
        {
            tString color; color << c[0] << c[1] << c[2] << c[3] << c[4] << c[5] << c[6] << c[7];
            if(color == "0xRESETT" || tColor::VerifyColorCode(color))
            {
                ret << color;
                c += 8; len -= 8;
            }
            else
            {
                ret << (*(c++));
                ret << "X";

                ++c; len -= 2;
                continue;
            }
        }
        ret << (*(c++));
        --len;
    }
    return ret;
}

extern bool st_verifyColorCodeStrictly;

// helper function: removes trailing color of string and returns number of chars
// used by color codes
static int RemoveTrailingColor( tString& s, int maxLen=-1 )
{
    // count bytes lost to color codes
    int posDisplacement = 0;
    int len = 0;

    // walk through string
    for ( int g=0; g < s.Len()-1; g++)
    {
        if (s(g) == '0' && s(g+1) == 'x')
        {
            // test if the code is legal ( not so far at the end that it overlaps )
            if ( s.Len() >= g + 9 )
            {
                // everything is in order, record color code usage and advance
                posDisplacement+=8;
                g+=7;
            }
            else if( !st_verifyColorCodeStrictly && maxLen <= 0 )
            {
                // illegal code! Remove it.
                s.SetLen( g );
                s[g]=0;
            }
        }
        else if ( maxLen > 0 )
        {
            if ( ++len >= maxLen )
            {
                // maximal end reached, cut it off
                s.SetLen( g );
                s[g]=0;
            }
        }
    }

    return posDisplacement;
}

// *******************************************************************************************
// *
// *	SetPos
// *
// *******************************************************************************************
//!
//!		@param	len     the desired length
//!		@param	cut	    only if set, the length of the string may be reduced
//!
// *******************************************************************************************


void tColoredString::SetPos( int len, bool cut )
{
    // determine desired raw length taking color codes into account and possibly cutting
    int wishLen = len + ::RemoveTrailingColor( *this, cut ? len : -1 );

    // delegate
    tString::SetPos( wishLen, cut );
}

// *******************************************************************************************
// *
// *	RemoveTrailingColor
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

void tColoredString::RemoveTrailingColor( void )
{
    // delegage
    ::RemoveTrailingColor( *this );
}

// filters illegal characters
class tCharacterFilter
{
public:
    tCharacterFilter()
    {
        int i;
        filter[0]=0;

        // map all unknown characters to underscores
        for (i=255; i>=0; --i)
        {
            filter[i] = '_';
        }

        // leave ASCII characters as they are
        // for (i=127; i>=32; --i)
        // no, leave all visible ISO Latin 1 characters as they are
        for ( i = 32; i <= 126; ++i )
            filter[ i ] = i;
        for ( i = 161; i <= 255; ++i )
            filter[ i ] = i;

        filter[ 160 ] = ' '; // non-breaking space

        // map return and tab to space
        SetMap('\n',' ');
        SetMap('\t',' ');

        //! map umlauts and stuff to their base characters
        /*
        SetMap(0xc0,0xc5,'A');
        SetMap(0xd1,0xd6,'O');
        SetMap(0xd9,0xdD,'U');
        SetMap(0xdf,'s');
        SetMap(0xe0,0xe5,'a');
        SetMap(0xe8,0xeb,'e');
        SetMap(0xec,0xef,'i');
        SetMap(0xf0,0xf6,'o');
        SetMap(0xf9,0xfc,'u');
        */

        //!todo: map weird chars
        // make this data driven.
    }

    char Filter( unsigned char in )
    {
        return filter[ static_cast< unsigned int >( in )];
    }
private:
    void SetMap( int in1, int in2, unsigned char out)
    {
        tASSERT( in2 <= 0xff );
        tASSERT( 0 <= in1 );
        tASSERT( in1 < in2 );
        for( int i = in2; i >= in1; --i )
            filter[ i ] = out;
    }

    void SetMap( unsigned char in, unsigned char out)
    {
        filter[ static_cast< unsigned int >( in ) ] = out;
    }

    char filter[256];
};

// *******************************************************************************************
// *
// *	NetFilter
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

void tString::NetFilter( void )
{
    static tCharacterFilter filter;

    // run through string
    for( int i = Len()-2; i>=0; --i )
    {
        // character to filter
        char & my = (*this)(i);

        my = filter.Filter(my);
    }
}

// *******************************************************************************************
// *
// *	RemoveHex
// *
// *******************************************************************************************
//!
//!
// *******************************************************************************************

void tColoredString::RemoveHex( void )
{
    /*tString oldS = *this;
    tString newS = "";

    for (int i=0; i<oldS.Len()-1; i++){
        if (oldS[i] == '0'){
            if (((oldS[i]+1) != oldS.Len()) && ((oldS[i+1] == 'x') || (oldS[i+1] == 'X')))
     {
     int r;
     int g;
    int b;
    char s[2];
    s[0] = oldS[i+2];
    s[1] = oldS[i+3];
    sscanf(s, "%x", &r);
    s[0] = oldS[i+4];
    s[1] = oldS[i+5];
    sscanf(s, "%x", &g);
    s[0] = oldS[i+6];
    s[1] = oldS[i+7];
    sscanf(s, "%x", &b);
    newS << ColorString(r/255, g/255, b/255);
    i = i + 8;
                if (i>=oldS.Len()){
                    break;
                }
            }
        }
        newS << oldS[i];
    }
    //        for (i=0; i<newS.Len()-1; i++){
    //        if (newS[i] == '0'){
    //            if (((newS[i]+1) != newS.Len()) && ((newS[i+1] == 'x') || (newS[i+1] == 'X'))){
    //    newS.RemoveHex();
    //    }
    //    }
    //    }
    //this = newS; */
}

bool st_colorStrings=true;

static tConfItem<bool> cs("COLOR_STRINGS",st_colorStrings);

static int RTC(REAL x){
    int ret=int(x*255);
    if (ret<0)
        ret=0;
    if (ret>255)
        ret=255;
    return ret;
}

static char hex_array[]="0123456789abcdef";

static char int_to_hex(int i){
    if (i<0 || i >15)
        return 'Q';
    else
        return hex_array[i];
}

tColoredString & operator <<(tColoredString &s, const tColoredStringProxy &colorCode )
{
    if (st_colorStrings)
    {
        if ( colorCode.r_ == -1 || colorCode.g_ == -1 || colorCode.b_ == -1 )
        {
            s << "0xRESETT";
            return s;
        }

        char cs[9];
        cs[0]='0';
        cs[1]='x';

        int RGB[3];
        RGB[0]=RTC(colorCode.r_);
        RGB[1]=RTC(colorCode.g_);
        RGB[2]=RTC(colorCode.b_);

        for(int i=0;i<3;i++){
            int lp=RGB[i]%16;
            int hp=(RGB[i]-lp)/16;
            cs[2+2*i]=int_to_hex(hp);
            cs[2+2*i+1]=int_to_hex(lp);
        }
        cs[8]=0;

        s << cs;
    }

    return s;
}

// *******************************************************************************************
// *
// *	StripWhitespace
// *
// *******************************************************************************************
//!
//!    @return     a string with all whitespace removed. "hi everyone " -> "hieveryone"
//!
// *******************************************************************************************

tString tString::StripWhitespace( void ) const
{
    tString toReturn;

    for( int i = 0; i<=Len()-2; i++ )
    {
        if( !isblank((*this)(i)) )
            toReturn << (*this)(i);
    }
    return toReturn;
}

tString tString::TrimWhitespace( void ) const
{
    tString toReturn = *this;

    while (!toReturn.empty() && isblank(toReturn[0]))
    {
        toReturn = toReturn.SubStr(1);
    }

    while (!toReturn.empty() && isblank(toReturn[toReturn.Len() - 1]))
    {
        toReturn = toReturn.SubStr(0, toReturn.Len() - 2);
    }

    return toReturn;
}



//! @return a string where all characters are converted to lowercase. "Hello World!" -> "hello world!"
tString tString::ToLower(void) const
{
    tString ret(*this);
    for( int i = ret.Len()-2; i >= 0; --i )
    {
        ret[i] = tolower( ret[i] );
    }
    return ret;
}

//! @return a string where all characters are converted to uppercase. "Hello World!" -> "HELLO WORLD!"
tString tString::ToUpper(void) const
{
    tString ret(*this);
    for( int i = ret.Len()-2; i >= 0; --i )
    {
        ret[i] = toupper( ret[i] );
    }
    return ret;
}

//  filtering all strings
class tStringFilter
{
public:
    tStringFilter()
    {
        int i;
        filter[0]=0;

        // map all unknown characters to underscores
        for (i=255; i>0; --i)
        {
            filter[i] = '_';
        }

        // leave ASCII characters as they are
        for (i=126; i>32; --i)
        {
            filter[i] = i;
        }

        // but convert uppercase characters to lowercase
        for (i='Z'; i>='A'; --i)
        {
            filter[i] = i + ('a' - 'A');
        }

        //! map umlauts and stuff to their base characters
        SetMap(0xc0,0xc5,'a');
        SetMap(0xd1,0xd6,'o');
        SetMap(0xd9,0xdD,'u');
        SetMap(0xdf,'s');
        SetMap(0xe0,0xe5,'a');
        SetMap(0xe8,0xeb,'e');
        SetMap(0xec,0xef,'i');
        SetMap(0xf0,0xf6,'o');
        SetMap(0xf9,0xfc,'u');

        // ok, some of those are a bit questionable, but still better than _...
        SetMap(161,'!');
        SetMap(162,'c');
        SetMap(163,'l');
        SetMap(165,'y');
        SetMap(166,'|');
        SetMap(167,'s');
        SetMap(168,'"');
        SetMap(169,'c');
        SetMap(170,'a');
        SetMap(171,'"');
        SetMap(172,'!');
        SetMap(174,'r');
        SetMap(176,'o');
        SetMap(177,'+');
        SetMap(178,'2');
        SetMap(179,'3');
        SetMap(182,'p');
        SetMap(183,'.');
        SetMap(185,'1');
        SetMap(187,'"');
        SetMap(198,'a');
        SetMap(199,'c');
        SetMap(208,'d');
        SetMap(209,'n');
        SetMap(215,'x');
        SetMap(216,'o');
        SetMap(221,'y');
        SetMap(222,'p');
        SetMap(231,'c');
        SetMap(241,'n');
        SetMap(247,'/');
        SetMap(248,'o');
        SetMap(253,'y');
        SetMap(254,'p');
        SetMap(255,'y');

        //map 0 to o because they look similar
        SetMap('0','o');

        // TODO: make this data driven.
    }

    char Filter( unsigned char in )
    {
        return filter[ static_cast< unsigned int >( in )];
    }
private:
    void SetMap( int in1, int in2, unsigned char out)
    {
        tASSERT( in2 <= 0xff );
        tASSERT( 0 <= in1 );
        tASSERT( in1 < in2 );
        for( int i = in2; i >= in1; --i )
            filter[ i ] = out;
    }

    void SetMap( unsigned char in, unsigned char out)
    {
        filter[ static_cast< unsigned int >( in ) ] = out;
    }

    char filter[256];
};

static bool st_IsUnderscore( char c )
{
    return c == '_';
}

bool tString::isNumber() const
{
    if (empty())
        return false;

    int dotCount = 0;
    bool hasDigit = false;

    for (int i = 0; i < Len(); ++i)
    {
        char c = (*this)(i);

        if (c == '\0')
            continue;

        if (i == 0 && c == '-')
            continue;

        if (c == '.')
        {
            dotCount++;
            if (dotCount > 1)
                return false;
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c)))
        {
            hasDigit = true;
        }
        else
        {
            return false;
        }
    }

    return hasDigit;
}




bool tString::containsNumber() const
{
    if (empty())
        return false;

    for (int i = 0; i < Len(); ++i)
    {
        char c = (*this)(i);

        if (c == '\0')
            continue;

        if (std::isdigit(static_cast<unsigned char>(c))) {
            return true;
        }
    }

    return false;
}

// function prototype for character testing functions
typedef bool TestSCharacter( char c );

// strips characters matching a test beginnings and ends of names
static void st_StripMatchingEnds( tString & stripper, TestSCharacter & beginTester, TestSCharacter & endTester)
{
    int len = stripper.Len() - 1;
    int first = 0, last = len;

    // eat whitespace from beginnig and end
    while ( first < len && beginTester( stripper[first] ) ) ++first;
    while ( last > 0 && ( !stripper[last] || endTester(stripper[last] ) ) ) --last;

    // strip everything?
    if ( first > last )
    {
        stripper = "";
        return;
    }

    // strip
    if ( first > 0 || last < stripper.Len() - 1 )
        stripper = stripper.SubStr( first, last + 1 - first );
}

//! @returns filtered string and with all of its characters in lowercased string
tString tString::Filter() const
{
    tString in(*this);
    tString out;
    out = tColoredString::RemoveColors( in );
    tStringFilter filter;

    // filter out illegal characters
    for (int i = out.Len()-1; i>=0; --i )
    {
        char & c = out[i];

        c = filter.Filter( c );
    }

    // strip leading and trailing unknown characters
    st_StripMatchingEnds( out, st_IsUnderscore, st_IsUnderscore );

    return out;
}

//static const char delimiters[] = "`~!@#$%^&*()-=_+[]\\{}|;':\",./<>? ";
//static tString delimiters("!?.:;_()-, ");
static tString delimiters("`~!@#$%^&*()-=_+[]\\{}|;':\",./<>? ");
static tConfItemLine st_wordDelimiters( "WORD_DELIMITERS", delimiters );

// *******************************************************************************************
// *
// *	PosWordRight
// *
// *******************************************************************************************
//!
//!    @param      start       The location of right to search
//!    @return                 Position relative from start to the delimiter to the right.
//!
// *******************************************************************************************

int tString::PosWordRight( int start ) const {
    int nextDelimiter = strcspn( SubStr( start, Len() ), delimiters );
    int toMove = nextDelimiter;

    // A delimter in our immediate path
    if ( toMove == 0 ) {
        // Move over delimiters
        while ( nextDelimiter == 0 && start + 1 < Len() ) {
            toMove++;
            start++;
            nextDelimiter = strcspn( SubStr( start, Len() ), delimiters );
        }
        // Skip over the word if not multiple delimiters
        if ( toMove == 1 ) {
            toMove += nextDelimiter;
        }
    }

    return toMove;
}

// *******************************************************************************************
// *
// *	PosWordLeft
// *
// *******************************************************************************************
//!
//!    @param      start       The location of left to search
//!    @return                 Position relative from start to the delimiter to the left.
//!
// *******************************************************************************************

int tString::PosWordLeft( int start ) const {
    return -1 * Reverse().PosWordRight( Len() - start - 1 );
}

// *******************************************************************************************
// *
// *	Reverse
// *
// *******************************************************************************************
//!
//!    @return                 A copy of the string reversed
//!
// *******************************************************************************************

tString tString::Reverse() const {
    tString reversed;
    for ( int index = Len() - 2; index >= 0; index-- ) {
        reversed << ( *this ) ( index );
    }
    return reversed;
}

// *******************************************************************************************
// *
// *    ExtractNonBlankSubString
// *
// *******************************************************************************************
//!
//!    @param   pos             Position where to start the extraction.
//!                             will contain the first position after the extracted string.
//!    @return                  The substring extracted. Leading blanks will not be taken.
//!
// *******************************************************************************************

tString tString::ExtractNonBlankSubString(int &pos, int numWordsToSkip) const {
    int currentPos = pos;
    tASSERT(currentPos >= 0);

    if (currentPos > Len())
        return tString("");

    // Skip the specified number of words
    for (int i = 0; i < numWordsToSkip; i++) {
        // Skip leading blanks
        while (currentPos < Len() && isblank((*this)(currentPos))) {
            currentPos++;
        }
        // Skip current word
        while (currentPos < Len() && !isblank((*this)(currentPos))) {
            currentPos++;
        }
    }

    // Extract the next non-blank substring
    tString toReturn("");
    while (currentPos < Len() && isblank((*this)(currentPos))) {
        currentPos++;
    }
    while (currentPos < Len() && !isblank((*this)(currentPos))) {
        toReturn << (*this)(currentPos++);
    }

    pos = currentPos;
    return toReturn;
}


// *******************************************************************************************
// *
// *	RemoveWordRight
// *
// *******************************************************************************************
//!
//!    @param      start       The location to start removing from
//!    @return                 Number of characters removed and to which direction
//!                                x > 0 right
//!                                x < 0 left
//!
// *******************************************************************************************

int tString::RemoveWordRight( int start ) {
    int removed = PosWordRight( start );
    RemoveSubStr( start, removed );
    return removed;
}

// *******************************************************************************************
// *
// *	RemoveWordLeft
// *
// *******************************************************************************************
//!
//!    @param      start       The location to start removing from
//!    @return                 Number of characters removed and to which direction
//!                                x > 0 right
//!                                x < 0 left
//!
// *******************************************************************************************

int tString::RemoveWordLeft( int start ) {
    int removed = PosWordLeft( start );
    RemoveSubStr( start, removed );
    return removed;
}

// *******************************************************************************************
// *
// *	RemoveSubStr
// *
// *******************************************************************************************
//!
//!    @param      start       The position in the string
//!    @param      length      How many characters to delete and the direction.
//!                                x > 0 deletes right
//!                                x < 0 deletes left
//!
// *******************************************************************************************

void tString::RemoveSubStr( int start, int length ) {
    int strLen = Len()-1;
    if ( length < 0 ) {
        start += length;
        length = abs( length );
    }

    if ( start + length > strLen || start < 0 || length == 0 ) {
        return;
    }

    if ( start == 0 ) {
        if ( strLen - length == 0 ) {
            *this = ("");
        }
        else {
            *this = SubStr( start + length, strLen );
        }
    }
    else {
        *this = SubStr( 0, start ) << SubStr( start + length, strLen );
    }

    SetLen(strLen+1-length);
}

// *******************************************************************************************
// *
// *	Truncate
// *
// *******************************************************************************************
//!
//!    @param      truncateAt       The postion to truncate at
//!    @return     A new string with the truncated text and "..." concatenated
//!
//!    Example: tString("word").Truncate( 2 ) -> "wo..."
//!
// *******************************************************************************************

tString tString::Truncate( int truncateAt ) const
{
    // The string does not need to be truncated
    if ( truncateAt >= Len() )
        return *this;

    return SubStr( 0, truncateAt ) << "...";
}

tString intTotString( int number )
{
    std::string s = std::to_string(number);
    return tString(s.c_str());
}

// *******************************************************************************************
// *
// *	tIsInList
// *
// *******************************************************************************************
//!
//!    @param      list       The string representation of the list
//!    @param      item       The item to look for
//!    @return     true if the list contains the item
//!
//!    Example: tIsInList( "ab, cd", "ab" ) -> true
//!
// *******************************************************************************************
//! check whether item is in a comma or whitespace separated list
bool tIsInList( tString const & list_, tString const & item )
{
    tString list = list_;

    while( list != "" )
    {
        // find the item
        int pos = list.StrPos( item );

        // no traditional match? shoot.
        if ( pos < 0 )
        {
            return false;
        }

        // check whether the match is a true list match
        if (
            ( pos == 0 || list[pos-1] == ',' || isblank(list[pos-1]) )
            &&
            ( pos + item.Len() >= list.Len() || list[pos+item.Len()-1] == ',' || isblank(list[pos+item.Len()-1]) )
            )
        {
            return true;
        }
        else
        {
            // no? truncate the list and go on.
            list = list.SubStr( pos + 1 );
        }
    }

    return false;
}

bool tIsInList(tString const &list_, int number)
{
    return tIsInList(list_, intTotString(number));
}

bool tRemoveFromList(tString &list, const tString &item)
{
    bool itemRemoved = false;
    int listLength = list.Len();

    while (list != "")
    {
        // find the item
        int pos = list.StrPos(item);

        // no traditional match? shoot.
        if (pos < 0)
        {
            break;
        }

        // check whether the match is a true list match
        if (
            (pos == 0 || list[pos - 1] == ',' || isblank(list[pos - 1])) &&
            (pos + item.Len() >= list.Len() || list[pos + item.Len()] == ',' || isblank(list[pos + item.Len()])))
        {
            // if match found construct the list without the item
            tString newList = list.SubStr(0, pos);

            // append remaining portion after the removed item
            if (pos + item.Len() < list.Len())
            {
                newList += list.SubStr(pos + item.Len() + 1); // +1 to skip the delimiter
            }

            list = newList;
            itemRemoved = true;
            break;
        }
        else
        {
            // no? Go on to the next portion
            list = list.SubStr(pos + 1);
        }
    }

    // restore list if no item was removed
    if (!itemRemoved)
    {
        list = list.SubStr(0, listLength);
    }

    return itemRemoved;
}

bool tRemoveFromList(tString &list, int number)
{
    return tRemoveFromList(list, intTotString(number));
}

// **********************************************************************
// *
// *	tToLower
// *
// **********************************************************************
//!
//!    @param      toTransform   The string to transform
//!
// **********************************************************************
void tToLower( tString & toTransform )
{
    for( int i = toTransform.Len()-2; i >= 0; --i )
    {
        toTransform[i] = tolower( toTransform[i] );
    }
}

// **********************************************************************
// *
// *	tToUpper
// *
// **********************************************************************
//!
//!    @param      toTransform   The string to transform
//!
// **********************************************************************
void tToUpper( tString & toTransform )
{
    for( int i = toTransform.Len()-2; i >= 0; --i )
    {
        toTransform[i] = toupper( toTransform[i] );
    }
}

#include "eChatCommands.h"
tString st_GetFormatTime(REAL seconds, bool color, bool showIfZero)
{
    int totalSeconds = static_cast<int>(seconds);
    int days = totalSeconds / 86400; // 86400 seconds in a day
    totalSeconds %= 86400;
    int hours = totalSeconds / 3600;
    totalSeconds %= 3600;
    int minutes = totalSeconds / 60;
    int remainingSeconds = totalSeconds % 60;
    int milliseconds = static_cast<int>((seconds - static_cast<int>(seconds)) * 1000);

    tString result;

    if (days > 0 || showIfZero)
    {
        result << (color ? tThemedTextBase.ItemColor() : "") 
               << days 
               << (color ? tThemedTextBase.MainColor() : "") 
               << "D";
        if (hours || minutes || remainingSeconds || milliseconds)
            result << ":";
    }

    if (hours > 0 || showIfZero)
    {
        result << (color ? tThemedTextBase.ItemColor() : "") 
               << ((hours < 10) ? "0" : "") 
               << hours 
               << (color ? tThemedTextBase.MainColor() : "") 
               << "H";
        if (minutes || remainingSeconds || milliseconds)
            result << ":";
    }

    if (minutes > 0 || showIfZero)
    {
        result << (color ? tThemedTextBase.ItemColor() : "") 
               << ((minutes < 10) ? "0" : "") 
               << minutes << (color ? tThemedTextBase.MainColor() : "") 
               << "M";
        if (remainingSeconds || milliseconds)
            result << ":";
    }

    if (remainingSeconds > 0 || showIfZero)
    {
        result << (color ? tThemedTextBase.ItemColor() : "") 
               << ((remainingSeconds < 10) ? "0" : "") 
               << remainingSeconds << (color ? tThemedTextBase.MainColor() : "") 
               << "S";
        if (milliseconds)
            result << ":";
    }

    if (milliseconds > 0 || showIfZero)
    {
        result << (color ? tThemedTextBase.ItemColor() : "") 
               << ((milliseconds < 100) ? (milliseconds < 10 ? "00" : "0") : "") 
               << milliseconds << (color ? tThemedTextBase.MainColor() : "") 
               << "MS";
    }

    if (result.empty())
        result << 0;
    return result;
}




tString st_GetCurrentTime( char const * szFormat , struct tm pTime)
{
    char szTemp[128];
    strftime(szTemp, sizeof(szTemp), szFormat, &pTime);
    return tString(szTemp);
}

// replacement for tString::EndsWith from the trunk
bool st_StringEndsWith( tString const & test, tString const & end )
{
    int start = test.Len() - end.Len();
    return start >= 0 && test.SubStr( start ) == end;
}

bool st_StringEndsWith( tString const & test, char const * end )
{
    return st_StringEndsWith( test, tString( end ) );
}

// **********************************************************************
// *
// *	Contains
// *
// **********************************************************************
//!
//!    @param      tofind       The string to search for within the stored string
//!    @return     true         Returns true if the string is found
//!
// **********************************************************************
#include <cstring>

bool tString::Contains(tString tofind)
{
    RecomputeLength();
    tofind.RecomputeLength();
    
    // if the length of tofind longer than the string, quit it!
    if (tofind.Len() > Len())
        return false;

    // Convert both strings to const char* for simpler comparison
    const char* thisStr = this->c_str();
    const char* tofindStr = tofind.c_str();

    // the total length of tofind, minus the 1 extra garbage
    int strCount = tofind.Len() - 1;

    for (int i = 0; i < Len() - strCount; ++i)
    {
        // Check if the substring starting from index i in thisStr matches tofindStr
        if (std::strncmp(thisStr + i, tofindStr, strCount) == 0)
        {
            return true;
        }
    }

    // if they don't match at all, too bad!
    return false;
}

bool tString::ContainsInsensitive(tString tofind)
{
    // if the length of tofind longer than the string, quit it!
    if (tofind.Len() > Len())
        return false;

    // Convert both strings to const char* for simpler comparison
    const char* thisStr = this->c_str();
    const char* tofindStr = tofind.c_str();

    // the total length of tofind, minus the 1 extra garbage
    int strCount = tofind.Len() - 1;

    for (int i = 0; i < Len() - strCount; ++i)
    {
        #ifdef _WIN32  
        if (_strnicmp(thisStr + i, tofindStr, strCount) == 0)
        #else 
        if (strncasecmp(thisStr + i, tofindStr, strCount) == 0)
        #endif
        {
            return true;
        }
    }

    // if they don't match at all, too bad!
    return false;
}



// void computeLPSArray(const tString& pat, int* lps)
// {
//     int len = 0;
//     int i = 1;

//     lps[0] = 0;

//     while (i < pat.Len())
//     {
//         if (pat[i] == pat[len])
//         {
//             len++;
//             lps[i] = len;
//             i++;
//         }
//         else
//         {
//             if (len != 0)
//             {
//                 len = lps[len - 1];
//             }
//             else
//             {
//                 lps[i] = 0;
//                 i++;
//             }
//         }
//     }
// }

// bool tString::Contains(tString tofind)
// {
//     int M = tofind.Len();
//     int N = Len();

//     int lps[M];
//     computeLPSArray(tofind, lps);

//     int i = 0;
//     int j = 0;

//     while (i < N)
//     {
//         if (tofind[j] == (*this)[i])
//         {
//             j++;
//             i++;
//         }

//         if (j == M)
//         {
//             return true;
//         }
//         else if (i < N && tofind[j] != (*this)[i])
//         {
//             if (j != 0)
//             {
//                 j = lps[j - 1];
//             }
//             else
//             {
//                 i++;
//             }
//         }
//     }
//     return false;
// }


bool tString::Contains(const char *tofind)
{
    return Contains(tString(tofind));
}

bool tString::ContainsInsensitive(const char *tofind)
{
    return ContainsInsensitive(tString(tofind));
}

bool tString::empty() const {
    return Len() == 0 || (*this)[0] == '\0';
}


// **********************************************************************
// *
// *	RemoveWord
// *
// **********************************************************************
//!
//!    @param      find_word       The word to remove from the string
//!    @return     newLine         Returns string without "find_word"
//!
// **********************************************************************

tString tString::RemoveWord(tString find_word)
{
    tString ret(*this);
    tString newLine;

    if (find_word.Len() > ret.Len())
        return tString("");
    else if (find_word == "")
        return ret;

    int count_word = find_word.Len() - 1;
    int this_word = ret.Len();

    for(int i = 0; i < this_word; i++)
    {
        tString putWordTogether;
        for(int j = 0; j < count_word; j++)
        {
            putWordTogether << ret[i+j];
        }

        if (putWordTogether == find_word)
            i += count_word - 1;
        else newLine << ret[i];
    }

    return newLine;
}

tString tString::RemoveWord(const char *find_word)
{
    return RemoveWord(tString(find_word));
}

tArray<tString> tString::SplitBySize(int size, bool fullWords)
{
    tString ret(*this);
    tArray<tString> arrayString;

    if (size <= 0)
    {
        return arrayString;
    }

    if (!fullWords)
    {
        int strLength = ret.Len();
        int numberOfChunks = (strLength + size - 1) / size;

        for (int i = 0; i < numberOfChunks; i++)
        {
            int startIndex = i * size;
            int endIndex = std::min(startIndex + size, strLength);
            arrayString[i] = ret.SubStr(startIndex, endIndex - startIndex);
        }
    }
    else
    {
        int index = 0;
        int arrayKey = 0;

        while (index < ret.Len())
        {
            int currentSize = 0;
            int lastSpace = -1;

            while (currentSize < size && index < ret.Len())
            {
                if (ret[index] == ' ')
                {
                    lastSpace = index;
                }
                currentSize++;
                index++;
            }

            if (index < ret.Len() && lastSpace != -1)
            {
                arrayString[arrayKey] = ret.SubStr(index - currentSize, lastSpace - (index - currentSize) + 1);
                index = lastSpace + 1;
            }
            else
            {
                arrayString[arrayKey] = ret.SubStr(index - currentSize, currentSize);
            }
            arrayKey++;
        }
    }

    return arrayString;
}


// **********************************************************************
// *
// *	Split
// *
// **********************************************************************
//!
//!    @param      delimiter       The expression to exclude when splitting string
//!    @return     arrayString     Returns string in array before and after "delimiter"
//!
// **********************************************************************
tArray<tString> tString::Split(tString delimiter)
{
    tString ret(*this);
    tArray<tString> arrayString;

    //  New Method
    int strleng = ret.Len() - 1;
    int delleng = delimiter.Len() - 1;
    if (delleng == 0)
        return arrayString;//no change

    int i = 0;
    int k = 0;
    while(i < strleng)
    {
        int j = 0;
        while (i + j < strleng && j < delleng && ret[i + j] == delimiter[j])
            j++;

        if (j == delleng)//found delimiter
        {
            arrayString[arrayString.Len()] = ret.SubStr(k, i - k);
            i += delleng;
            k = i;
        }
        else
        {
            i++;
        }
    }
    arrayString[arrayString.Len()] = ret.SubStr(k, i - k);

    return arrayString;
}

tArray<tString> tString::Split(const char *delimiter)
{
    return Split(tString(delimiter));
}

tArray<tString> st_explode(tString delimiter, tString ret)
{
    tArray<tString> arrayString;

    int strleng = ret.Len() - 1;
    int delleng = delimiter.Len() - 1;
    if (delleng == 0)
        return arrayString;//no change

    int i = 0;
    int k = 0;
    while(i < strleng)
    {
        int j = 0;
        while (i + j < strleng && j < delleng && ret[i + j] == delimiter[j])
            j++;

        if (j == delleng)//found delimiter
        {
            arrayString[arrayString.Len()] = ret.SubStr(k, i - k);
            i += delleng;
            k = i;
        }
        else
        {
            i++;
        }
    }
    arrayString[arrayString.Len()] = ret.SubStr(k, i - k);

    return arrayString;
}

// **********************************************************************
// *
// *	Replace
// *
// **********************************************************************
//!
//!    @param      old_word       The word to search up
//!    @param      new_word       Word to replace with the old one with
//!    @return     strString    Returns string
//!
// **********************************************************************

tString tString::Replace(tString old_word, tString new_word)
{
    std::string source = this->stdString();
    std::string oldStr = old_word.stdString();
    std::string newStr = new_word.stdString();

    size_t pos = 0;
    while((pos = source.find(oldStr, pos)) != std::string::npos)
    {
        source.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }

    return tString(source);
}



tString tString::Replace(const char *old_word, const char *new_word)
{
    return Replace(tString(old_word), tString(new_word));
}

size_t findCaseInsensitive(const std::string &strHaystack, const std::string &strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(),   strNeedle.end(),
        [](char ch1, char ch2) -> bool { return std::tolower(ch1) == std::tolower(ch2); }
    );

    if (it != strHaystack.end())
        return it - strHaystack.begin();
    else
        return std::string::npos;
}

tString tString::ReplaceInsensitive(tString old_word, tString new_word)
{
    std::string source = this->stdString();
    std::string oldStr = old_word.stdString();
    std::string newStr = new_word.stdString();

    size_t pos = 0;
    while ((pos = findCaseInsensitive(source, oldStr)) != std::string::npos)
    {
        source.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }

    return tString(source.c_str());
}

tString tString::ReplaceInsensitive(const char *old_word, const char *new_word)
{
    return ReplaceInsensitive(tString(old_word), tString(new_word));
}

// **********************************************************************
// *
// *	IsNumeric
// *
// **********************************************************************
//!
//!    @return     Returns true if string contains numeric values
//!
// **********************************************************************
bool tString::IsNumeric()
{
    tString ret(*this);
    tString numericValues("012456789ef.-+");

    //  return false when string is blank
    if (ret.Filter() == "")
        return false;

    for(int i = 0; i < ret.Len(); i++)
    {
        bool found = false;
        //  return false if current character in string is not a number
        for(int id = 0; id < numericValues.Len(); id++)
        {
            if (ret.ToLower()[i] == numericValues[id])
                found = true;
        }

        if (!found) return false;
    }

    //  looks like everything worked out. Good!
    return true;
}

// **********************************************************************
// *
// *	RemoveCharacter
// *
// **********************************************************************
//!
//!    @param      find_word       The word to remove from the string
//!    @return     newLine         Returns string without "find_word"
//!
// **********************************************************************

tString tString::RemoveCharacter(char character)
{
    tString ret(*this);
    tString newLine;

    for(int i = 0; i < ret.Len(); i++)
    {
        if (ret[i] != character)
            newLine << ret[i];
    }

    return newLine;
}

tString htmlentities(tString str)
{
    tString ret;

    for(int i = 0; i < (str.Len() - 1); i++)
    {
        switch (str[i])
        {
            case '\"':  ret << "&quot;";    break;
            case '\'':  ret << "&apos;";    break;
            case '<':   ret << "&lt;";      break;
            case '>':   ret << "&gt;";      break;
            case '*':   ret << "&#42;";     break;
            case '/':   ret << "&#47;";     break;
            case '|':   ret << "&#124;";    break;
            case ':':   ret << "&#58;";     break;
            case '?':   ret << "&#63;";     break;

            case '.': if(i == 0) { ret << "&#46;"; break;} //else, falls through...HACK to avoid hidden paths

            default: ret << str[i]; break;
        }
    }

    return ret;
}


size_t tString::Find(char ch, size_t start_pos) const {
    std::string this_str(*this); // Convert tString to std::string
    size_t pos = this_str.find(ch, start_pos);
    return pos;
}

tString& tString::Insert(size_t pos, const tString& str) {
    std::string this_str(*this); // Convert tString to std::string
    std::string insert_str(str); // Convert tString str to std::string
    this_str.insert(pos, insert_str);
    *this = tString(this_str.c_str()); // Assign the modified std::string back to tString
    return *this;
}

size_t tString::FindFirstOf(char ch, size_t start_pos) const {
    std::string this_str(*this); // Convert tString to std::string
    size_t pos = this_str.find_first_of(ch, start_pos);
    return pos;
}

size_t tString::FindLastOf(char ch, size_t start_pos) const {
    std::string this_str(*this); // Convert tString to std::string
    size_t pos = this_str.find_last_of(ch, start_pos);
    return pos;
}

size_t tString::FindStr(const tString& substring, size_t start_pos) const {
    std::string this_str(*this); // Convert tString to std::string
    std::string search_str(substring); // Convert tString substring to std::string
    size_t found_pos = this_str.find(search_str, start_pos);
    return found_pos;
}

tString& tString::Erase(size_t pos, size_t len) {
    std::string this_str(*this); // Convert tString to std::string
    this_str.erase(pos, len);
    *this = tString(this_str); // Assign the modified std::string back to tString
    return *this;
}





char tString::at(size_t pos) const {
    if (pos < Len()) {
        return (*this)(pos);
    } else {
        throw std::out_of_range("tString::At: position out of range");
    }
}


char* tString::Begin() {
    return &(*this)(0);
}

const char* tString::Begin() const {
    return &(*this)(0);
}


std::string tString::stdString() const {
    std::string str(*this);
    return str;
}

const char* tString::c_str() const {
    return operator const char*();
}

static void sg_copyToClipboard(std::istream &s)
{
    tString params;
    params.ReadLine(s, true);

    if (copyToClipboard(params))
        con << "Copied '" << params << "' to clipboard.\n";
    else
        con << "Failed to copy to clipboard.\n";

}

static tConfItemFunc sg_copyToClipboardConf("COPY_TO_CLIPBOARD", &sg_copyToClipboard);

bool copyToClipboard(tString contents)
{
    int contentsPos = contents.StrPos("\n");
    if (contentsPos != -1)
    {
        contents = tColoredString::RemoveColors(contents); // Remove newline character
    }

    #ifdef MACOSX
        return false;
    #endif

    #ifdef WIN32
    if (OpenClipboard(0))
    {
        EmptyClipboard();

        HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, contents.Len() + 1);
        char *pchData = static_cast<char *>(GlobalLock(hClipboardData));
        strcpy(pchData, contents);
        GlobalUnlock(hClipboardData);

        SetClipboardData(CF_TEXT, hClipboardData);
        CloseClipboard();
        return true;
    }
    #else
    std::string cmd = "echo -n " + contents + " | xclip -selection clipboard";
    if (system(cmd.c_str()) == 0)
    {
        return true;
    }
    #endif

    return false;
}

bool pasteFromClipboard(tString *content, int& cursorPos)
{
    #ifdef MACOSX
        return false;
    #endif

#ifndef WIN32
        std::unique_ptr<FILE, decltype(&pclose)> clip(popen("xclip -selection clipboard -o", "r"), pclose);
        if( clip )
#else
        if (OpenClipboard(0))
#endif
        {
    #ifdef WIN32
            HANDLE hClipboardData = GetClipboardData(CF_TEXT);
            char *pchData = (char*)GlobalLock(hClipboardData);
            tString cData(pchData);
    #else
            tString cData;
            char buf[128];
            while( std::fgets(buf, 128, clip.get()) )
            {
                cData << buf;
            }

            cData = st_UTF8ToLatin1( cData );
    #endif

        // replace newlines with spaces
        for (int i = 0; i < cData.Len(); ++i)
        {
            if (cData(i) == '\n' || cData(i) == '\r')
                cData(i) = ' ';
        }

        tString oContent(*content);
        tString aContent = oContent.SubStr(0, cursorPos);
        tString bContent = oContent.SubStr(cursorPos);

        tString nContent = aContent + cData + bContent;

        *content = nContent;
        cursorPos += cData.Len() - 1;

        GlobalUnlock(hClipboardData);
        CloseClipboard();

        return true;
    }

    return false;
}

tString getTimeAgoString(REAL seconds)
{
    tString result;

    if (seconds < 60)
    {
        result << std::to_string((int)seconds) << " seconds ago";
    }
    else if (seconds < 3600)
    {
        REAL minutes = seconds / 60;
        result << std::to_string((int)minutes) << " minutes ago";
    }
    else if (seconds < 86400)
    {
        REAL hours = seconds / 3600;
        result << std::to_string((int)hours) << " hours ago";
    }
    else
    {
        REAL days = seconds / 86400;
        result << std::to_string((int)days) << " days ago";
    }

    return result;
}

tString randomStr(std::string charset, int length)
{
    std::string randomStr;
    randomStr.resize(length);
    for (int i = 0; i < length; i++)
    {
        int index = rand() % charset.length();
        randomStr[i] = charset[index];
    }
    return tString(randomStr);
}

tString tThemedText::LabelText(const std::string& label) const
{
    tString output;
    output << HeaderColor()
           << label
           << " - "
           << MainColor();

    return output;
}

// Bright Red for headers
tString st_ThemeBaseColorHeader("0xff0033");
static tConfItem<tString> st_ThemeBaseColorHeaderConf("LABEL_BASE_COLOR_HEADER", st_ThemeBaseColorHeader);
// White for main
tString st_ThemeBaseColorMain("0xffffff");
static tConfItem<tString> st_ThemeBaseColorMainConf("LABEL_BASE_COLOR_MAIN", st_ThemeBaseColorMain);
// Dark Red for items
tString st_ThemeBaseColorItem("0xee0000");
static tConfItem<tString> st_ThemeBaseColorItemConf("LABEL_BASE_COLOR_ITEM", st_ThemeBaseColorItem);
// Pinkish red for error messages as an accent color
tString st_ThemeBaseColorError("0xee5577");
static tConfItem<tString> st_ThemeBaseColorErrorConf("LABEL_BASE_COLOR_ERROR", st_ThemeBaseColorError);


tThemedText tThemedTextBase(st_ThemeBaseColorHeader, st_ThemeBaseColorMain, st_ThemeBaseColorItem, st_ThemeBaseColorError);