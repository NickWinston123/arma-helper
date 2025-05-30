/*

*************************************************************************

ArmageTron -- Just another Tron Lightcycle Game in 3D.
Copyright (C) 2000  Manuel Moos (manuel@moosnet.de)
Copyright (C) 2004  Armagetron Advanced Team (http://sourceforge.net/projects/armagetronad/)

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

#ifndef ArmageTron_MENU_H
#define ArmageTron_MENU_H

#include "rFont.h"

#include "tList.h"
#include "tString.h"
#include "tCallback.h"
#include "tLocale.h"
#include "tDirectories.h"

#include "rSDL.h"
#ifndef DEDICATED
#include <SDL_events.h>
#endif

#include <deque>

#include "defs.h"

class uMenuItem;

class uMenu{
    friend class uMenuItem;

protected:
    tList<uMenuItem>      items;
    static FUNCPTR       idle;

    REAL menuTop;
    REAL menuBot;
    REAL yOffset;

    bool                 exitFlag;

    REAL                 spaceBelow;
    REAL                 center;

    int                  selected;

    REAL YPos(int num);
public:
    static bool          wrap;
    
    // different quick exit types
    enum QuickExit
    {
        QuickExit_Off  = 0, // no quick exit, keep going
        QuickExit_Game = 1, // quit to game selection menu
        QuickExit_Total = 2 // quit the program
    };

    static QuickExit     quickexit;
    static bool          exitToMain;

    tOutput              title;

    FUNCPTR IdleFunc(){return idle;}
    static void SetIdle(FUNCPTR idle_func) {idle=idle_func;}

    // poll input, return true if ESC was pressed
    static bool IdleInput( bool processInput );

    void SetCenter(REAL c) {center=c;}
    void SetTop(REAL t) {menuTop=t;}
    void SetBot(REAL b) {menuBot=b;spaceBelow=1+menuBot;}
    REAL GetTop() const {return menuTop;}
    REAL GetBot() const {return menuBot;}
    void SetSelected(int s) {selected = s;}
    int GetSelected() const {return selected;}
    int  NumItems()         {return items.Len();}
    uMenuItem* Item(int i)  { return items[i]; }
    void AddItem(uMenuItem* item);
    void RemoveItem(uMenuItem* item);

#ifdef SLOPPYLOCALE
    uMenu(const char *t,bool exit_item=true);
#endif
    uMenu(const tOutput &t,bool exit_item=true);
    virtual ~uMenu();

    //! enters the menu; calls idle_func before rendering every frame
    inline void Enter(){OnEnter();}

    void ReverseItems();

    // paints a nice background
    static void GenericBackground(REAL top=1.0);

    // marks the menu for exit
    inline void Exit(){OnExit();}

    void RequestSpaceBelow(REAL x){
        if (spaceBelow<x)
            spaceBelow=x;
    }

    // print a big message and a small interpretation
    static bool Message(const tOutput& message, const tOutput& interpretation, REAL timeout = -1);

    //! returns whether there is currently an active menu
    static bool MenuActive();
protected:
    //! handles a key press
    virtual void HandleEvent( SDL_Event event );

    //! enters the menu; calls idle_func before rendering every frame
    virtual void OnEnter();

    //! marks the menu for exit
    virtual void OnExit();

    int GetNextSelectable(int start);
    int GetPrevSelectable(int start);


    //! called every frame before the menu is rendered
    virtual void OnRender();
};


// *****************************************************

class uMenuItem{
    friend class uMenu;

    int idnum;
    uMenuItem(){}
protected:
    uMenu  *menu;
    tOutput helpText;

    void DisplayText(REAL x,REAL y,const char *text,bool selected,
                     REAL alpha=1,int center=0,int cursor=0,int cursorPos=0, rTextField::ColorMode colorMode = rTextField::COLOR_USE );
    void DisplayTextSpecial(REAL x,REAL y,const char *text,bool selected,
                            REAL alpha=1,int center=0);
public:
    uMenuItem(uMenu *M,const tOutput &help)
            :idnum(-1),menu(M),helpText(help){
        menu->items.Add(this,idnum);
    }

#ifdef SLOPPYLOCALE
    uMenuItem(uMenu *M,const char *help)
            :idnum(-1),menu(M),helpText(help){
        menu->items.Add(this,idnum);
    }
#endif

    virtual ~uMenuItem()
    {
        if (menu && idnum >= 0)
            menu->items.Remove(this,idnum);
    }

    //! called when the menu item is selected, the incoming parameter says
    //! whether help should be displayed, the function returns true if
    //! the menu code itself should handle the display or whether the menu item
    //! does that.
    virtual bool DisplayHelp( bool display, REAL y, REAL alpha ){return display;}

    virtual tString Help(){return tString(helpText);}
    // displays the menuitem at position x,y. set selected to true
    // if the item is currently under the cursor
    virtual void Render(REAL ,REAL ,REAL =1,bool =0){}

    virtual void RenderBackground(){
        menu->GenericBackground();
    }

    // if the user presses left/right on menuitem
    virtual void LeftRight(int ){} //lr=-1:left lr=+1: right
    virtual void LeftRightRelease(){}

    virtual void Enter(){} // if the user presses enter/space on menu

    virtual bool Event(SDL_Event &){return false;} // if the key c is
    // pressed,mouse moved ...
    // while menuitem is active; return value: key was used

    virtual REAL SpaceRight(){return 0;}

    int GetID(){return idnum;}

    virtual bool IsSelectable(){return true;};

protected:
    void SetColor( bool selected, REAL alpha );            //!< Sets the color of text output for this menuitem
};


// *****************************************************
// Menu Exit
// *****************************************************

class uMenuItemExit:public uMenuItem{
    tOutput t;

    static const tOutput& ExitText();
    static const tOutput& ExitHelp();

public:
    uMenuItemExit(uMenu *M,
                  const tOutput &text = ExitText(),
                  const tOutput &help = ExitHelp())
            :uMenuItem(M,help)
    ,t(text){}

    // displays the menuitem at position x,y. set selected to true
    // if the item is currently under the cursor
    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0){
        DisplayTextSpecial(x,y,tString(t),selected,alpha);
    }

    virtual void Enter(){menu->Exit();}
    // if the user presses enter/space on menu
};

// *****************************************************
// Selection
// *****************************************************

template<class T> class uSelectItem{
public:
    int idnum;

    tOutput description;
    tOutput helpText;
    T value;

    uSelectItem(const tOutput& desc,const tOutput& help,T val)
            :idnum(-1),description(desc),helpText(help),value(val){}
};

class uMenuItemDivider: public uMenuItem
{
    public:
        uMenuItemDivider(uMenu *M)
        : uMenuItem( M, tOutput() )
        {
        }

        virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0)
        {
        }

        virtual bool IsSelectable()
        {
            return false;
        }
};

template<class T> class uMenuItemSelection:public uMenuItem{
protected:
    tList<uSelectItem<T> >	choices;
    tOutput               	title;
    int                   	select;
    T *                   	target;
public:
#ifdef SLOPPYLOCALE
    uMenuItemSelection(uMenu *m,
                       const char* tit,const char *help,
                       T &targ)
            :uMenuItem(m,help),title(tit),select(0),target(&targ){}
#endif

    uMenuItemSelection(uMenu *m,
                       const tOutput &tit,const tOutput &help,
                       T &targ)
            :uMenuItem(m,help),title(tit),select(0),target(&targ){}

    ~uMenuItemSelection(){
        Clear();
    }

    void Clear(){
        for(int i=choices.Len()-1;i>=0;i--){
            uSelectItem<T> *x=choices(i);
            choices.Remove(x,x->idnum);
            delete x;
        }
        select=0;
    }

    void NewChoice(uSelectItem<T> *c){
        choices.Add(c,c->idnum);
    }

    void NewChoice(const tOutput& desc,const tOutput& help,T val){
        uSelectItem<T> *x=new uSelectItem<T>(desc,help,val);
        NewChoice(x);
    }

    virtual void LeftRight(int lr){
        select+=lr;
        if(select>=choices.Len())
            select=choices.Len()-1;
        if(select<0)
            select=0;
        if (choices.Len())
            *target=choices(select)->value;
    }

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0){
        for(int i=choices.Len()-1;i>=0;i--)
            if (choices(i)->value==*target)
                select=i;

        DisplayText(REAL(x-.02),y,title,selected,alpha,1);
        if (choices.Len()>0)
            DisplayText(REAL(x+.02),y,choices(select)->description,selected,alpha,-1);
    }

    virtual tString Help(){
        tString ret;
        ret << helpText;
        ret << "\n";
        ret << choices(select)->helpText;
        return ret;
    }

};

template<class T> class uSelectEntry{
public:
    uSelectEntry(uMenuItemSelection<T> &sel,
                 const char *desc,
                 const char *help,T val){
        sel.NewChoice(desc,help,val);
    }
};

// *****************************************
//               Toggle
// *****************************************

class uMenuItemToggle: public uMenuItemSelection<bool>{
    void NewChoice(uSelectItem<bool> *c);
    void NewChoice(const char *desc,bool val);
public:
#ifdef SLOPPYLOCALE
    uMenuItemToggle(uMenu *m,const char *tit,
                    const char *help,bool &targ);
#endif
    uMenuItemToggle(uMenu *m,const tOutput& title,
                    const tOutput& help,bool &targ);
    ~uMenuItemToggle();

    virtual void LeftRight(int);
    virtual void Enter();
};


// *****************************************
//               Integer Choose
// *****************************************

class uMenuItemInt:public uMenuItem{
protected:
    tOutput title;
    int &target;
    int Min,Max;
    int Step;
public:
    /*
      uMenuItemInt(uMenu *m,const char *tit,
      const char *help,int &targ,
      int mi,int ma,int step=1);
    */
    uMenuItemInt(uMenu *m,const tOutput &title,
                 const tOutput &help,int &targ,
                 int mi,int ma,int step=1);

    ~uMenuItemInt(){}

    virtual void LeftRight(int);

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0);
};

