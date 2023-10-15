#ifndef ArmageTron_eChatCommands_H
#define ArmageTron_eChatCommands_H

#include "tString.h"
#include "tSysTime.h"

#include <unordered_map>
#include <functional>
#include "defs.h"

#include "ePlayer.h"

#include "eFloor.h"
#include "../tron/gCycle.h"

extern tString se_chatCommandsThemeHeader;
extern tString se_chatCommandsThemeMain;
extern tString se_chatCommandsThemeItem;
extern tString se_chatCommandsThemeError;

extern tString se_consoleCommand;
extern tString se_colorsCommand;
extern tString se_infoCommand;
extern tString se_rgbCommand;
extern tString se_colorVarFile;
extern tString se_browserCommand;
extern tString se_speakCommand;
extern tString se_rebuildCommand;
extern tString se_watchCommand;
extern tString se_activeStatusCommand;
extern tString se_reverseCommand;
extern tString se_spectateCommand;
extern tString se_joinCommand;
extern tString se_searchCommand;
extern tString se_nameSpeakCommand;
extern int se_nameSpeakCommandInterval;
extern tString se_respawnCommand;
extern tString se_rebuildGridCommand;
extern tString se_saveConfigCommand;
extern tString se_replyCommand;
extern tString se_nicknameCommand;
extern tString se_statsCommand;
extern tString se_reconnectCommand;
extern tString se_calculateCommand;
extern tString se_updateCommand;
extern tString se_encryptCommand;
extern bool se_encryptCommandWatch;
extern bool se_encryptCommandWatchFeedback;
extern int se_encryptCommandWatchValidateWindow;
extern int se_encryptCommandLength;
extern tString se_encryptCommandPrefix;
extern tString se_voteCommand;
extern tString se_renameCommand;
extern tString se_leaveCommand;
extern tString se_quitCommand;
extern int se_quitCommandTime;

class eChatCommand
{
public:
    virtual bool execute(tString args) = 0;

    bool execute(ePlayer *p, tString args)
    {
        player = p;

        if (!player)
            return false;

        netPlayer = se_GetLocalPlayer(player->ID());

        if (playerRequired() && netPlayer == nullptr)
        {
            con << CommandText() << "Player is required to run this command.\n";
            return false;
        }

        return execute(args);
    }

    static tString HeaderText() { return se_chatCommandsThemeHeader; }
    static tString MainText() { return se_chatCommandsThemeMain; }
    static tString ItemText() { return se_chatCommandsThemeItem; }
    static tString ErrorText() { return se_chatCommandsThemeError; }

    bool playerRequired()
    {
        return requirePlayer;
    }

    static tString CommandText(std::string cmd)
    {
        tString output;

        output << HeaderText()
               << cmd
               << " - "
               << MainText();
        return output;
    }

    tString CommandText()
    {
        return CommandText(commandName);
    }

    const std::string &getCommandName() const
    {
        return commandName;
    }

protected:
    eChatCommand(const std::string &name) : commandName(name) {}

protected:
    bool requirePlayer = true;

private:
    std::string commandName;

public:
    ePlayer *player;
    ePlayerNetID *netPlayer;
};

std::unordered_map<tString, std::function<std::unique_ptr<eChatCommand>()>> CommandFactory();
extern bool LocalChatCommands(ePlayer *player, tString args, const std::unordered_map<tString, std::function<std::unique_ptr<eChatCommand>()>> &commandFactories = CommandFactory());

extern void LoadChatCommandConfCommands();

class MsgCommand : public eChatCommand
{
public:
    MsgCommand() : eChatCommand("MsgCommand") {}
    bool execute(tString args) override;
};

class ConsoleCommand : public eChatCommand
{
public:
    ConsoleCommand() : eChatCommand("ConsoleCommand") {}

    bool execute(tString args) override;
};

// Gather all the rgb colors and put them in a nice list.
// Usage /colors with no parameters returns all players and their colors.
//       /colors playername returns that specific players color or more depending if the search term is found in other player names
class ColorsCommand : public eChatCommand
{
public:
    ColorsCommand() : eChatCommand("ColorsCommand") {}

    static tColoredString cycleColorPreview(REAL r, REAL g, REAL b);

    static tColoredString localPlayerPreview(ePlayer *local_p);

    static tColoredString gatherPlayerColor(ePlayerNetID *p, bool showReset = true);

static tString localPlayerMode(ePlayer *local_p);
    bool execute(tString args) override;
};

/*
List player information.
Displays:
Colored Name
Position
Direction
Used Rubber out of max
Speed
Spectating / Playing
Chatting
Sometimes Alive / Dead
Usage: /info - Returns own information
       /info playername - Returns that players name. (or more depending if the search word is found in more than one player)
*/
class listPlayerInfoCommand : public eChatCommand
{
public:
    listPlayerInfoCommand() : eChatCommand("listPlayerInfoCommand") 
    {
        requirePlayer = false;
    }
    tColoredString gatherPlayerInfo(ePlayerNetID *p);
    bool execute(tString args) override;
};

