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

#ifndef ArmageTron_STRING_H
#define ArmageTron_STRING_H

#include "config.h"

#include "tArray.h"
#include "tMemStack.h"

#include <sstream>
#include <iostream>
#include <iosfwd>

//typedef tArray<char> string;
class tOutput;

class tString:public tArray<char>{
private:
public:
    tString();
    tString(const tString &);
    explicit tString(const char *);
    explicit tString(std::string);
    explicit tString(const tOutput &);

    void ReadLine(std::istream &s, bool enableEscapeSequences=false);

    tString & operator+=(const char *c);
    tString & operator=(const char *c);
    tString & operator<<(const char *c);
    tString operator+(const char *c) const;
    tString & operator<<(char c);
    tString & operator+=(char c);
    tString operator+(char c) const;

    // comparison operators
    int Compare( const char* other ) const;		// wrapper for strcmp
    int Compare( const char* other, bool ignoreCase ) const; // strcasecmp

    bool operator==( const char* other ) const;
    bool operator!=( const char* other ) const;
    bool operator<( const char* other ) const;
    bool operator>( const char* other ) const;
    bool operator<=( const char* other ) const;
    bool operator>=( const char* other ) const;

    operator const char*() const;
    const char* c_str() const;

    tString & operator =(const tString &s);
    tString & operator =(const tOutput &s);
    tString & operator+=(const tString &s);

    bool isNumber() const;
    bool containsNumber() const;

    // Z-Man: stupid, stupid disambiguation for Visual C++.
    // not harmful to GCC, but not required.
#ifdef _MSC_VER
    template<class I> char& operator[](I i) {
        return tArray<char>::operator [](i);
    };
#endif

    // bool operator==(const tString &other) const;
    //	bool operator>=(const tString &other) const;

    //	static char * ReserveTempString();
    //	static int    TempStringLength();
    //	static void   MakeTempStringLonger();
    //	static void   FreeTempString();

    void Clear(){
        tArray<char>::Clear();
    }

    //! makes this string exactly of length len.
    void SetPos( int len, bool cut );

    bool EndsWith( const tString & other) const;    //!< determines whether this string ends with the argument string
    bool EndsWith( const char * other) const;        //!< determines whether this string ends with the argument string

    //! determines whether this string starts with the argument string
    bool StartsWith( const tString & other ) const;
    bool StartsWith( const char * other ) const;

    tString LTrim() const;                          //!< returns a copy with leading whitespace removed
    tString RTrim() const;                          //!< returns a copy with trailing whitespace removed
    tString Trim() const;                           //!< returns a copy with leading and trailing whitespace removed

    //Get the position of a substring within a string...
    int StrRPos(const tString & tofind) const;
    int StrRPos( const tString & tofind, int startPos) const;
    int StrPos( const tString &tofind ) const;      //!< Get the position of a substring within this string.
    int StrPos( int start, const tString &tofind ) const; //!< Get the position of a substring within this string.
    int StrPos( const char * tofind ) const;        //!< Get the position of a substring within this string.
    int StrPos( int start, const char * tofind ) const; //!< Get the position of a substring within this string.

    std::string stdString()  const;

    tString& Insert(size_t pos, const tString& str);
    size_t FindFirstOf(char ch, size_t start_pos) const;
    size_t FindLastOf(char ch, size_t start_pos) const;
    char at(size_t pos) const;

    //Get a substring within a string...
    tString SubStr( const int start, int len) const;
    tString SubStr( const int start ) const;

    //toInt as getInt is funky...
    int toInt( const int pos ) const;
    int toInt() const;

    //toReal for REAL types
    REAL toReal( const REAL pos) const;
    REAL toReal() const;

    bool IsNumeric() const;

    //! confirms whether the tofind exists within the current string
    bool Contains(tString tofind);
    bool ContainsInsensitive(tString tofind);
    bool Contains(const char *tofind);
    bool ContainsInsensitive(const char *tofind);

    bool empty() const;

    void RecomputeLength()
    {
        int actualLength = 0;
        while((*this)[actualLength] != '\0' && actualLength < Size())
        {
            actualLength++;
        }

        SetLen(actualLength + 1);
    }


    //! remove the specified character and return string
    tString RemoveCharacter(char character);

    //! compares two strings alphanumerically
    static int CompareAlphaNumerical( const tString& a, const tString &b);