// *****************************************
//               Float Choose
// *****************************************

class uMenuItemReal:public uMenuItem{
protected:
    tOutput title;
    REAL &target;
    REAL Min,Max;
    REAL Step;
public:
    /*
      uMenuItemInt(uMenu *m,const char *tit,
      const char *help,int &targ,
      int mi,int ma,int step=1);
    */
    uMenuItemReal(uMenu *m,const tOutput &title,
                 const tOutput &help,REAL &targ,
                 REAL mi,REAL ma,REAL step=1);

    ~uMenuItemReal(){}

    virtual void LeftRight(int);

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0);
};

// *****************************************************
//  String query
// *****************************************************


class uMenuItemString: public uMenuItem{
protected:
    tOutput  description;
    tString *content;
    int      cursorPos;
    int		maxLength_;

    // color mode used for rendering
    rTextField::ColorMode colorMode_;
public:
    uMenuItemString(uMenu *M,const tOutput& desc,
                    const tOutput& help,tString &c, int maxLength = 1024 );
    virtual ~uMenuItemString(){}

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0);

    virtual bool Event(SDL_Event &e);

    uMenu *MyMenu(){return menu;}

    void SetColorMode( rTextField::ColorMode colorMode )
    {
        colorMode_ = colorMode;
    }

    rTextField::ColorMode GetColorMode() const
    {
        return colorMode_;
    }
};