/* Allow us to change our current RGB easily.
Usage:
    /rgb with no parameters displays current rgb.
    /rgb 15 3 3 will set your RGB to R15 G3 B3.
    /rgb unique gives you unique colors.
    /rgb random gives you random colors.
    /rgb 2 command will run the passed command for player 2.
    /rgb save will save your current colors to file.
    /rgb save player will save the players current colors to var/\1.
    /rgb list will list your current saved colors.
    /rgb load 1 will load from line #1 in saved file. (As shown in /rgb list)
    /rgb clearall will clear the entire list of saved colors.
    /rgb clear 1 will clear your saved color #1 from the list of saved colors. */
class RgbCommand : public eChatCommand
{
public:
    RgbCommand() : eChatCommand("RgbCommand")
    {
        requirePlayer = false;
    }
    static std::tuple<tString, int, int, int> se_extractColorInfoFromLine(const tString &line);
    static void se_outputColorInfo(int index, const tString &Name, REAL r, REAL g, REAL b);
    bool execute(tString args) override;
};

class BrowserCommand : public eChatCommand
{
public:
    BrowserCommand() : eChatCommand("BrowserCommand") {}
    bool execute(tString args) override;
};

class SpeakCommand : public eChatCommand
{
public:
    SpeakCommand() : eChatCommand("SpeakCommand") {}
    bool execute(tString args) override;
};

class RebuildCommand : public eChatCommand
{
public:
    RebuildCommand() : eChatCommand("RebuildCommand") {}
    bool execute(tString args) override;
};

class WatchCommand : public eChatCommand
{
public:
    WatchCommand() : eChatCommand("WatchCommand") {}
    bool execute(tString args) override;
};

class ActiveStatusCommand : public eChatCommand
{
public:
    ActiveStatusCommand() : eChatCommand("ActiveStatusCommand") 
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class ReverseCommand : public eChatCommand
{
public:
    ReverseCommand() : eChatCommand("ReverseCommand") {}
    bool execute(tString args) override;
};

class SpectateCommand : public eChatCommand
{
public:
    SpectateCommand() : eChatCommand("SpectateCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class JoinCommand : public eChatCommand
{
public:
    JoinCommand() : eChatCommand("JoinCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

static std::vector<std::pair<tString, tString>> searchableFiles =
    {
        {tString("chat"), tString("chatlog.txt")},
        {tString("console"), tString("consolelog-limited.txt")},
        {tString("console-full"), tString("consolelog.txt")},
};
class SearchCommand : public eChatCommand
{
public:
    SearchCommand() : eChatCommand("SearchCommand") 
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class NameSpeakCommand : public eChatCommand
{
public:
    NameSpeakCommand() : eChatCommand("NameSpeakCommand") 
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class RespawnCommand : public eChatCommand
{
public:
    RespawnCommand() : eChatCommand("RespawnCommand") {}
    bool execute(tString args) override;
};

class RebuildGridCommand : public eChatCommand
{
public:
    RebuildGridCommand() : eChatCommand("RebuildGridCommand") {}
    bool execute(tString args) override;
};

class SaveConfigCommand : public eChatCommand
{
public:
    SaveConfigCommand() : eChatCommand("SaveConfigCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class ReplyCommand : public eChatCommand
{
public:
    ReplyCommand() : eChatCommand("ReplyCommand") {}
    bool execute(tString args) override;
};

class NicknameCommand : public eChatCommand
{
public:
    NicknameCommand() : eChatCommand("NicknameCommand") {}
    bool execute(tString args) override;
};

class StatsCommand : public eChatCommand
{
public:
    StatsCommand() : eChatCommand("StatsCommand") {}
    bool execute(tString args) override;
};

class ReconnectCommand : public eChatCommand
{
public:
    ReconnectCommand() : eChatCommand("ReconnectCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

#include <queue>
class CalculateCommand : public eChatCommand
{
public:
    CalculateCommand() : eChatCommand("CalculateCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;

    static tString preprocess(const tString &input);

    static std::queue<tString> infixToPostfix(const tString &infix);
};

class UpdateCommand : public eChatCommand
{
public:
    UpdateCommand() : eChatCommand("UpdateCommand") {}
    bool execute(tString args) override;
};

class EncryptCommand : public eChatCommand
{
public:
    EncryptCommand() : eChatCommand("EncryptCommand") {}
    static REAL getEncryptLocaltime();
    static tString GenerateHash(double time);
    static bool ValidateHash(tString givenHash, double time);
    static bool handleEncryptCommandAction(ePlayerNetID *player, tString message);
    bool execute(tString args) override;
};

class VoteCommand : public eChatCommand
{
public:
    VoteCommand() : eChatCommand("VoteCommand") {}
    bool execute(tString args) override;

private:
    void displayPollsMenu();
    void sendChatMessage(ePlayerNetID *player, tString &message);
    void processVote(const tArray<tString> &params);
};

class RenameCommand : public eChatCommand
{
public:
    RenameCommand() : eChatCommand("RenameCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class LeaveCommand : public eChatCommand
{
public:
    LeaveCommand() : eChatCommand("LeaveCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class QuitCommand : public eChatCommand
{
public:
    QuitCommand() : eChatCommand("QuitCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

class BookmarksCommand : public eChatCommand
{
public:
    BookmarksCommand() : eChatCommand("BookmarksCommand")
    {
        requirePlayer = false;
    }
    bool execute(tString args) override;
};

#endif