    template <typename T>
    static tString ConvertToTString(const T& passedVal) {
        tString value;
        value << passedVal;
        return value;
    }

    //! strips all whitespace from a string
    tString StripWhitespace( void ) const;
    tString TrimWhitespace( void ) const;

    //! Converts the string to lowercase
    tString ToLower(void) const;
    //! Converts the string to uppercase
    tString ToUpper(void) const;

    //! Filters string and returns all characters in lowercased string
    tString Filter() const;

    int PosWordRight(int start) const;          //! Computes the position of the next delimiter relative to start
    int PosWordLeft(int start) const;           //! Computes the position of the previous delimiter relative to start
    int RemoveWordRight(int start);             //! Remove word right according to the delimiters
    int RemoveWordLeft(int start);              //! Remove word left according to the delimiters
    void RemoveSubStr(int start, int length);   //! Remove a substring, in-place
    tString Reverse() const;                    //! Reverses strings

    //  removes that word from the string if it exists
    tString RemoveWord(tString find_word);
    tString RemoveWord(const char *find_word);

    //  splits string intoarrays
    tArray<tString> Split(tString del_word);
    tArray<tString> Split(const char *del_word);
    tArray<tString> SplitForPlayers();
    tArray<tString> SplitBySize(int size, bool fullWords = false);

    bool MatchesAtIgnoreCase(int startPos, const tString &pattern) const;
    tString Replace(const tString &old_word, const tString &new_word);
    tString Replace(const char *old_word, const char *new_word);
    tString ReplaceInsensitive(const tString &old_word, const tString &new_word);
    tString ReplaceInsensitive(const char *old_word, const char *new_word);

    tString ExtractNonBlankSubString( int &pos, int numWordsToSkip = 0) const; //!< Extract non blank char sequence starting at pos

    //! Truncate a string
    tString Truncate( int truncateAt ) const;

    void NetFilter();                           //!< filters strings from the net for strange things like newlines
};

class tThemedText
{
public:
    tThemedText(tString& headerColor, tString& mainColor, tString& itemColor, tString& errorColor) :
        headerColor(headerColor), mainColor(mainColor), itemColor(itemColor), errorColor(errorColor) {}

    tString HeaderColor() const { return headerColor; }
    tString MainColor()   const { return mainColor;   }
    tString ItemColor()   const { return itemColor;   }
    tString ErrorColor()  const { return errorColor;  }

    tString LabelText(const std::string& label) const;

private:
    tString& headerColor;
    tString& mainColor;
    tString& itemColor;
    tString& errorColor;
};

extern tThemedText tThemedTextBase;
extern tString st_ThemeBaseColorHeader, st_ThemeBaseColorMain, st_ThemeBaseColorItem, st_ThemeBaseColorError;

//! proxy class for inserting color markings
struct tColoredStringProxy
{
    REAL r_, g_, b_;

    tColoredStringProxy( REAL r, REAL g, REAL b )
            : r_(r), g_(g), b_(b)
    {}
};

//!< strings that know about color markings
class tColoredString: public tString
{
public:

    ~tColoredString();                                      //!< Destructor
    tColoredString();                                       //!< Default constructor
    tColoredString( const tColoredString& other );          //!< Copy constructor
    explicit tColoredString( const tString& other );        //!< Base copy constructor
    explicit tColoredString( const char * other );          //!< Constructor from raw C string
    explicit tColoredString( const tOutput & other );       //!< Constructor from output gatherer

    //! Assignment operators
    tString & operator = ( const char * c );
    tString & operator = ( const tString & s );
    tString & operator = ( const tOutput & s );
    tColoredString & operator = ( const tColoredString & s ) = default;

    static tString RemoveColors( const char *c );           //!< Removes the color codes from a string
    static tString RemoveColors( const char *c, bool darkonly );           //!< Removes the color codes from a string

    static tString RemoveColorsLoose( const char *c );      //!< Removes only actual unescaped color codes from a string

    static tString RemoveBadColors( const char * c );      //!< Remove invalid color codes

    static tString EscapeBadColors( const char *c, std::string lastKnownGoodColor = "0xRESETT" ); // !< Cancels out nonexistant color codes.
    static tString ReplaceBadColors( const char * c ); // !< Replaces invalid color codes with can't-be-interpreted as color code


    void SetPos( int len, bool cut=false );                 //!< Makes sure string has length len when color codes are removed

    void RemoveTrailingColor();                             //!< Removes trailing, unfinished color code