//  does not cound for the existance of color codes within string
class uMenuItemColorLine: public uMenuItemString
{
    public:
        uMenuItemColorLine(uMenu *M, tString &c, int maxLength = 1024 ):
                        uMenuItemString(M,"$player_name_text","$player_name_help",c, maxLength) {}
        virtual ~uMenuItemColorLine(){}

};

class uMenuItemStringWithHistory : protected uMenuItemString {
protected:
    std::deque<tString> &m_History;
    unsigned int m_HistoryPos;
    unsigned int m_HistoryLimit;
public:
    uMenuItemStringWithHistory(uMenu *M, const tOutput& desc, const tOutput& help, tString &c, int maxLength, std::deque<tString> &history, int limit);

    static void SaveHistoryToFile(const tString& filename, std::deque<tString>& history);
    static void LoadHistoryFromFile(const tString& filename, std::deque<tString>& history);


    ~uMenuItemStringWithHistory();

    virtual void RenderBackground(){
        menu->GenericBackground(menu->GetTop());
    }

    virtual bool Event(SDL_Event &e);
};


// *****************************************************
//  Submenu
// *****************************************************


class uMenuItemSubmenu: public uMenuItem{
    uMenu *submenu;
public:
    uMenuItemSubmenu(uMenu *M,uMenu *s,
                     const tOutput& help);
    virtual ~uMenuItemSubmenu(){}

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0);

    virtual void Enter();
};


