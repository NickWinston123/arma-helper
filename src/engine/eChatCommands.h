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
#include "../tron/gHelper/gHelperUtilities.h"

extern tString se_chatCommandsThemeHeader;
extern tString se_chatCommandsThemeMain;
extern tString se_chatCommandsThemeItem;
extern tString se_chatCommandsThemeError;

extern tString se_nameSpeakCommand;
extern int se_nameSpeakCommandInterval;
extern bool se_encryptCommandWatch;


class eChatCommand
{
public:
    bool playerRequired()
    {
        return requirePlayer;
    }

    virtual bool execute(tString args) = 0;

    bool execute(ePlayer *p, tString args)
    {
        player = p;

        if (!player)
            return false;

        netPlayer = player->netPlayer;

        if (playerRequired() && netPlayer == nullptr)
        {
            con << CommandLabel() << "Player is required to run this command.\n";
            return false;
        }

        return execute(args);
    }

    tString HeaderColor() { return theme.HeaderColor(); }
    tString MainColor() { return theme.MainColor(); }
    tString ItemColor() { return theme.ItemColor(); }
    tString ErrorColor() { return theme.ErrorColor(); }

    tString CommandLabel()
    {
        return theme.LabelText(commandName);
    }

    const std::string &getCommandName() const
    {
        return commandName;
    }

protected:
    eChatCommand(const std::string &name) : commandName(name) {}
    bool requirePlayer = true;

private:
    std::string commandName;

public:
    ePlayer *player;
    ePlayerNetID *netPlayer;
    static tThemedText theme;
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

    static tColoredString cycleColorPreview(REAL r, REAL g, REAL b, bool resetColor = true);
    static tColoredString cycleColorPreview(tColor rgb);

    static tColoredString localPlayerPreview(ePlayer *local_p);

    static tColoredString gatherPlayerColor(ePlayerNetID *p, bool showReset = true);
    static tColoredString gatherPlayerColor(tString playerName, tColor rgb, tString mode = tString(""), bool showReset = true);

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

class SpeakBotCommand : public eChatCommand
{
public:
    SpeakBotCommand() : eChatCommand("SpeakBotCommand") {}
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

    static tString process(tString args, bool showErrors = true);
    static tString preprocess(const tString &input);

    static std::queue<tString> infixToPostfix(const tString &infix);

    static bool isNumericToken(const std::string& token);
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