    void RemoveHex();                                       //!< ?

    static bool HasColors( const char *c );         //!< Checks whether the string contains any string or not.
    static tString LowerColors (const char *c );    //!< Sets all color codes in string to lower cased

    //! Creates a color string inserter
    inline static tColoredStringProxy ColorString( REAL r,REAL g,REAL b )
    {
        return tColoredStringProxy( r, g, b );
    }
};

std::ostream & operator<< (std::ostream &s,const tString &x);
std::istream & operator>> (std::istream &s,tString &x);

//! check whether item is in a comma or whitespace separated list
bool tIsInList( tString const & list, tString const & item );
bool tIsInList( tString const & list, int number );

bool tIsEnabledForPlayer(tString const &list_, int playerID);

bool tRemoveFromList(tString & list, const tString & item);
bool tRemoveFromList(tString & list, int number);


tString tintTotString( int number );

//! converts a string to lowercase
void tToLower( tString & toTransform );

//! converts a string to uppercase
void tToUpper( tString & toTransform );

tString htmlentities(tString str);

tString randomStr(std::string charset, int length);

//#define tMAX_STRING_OUTPUT 1000
//extern char st_stringOutputBuffer[tMAX_STRING_OUTPUT];

/*
  tString & operator <<(tString &s,const char* c){
  return s+=c;
  }

  tString & operator <<(tString &s,char c){
  return s+=c;
  }
*/

//! string building streaming operator
template<class T> tString & operator <<(tString &s,const T &c)
{
    std::ostringstream S;

    S << c << '\0';

    return s+=S.str().c_str();
}

//! string building streaming operator
tColoredString & operator <<(tColoredString &s, const tColoredStringProxy &colorCode );

//! string building streaming operator
template<class T> tColoredString & operator <<(tColoredString &s,const T &c)
{
    // delegate to basic string function...
    static_cast< tString& >( s ) << c;

    // but don't allow overhanging color codes
    s.RemoveTrailingColor();

    return s;
}

std::stringstream& operator<<(std::stringstream& s,const tString &t);
//std::stringstream& operator<<(std::stringstream& s, const int &t);
//std::stringstream& operator<<(std::stringstream& s, const float &t);
//std::stringstream& operator<<(std::stringstream& s, const short unsigned int &t);
//std::stringstream& operator<<(std::stringstream& s, const short int &t);
//std::stringstream& operator<<(std::stringstream& s, const unsigned int &t);
//std::stringstream& operator<<(std::stringstream& s, const unsigned long &t);
//std::stringstream& operator<<(std::stringstream& s, char t);
//std::stringstream& operator<<(std::stringstream& s, bool t);
//std::stringstream& operator<<(std::stringstream& s, const char * const &t);

bool operator==( const char* first, const tString& second );
bool operator!=( const char* first, const tString& second );
bool operator<( const char* first, const tString& second );
bool operator>( const char* first, const tString& second );
bool operator<=( const char* first, const tString& second );
bool operator>=( const char* first, const tString& second );

bool operator==( const tString& first, const tString& second );
bool operator!=( const tString& first, const tString& second );
bool operator<( const tString& first, const tString& second );
bool operator>( const tString& first, const tString& second );
bool operator<=( const tString& first, const tString& second );
bool operator>=( const tString& first, const tString& second );


/*
  void operator <<(tString &s,const char * c);
  void operator <<(tString &s,const unsigned char * c);
  void operator <<(tString &s,int c);
  void operator <<(tString &s,float c);
*/
#include "tSysTime.h"

tString st_GetCurrentTime(char const *szFormat, struct tm pTime = getCurrentLocalTime());
tString st_GetFormatTime(REAL seconds, bool color = false, bool showIfZero = true);
// replacement for tString::EndsWith from the trunk
bool st_StringEndsWith( tString const & test, tString const & end );
bool st_StringEndsWith( tString const & test, char const * end );

extern tArray<tString> str_explode(tString delimiter, tString ret);

bool copyToClipboard(tString contents);
bool pasteFromClipboard(tString *content, int& cursorPos);

namespace std {
    template <>
    struct hash<tString> {
        std::size_t operator()(const tString& k) const {
            std::size_t res = 0;
            const char* cStr = k;
            while (*cStr != '\0') {
                res = res * 101  +  *cStr++;
            }
            return res;
        }
    };
}
tString getTimeAgoString(REAL seconds);

#endif