// *****************************************************
//  generic action
// *****************************************************

class uMenuItemAction: public uMenuItem{
public:
    uMenuItemAction(uMenu *M,const tOutput& name,
                    const tOutput& help );

    virtual ~uMenuItemAction(){}

    virtual void Render(REAL x,REAL y,REAL alpha=1,bool selected=0);

    virtual void Enter() = 0;
protected:
    tOutput  name_;
};

// *****************************************************
//  Execute function
// *****************************************************


class uMenuItemFunction: public uMenuItemAction{
    FUNCPTR  func;
public:
    uMenuItemFunction(uMenu *M,const tOutput& name,
                      const tOutput& help,FUNCPTR f);

    virtual ~uMenuItemFunction(){}

    virtual void Enter();
};


class uMenuItemFunctionInt: public uMenuItemAction{
    INTFUNCPTR  func;
    int         arg;
public:
    uMenuItemFunctionInt(uMenu *M,const tOutput& name,
                         const tOutput& help,INTFUNCPTR f,int arg);

    virtual ~uMenuItemFunctionInt(){}

    virtual void Enter();

    int getArg() { return arg; }
};

// *****************************************************
//  File Selection (added by k)
// *****************************************************

class uMenuItemFileSelection: public uMenuItemSelection<tString>
{
    void NewChoice( uSelectItem<bool> * );
    void NewChoice( char *, bool );
protected:
    tString dir_;
    tString fileSpec_;
    int     getFilesFlag_;
    bool    formatName_;
    tString defaultFileName_;
    tString defaultFilePath_;
public:
    uMenuItemFileSelection( uMenu *m, char *tit, const char *help, tString& targ,
                            const char *dir, const char *fileSpec, int getFilesFlag = 0, bool formatName = true )
            :uMenuItemSelection<tString>( m, tit, help, targ )
    {
        SetParams( dir, fileSpec, getFilesFlag, formatName, "", "" );
        Reload();
    }

    uMenuItemFileSelection( uMenu *m, char *tit, const char *help, tString& targ,
                            const char *dir, const char *fileSpec,
                            const char *defaultFileName, const char *defaultFilePath,
                            int getFilesFlag = 0, bool formatName = true )
            :uMenuItemSelection<tString>( m, tit, help, targ )
    {
        SetParams( dir, fileSpec, getFilesFlag, formatName, defaultFileName, defaultFilePath );
        Reload();
    }

    virtual ~uMenuItemFileSelection() {}

    void SetDir( const char *dir ) { dir_ = dir; }
    void SetFileSpec( const char *fileSpec ) { fileSpec_ = fileSpec; }
    void SetFormatName( bool formatName ) { formatName_ = formatName; }
    void SetGetFilesFlag( int getFilesFlag ) { getFilesFlag_ = getFilesFlag; }
    void SetDefaultFileName( const char *defaultFileName ) { defaultFileName_ = defaultFileName; }
    void SetDefaultFilePath( const char *defaultFilePath ) { defaultFilePath_ = defaultFilePath; }

    void SetParams( const char *dir, const char *fileSpec, int getFilesFlag,
                    bool formatName, const char *defaultFileName, const char *defaultFilePath )
    {
        SetDir( dir );
        SetFileSpec( fileSpec );
        SetGetFilesFlag( getFilesFlag );
        SetFormatName( formatName );
        SetDefaultFileName( defaultFileName );
        SetDefaultFilePath( defaultFilePath );
    }

    void Reload();

    void LoadDirectory( const char *dir, const char *fileSpec, bool formatName = true );

    void AddFile( const char *fileName, const char *filePath, bool formatName = true );
};

// *****************************************************
// Menu Enter/Leave-Callback
// *****************************************************

class uCallbackMenuEnter: public tCallback{
public:
    uCallbackMenuEnter(VOIDFUNC *f);
    static void MenuEnter();
};

class uCallbackMenuLeave: public tCallback{
public:
    uCallbackMenuLeave(VOIDFUNC *f);
    static void MenuLeave();
};

class uCallbackMenuBackground: public tCallback{
public:
    uCallbackMenuBackground(VOIDFUNC *f);
    static void MenuBackground();
};


inline void uMenu::AddItem(uMenuItem* item)     { items.Add(item, item->idnum); }
inline void uMenu::RemoveItem(uMenuItem* item)  { items.Remove(item, item->idnum); }


class uMenuItemToggleThemed : public uMenuItemToggle
{
public:
    uMenuItemToggleThemed(uMenu *menu,
                          const char *title,
                          const char *help,
                          bool &target,
                          tThemedText &theTheme,
                          REAL &valueLocX)
        : uMenuItemToggle(menu, "", help, target),
          customTitle(title),
          targetPtr(&target),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
        bool currentValue = *targetPtr;

        tString themedLabel = menutheme.HeaderColor() + customTitle;
        tString themedValue = currentValue
                                  ? (menutheme.MainColor() + tString("Enabled"))
                                  : (menutheme.ErrorColor() + tString("Disabled"));

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, themedValue.c_str(), selected, alpha, -1);
    }

private:
    tString customTitle;
    bool *targetPtr;
    tThemedText &menutheme;
    REAL &locX;
};

class uMenuItemIntThemed : public uMenuItemInt
{
public:
    uMenuItemIntThemed(uMenu *menu,
                       const char *title,
                       const char *help,
                       int &target,
                       int minVal,
                       int maxVal,
                       int stepVal,
                       tThemedText &theTheme,
                       REAL &valueLocX)
        : uMenuItemInt(menu, "", help, target, minVal, maxVal, stepVal),
          customTitle(title),
          targetPtr(&target),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
        tString themedLabel = menutheme.HeaderColor() + customTitle;

        tString valueStr;
        valueStr << *targetPtr;
        tString themedValue = menutheme.ItemColor() + valueStr;

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, themedValue.c_str(), selected, alpha, -1);
    }

private:
    tString customTitle;
    int *targetPtr;
    tThemedText &menutheme;
    REAL &locX;
};

class uMenuItemRealThemed : public uMenuItemReal
{
public:
    uMenuItemRealThemed(uMenu *menu,
                        const char *title,
                        const char *help,
                        REAL &target,
                        REAL minVal,
                        REAL maxVal,
                        REAL stepVal,
                        tThemedText &theTheme,
                        REAL &valueLocX)
        : uMenuItemReal(menu, "", help, target, minVal, maxVal, stepVal),
          customTitle(title),
          targetPtr(&target),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
        tString themedLabel = menutheme.HeaderColor() + customTitle;

        tString valueStr;
        valueStr << *targetPtr;
        tString themedValue = menutheme.ItemColor() + valueStr;

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, themedValue.c_str(), selected, alpha, -1);
    }

private:
    tString customTitle;
    REAL *targetPtr;
    tThemedText &menutheme;
    REAL &locX;
};

class uMenuItemStringThemed : public uMenuItemString
{
public:
    uMenuItemStringThemed(uMenu *menu,
                          const char *title,
                          const char *help,
                          tString &target,
                          tThemedText &theTheme,
                          REAL &valueLocX,
                          int maxLength = 1024)
        : uMenuItemString(menu, tString(""), help, target, maxLength),
          customTitle(title),
          menutheme(theTheme),
          locX(valueLocX)
    {
    }

    virtual void Render(REAL x, REAL y, REAL alpha = 1, bool selected = false) override
    {
    #ifndef DEDICATED
        static int counter = 0;
        counter++;

        int cmode = 0;
        if (selected)
        {
            cmode = 1;
            if (counter & 32)
                cmode = 2;
        }

        rTextField::ColorMode currentColorMode = colorMode_;
        if (currentColorMode == rTextField::COLOR_SHOW && !selected)
            currentColorMode = rTextField::COLOR_USE;

        tString themedLabel = menutheme.HeaderColor() + customTitle;

        DisplayText(x, y, themedLabel.c_str(), selected, alpha, 1);
        DisplayText(x + locX, y, &((*content)[0]), selected, alpha, -1,
                    cmode, cursorPos, currentColorMode);
    #endif
    }

private:
    tString customTitle;
    tThemedText &menutheme;
    REAL &locX;
};

#endif

