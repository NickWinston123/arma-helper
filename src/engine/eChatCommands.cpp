#include "eChatCommands.h"

#include "ePlayer.h"
#include "ePlayerStats.h"
#include "eChatBot.h"
#include "eTimer.h"
#include "eFloor.h"
#include "rConsole.h"
#include "tRecorder.h"
#include "nServerInfo.h"
#include "../tron/gGame.h"
#include "../tron/gCycle.h"
#include "../tron/gMenus.h"
#include "../tron/gServerBrowser.h"
#include "../tron/gServerFavorites.h"
#include "../tron/gHelper/gHelperUtilities.h"


// Bright Red for headers
tString se_chatCommandsThemeHeader("0xff0033");
static tConfItem<tString> se_chatCommandsThemeHeaderConf("LOCAL_CHAT_COMMANDS_THEME_HEADER", se_chatCommandsThemeHeader);
// White for main
tString se_chatCommandsThemeMain("0xffffff");
static tConfItem<tString> se_chatCommandsThemeMainConf("LOCAL_CHAT_COMMANDS_THEME_MAIN", se_chatCommandsThemeMain);
// Dark Red for items
tString se_chatCommandsThemeItem("0xee0000");
static tConfItem<tString> se_chatCommandsThemeItemConf("LOCAL_CHAT_COMMANDS_THEME_ITEM", se_chatCommandsThemeItem);
// Pinkish red for error messages as an accent color
tString se_chatCommandsThemeError("0xee5577");
static tConfItem<tString> se_chatCommandsThemeErrorConf("LOCAL_CHAT_COMMANDS_THEME_ERROR", se_chatCommandsThemeError);

tThemedText eChatCommand::theme(se_chatCommandsThemeHeader,se_chatCommandsThemeMain,se_chatCommandsThemeItem,se_chatCommandsThemeError);

// our local commands (should always be lowercase)
tString se_consoleCommand("/con");
static tConfItem<tString> se_consoleCommandConf("LOCAL_CHAT_COMMAND_CONSOLE", se_consoleCommand);

tString se_colorsCommand("/colors");
static tConfItem<tString> se_colorsCommandConf("LOCAL_CHAT_COMMAND_COLORS", se_colorsCommand);

tString se_infoCommand("/info");
static tConfItem<tString> se_infoCommandConf("LOCAL_CHAT_COMMAND_INFO", se_infoCommand);

tString se_rgbCommand("/rgb");
static tConfItem<tString> se_rgbCommandConf("LOCAL_CHAT_COMMAND_RGB", se_rgbCommand);
tString se_colorVarFile("colors.txt");
static tConfItem<tString> se_colorVarFileConf("LOCAL_CHAT_COMMAND_RGB_FILE", se_colorVarFile);

tString se_browserCommand("/browser");
static tConfItem<tString> se_browserCommandConf("LOCAL_CHAT_COMMAND_BROWSER", se_browserCommand);

tString se_speakCommand("/speak");
static tConfItem<tString> se_speakCommandConf("LOCAL_CHAT_COMMAND_SPEAK", se_speakCommand);
static bool se_speakCommandChatFlag = false;
static tConfItem<bool> se_speakCommandChatFlagConf("LOCAL_CHAT_COMMAND_SPEAK_CHAT_FLAG", se_speakCommandChatFlag);
static REAL se_speakCommandDelay = 0;
static tConfItem<REAL> se_speakCommandDelayConf("LOCAL_CHAT_COMMAND_SPEAK_DELAY", se_speakCommandDelay);

tString se_rebuildCommand("/rebuild");
static tConfItem<tString> se_rebuildCommandConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_REBUILD", se_rebuildCommand);

tString se_watchCommand("/watch");
static tConfItem<tString> se_watchCommandConf("LOCAL_CHAT_COMMAND_WATCH", se_watchCommand);

tString se_activeStatusCommand("/afk");
static tConfItem<tString> se_activeStatusCommandConf("LOCAL_CHAT_COMMAND_ACTIVE_STATUS", se_activeStatusCommand);

tString se_reverseCommand("/rev");
static tConfItem<tString> se_reverseCommandConf("LOCAL_CHAT_COMMAND_REVERSE", se_reverseCommand);

tString se_spectateCommand("/spec");
static tConfItem<tString> se_spectateCommandConf("LOCAL_CHAT_COMMAND_SPECTATE", se_spectateCommand);
bool se_spectateCommandInstant = false;
static tConfItem<bool> se_spectateCommandInstantConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_SPECTATE_INSTANT", se_spectateCommandInstant);

tString se_joinCommand("/join");
static tConfItem<tString> se_joinCommandConf("LOCAL_CHAT_COMMAND_JOIN", se_joinCommand);

tString se_searchCommand("/search");
static tConfItem<tString> se_searchCommandConf("LOCAL_CHAT_COMMAND_SEARCH", se_searchCommand);
static REAL se_searchCommandMaxFileSizeMB = 50; // 50 MB
static tConfItem<REAL> se_searchCommandMaxFileSizeMBConf("LOCAL_CHAT_COMMAND_SEARCH_MAX_FILE_SIZE", se_searchCommandMaxFileSizeMB);
static bool se_searchCommandCaseSensitive = false;
static tConfItem<bool> se_searchCommandCaseSensitiveConf("LOCAL_CHAT_COMMAND_CASE_SENSITIVE", se_searchCommandCaseSensitive);
static int se_searchCommandEmptySearchNumLines = 20;
static tConfItem<int> se_searchCommandEmptySearchNumLinesConf("LOCAL_CHAT_COMMAND_EMPTY_SEARCH_NUM_LINES", se_searchCommandEmptySearchNumLines);

tString se_nameSpeakCommand("/namespeak");
static tConfItem<tString> se_nameSpeakCommandConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_NAMESPEAK", se_nameSpeakCommand);
int se_nameSpeakCommandInterval = 1;
static tConfItem<int> se_nameSpeakCommandIntervalConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_NAMESPEAK_INTERVAL", se_nameSpeakCommandInterval);
static bool se_nameSpeakCommandSplitByNameSize = false;
static tConfItem<bool> se_nameSpeakCommandSplitByNameSizeConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_NAMESPEAK_SPLIT_BY_NAME_SIZE", se_nameSpeakCommandSplitByNameSize);

tString se_respawnCommand("/res");
static tConfItem<tString> se_respawnCommandConf("LOCAL_CHAT_COMMAND_RESPAWN", se_respawnCommand);

tString se_rebuildGridCommand("/regrid");
static tConfItem<tString> se_rebuildGridCommandConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_REBUILD_GRID", se_rebuildGridCommand);

tString se_saveConfigCommand("/savecfg");
static tConfItem<tString> se_saveConfigCommandConf("LOCAL_CHAT_COMMAND_SAVE_CONFIG", se_saveConfigCommand);

tString se_replyCommand("/r");
static tConfItem<tString> se_replyCommandConf("LOCAL_CHAT_COMMAND_REPLY", se_replyCommand);

tString se_nicknameCommand("/nickname");
static tConfItem<tString> se_nicknameCommandConf("LOCAL_CHAT_COMMAND_NICKNAME", se_nicknameCommand);

tString se_statsCommand("/stats");
static tConfItem<tString> se_statsCommandConf("LOCAL_CHAT_COMMAND_STATS", se_statsCommand);

tString se_reconnectCommand("/reconnect");
static tConfItem<tString> se_reconnectCommandConf("LOCAL_CHAT_COMMAND_RECONNECT", se_reconnectCommand);

tString se_calculateCommand("/calc");
static tConfItem<tString> se_calculateCommandConf("LOCAL_CHAT_COMMAND_CALCULATE", se_calculateCommand);

tString se_updateCommand("/update");
static tConfItem<tString> se_updateCommandConf("LOCAL_CHAT_COMMAND_UPDATE", se_updateCommand);

tString se_encryptCommand("/enc");
static tConfItem<tString> se_encryptCommandConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_ENCRYPT", se_encryptCommand);
bool se_encryptCommandWatch = false;
static tConfItem<bool> se_encryptCommandWatchConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_ENCRYPT_WATCH", se_encryptCommandWatch);
bool se_encryptCommandWatchFeedback = true;
static tConfItem<bool> se_encryptCommandWatchFeedbackConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_ENCRYPT_WATCH_FEEDBACK", se_encryptCommandWatchFeedback);
int se_encryptCommandWatchValidateWindow = 1;
static tConfItem<int> se_encryptCommandWatchValidateWindowConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_ENCRYPT_VALIDATE_WINDOW", se_encryptCommandWatchValidateWindow);
int se_encryptCommandLength = 15;
static tConfItem<int> se_encryptCommandLengthConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_ENCRYPT_LENGTH", se_encryptCommandLength);
tString se_encryptCommandPrefix = tString("@$#");
static tConfItem<tString> se_encryptCommandPrefixConf = HelperCommand::tConfItem("LOCAL_CHAT_COMMAND_ENCRYPT_PREFIX", se_encryptCommandPrefix);

tString se_voteCommand("/vote");
static tConfItem<tString> se_voteCommandConf("LOCAL_CHAT_COMMAND_VOTE", se_voteCommand);

tString se_renameCommand("/rename");
static tConfItem<tString> se_renameCommandConf("LOCAL_CHAT_COMMAND_RENAME", se_renameCommand);

tString se_leaveCommand("/leave");
static tConfItem<tString> se_leaveCommandConf("LOCAL_CHAT_COMMAND_LEAVE", se_leaveCommand);

tString se_quitCommand("/quit");
static tConfItem<tString> se_quitCommandConf("LOCAL_CHAT_COMMAND_QUIT", se_quitCommand);
int se_quitCommandTime = 3;
static tConfItem<int> se_quitCommandTimeConf("LOCAL_CHAT_COMMAND_QUIT_TIME", se_quitCommandTime);

tString se_bookmarksCommand("/bookmarks");
static tConfItem<tString> se_bookmarksCommandConf("LOCAL_CHAT_COMMAND_BOOKMARKS", se_bookmarksCommand);

static void TempConfItemCommandRunner(std::istream &input)
{
    tString args, inputStr;
    inputStr.ReadLine(input);

    ePlayer *player = sn_consoleUser();

    args << tConfItemBase::lastLoadCommandName
         << " "
         << inputStr;

    LocalChatCommands(player, args);
}

static TempConfItemManager *chatCommandConfItems = NULL;
void LoadChatCommandConfCommands()
{
    if (chatCommandConfItems == nullptr)
        chatCommandConfItems = new TempConfItemManager();

    if (!chatCommandConfItems)
        return;

    chatCommandConfItems->DeleteConfitems();

    for (auto chatcmdFactory : CommandFactory())
    {
        tString command;
        command << chatcmdFactory.first;
        tToUpper(command);
        chatCommandConfItems->StoreConfitem(new tConfItemFunc(command, &TempConfItemCommandRunner));
    }
}

std::unordered_map<tString, std::function<std::unique_ptr<eChatCommand>()>> CommandFactory()
{
    std::unordered_map<tString, std::function<std::unique_ptr<eChatCommand>()>> commandFactories;

    auto addCommand = [&commandFactories](const tString &commandName, auto commandFunc)
    {
        commandFactories.emplace(commandName.ToLower(), commandFunc);
    };

    commandFactories.emplace("/msg", []()
                             { return std::make_unique<MsgCommand>(); });
    addCommand(se_consoleCommand, []()
               { return std::make_unique<ConsoleCommand>(); });
    addCommand(se_colorsCommand, []()
               { return std::make_unique<ColorsCommand>(); });
    addCommand(se_infoCommand, []()
               { return std::make_unique<listPlayerInfoCommand>(); });
    addCommand(se_rgbCommand, []()
               { return std::make_unique<RgbCommand>(); });
    addCommand(se_browserCommand, []()
               { return std::make_unique<BrowserCommand>(); });
    addCommand(se_speakCommand, []()
               { return std::make_unique<SpeakCommand>(); });
    if (helperConfig::sghuk)
        addCommand(se_rebuildCommand, []()
               { return std::make_unique<RebuildCommand>(); });
    addCommand(se_watchCommand, []()
               { return std::make_unique<WatchCommand>(); });
    addCommand(se_activeStatusCommand, []()
               { return std::make_unique<ActiveStatusCommand>(); });
    addCommand(se_reverseCommand, []()
               { return std::make_unique<ReverseCommand>(); });
    addCommand(se_spectateCommand, []()
               { return std::make_unique<SpectateCommand>(); });
    addCommand(se_joinCommand, []()
               { return std::make_unique<JoinCommand>(); });
    addCommand(se_searchCommand, []()
               { return std::make_unique<SearchCommand>(); });
    if (helperConfig::sghuk)
        addCommand(se_nameSpeakCommand, []()
               { return std::make_unique<NameSpeakCommand>(); });
    addCommand(se_respawnCommand, []()
               { return std::make_unique<RespawnCommand>(); });
    if (helperConfig::sghuk)
        addCommand(se_rebuildGridCommand, []()
                { return std::make_unique<RebuildGridCommand>(); });
    addCommand(se_saveConfigCommand, []()
               { return std::make_unique<SaveConfigCommand>(); });
    addCommand(se_replyCommand, []()
               { return std::make_unique<ReplyCommand>(); });
    addCommand(se_nicknameCommand, []()
               { return std::make_unique<NicknameCommand>(); });
    addCommand(se_statsCommand, []()
               { return std::make_unique<StatsCommand>(); });
    addCommand(se_reconnectCommand, []()
               { return std::make_unique<ReconnectCommand>(); });
    addCommand(se_calculateCommand, []()
               { return std::make_unique<CalculateCommand>(); });
    addCommand(se_updateCommand, []()
               { return std::make_unique<UpdateCommand>(); });
    if (helperConfig::sghuk)
        addCommand(se_encryptCommand, []()
               { return std::make_unique<EncryptCommand>(); });
    addCommand(se_voteCommand, []()
               { return std::make_unique<VoteCommand>(); });
    addCommand(se_renameCommand, []()
               { return std::make_unique<RenameCommand>(); });
    addCommand(se_leaveCommand, []()
               { return std::make_unique<LeaveCommand>(); });
    addCommand(se_quitCommand, []()
               { return std::make_unique<QuitCommand>(); });
    addCommand(se_bookmarksCommand, []()
               { return std::make_unique<BookmarksCommand>(); });

    return commandFactories;
}

bool LocalChatCommands(ePlayer *player, tString args, const std::unordered_map<tString, std::function<std::unique_ptr<eChatCommand>()>> &commandFactories)
{

    if (player == nullptr)
        return false;

    // Trim whitespace from the command
    args = args.TrimWhitespace();

    // Split command into commandName and arguments
    int spaceIndex = args.StrPos(" ");
    spaceIndex = (spaceIndex == -1) ? args.Len() : spaceIndex;

    tString commandName = args.SubStr(0, spaceIndex);
    tString arguments = args.SubStr(spaceIndex + 1);
    tString commandNameUpper(commandName.ToUpper());

    // Find and execute the chat command
    auto chatcmdFactory = commandFactories.find(commandName.ToLower());
    if (chatcmdFactory != commandFactories.end())
    {
        auto chatcmd = chatcmdFactory->second();

        try
        {
            chatcmd->execute(player, arguments);
        }
        catch (const std::exception &e)
        {
            con << "There was an error executing the command: " << commandName << ": "
                << e.what()
                << "\n";
            return false;
        }

        return true;
    }

    if (SymLinkedCommandRunnertStr(commandNameUpper))
        return true;

    return false;
}

bool MsgCommand::execute(tString args)
{
    int pos = 0;
    tString name = args.ExtractNonBlankSubString(pos);
    ePlayerNetID *msgTarget = ePlayerNetID::GetPlayerByName(name, false);

    if (msgTarget)
    {
        netPlayer->lastMessagedByPlayer = nullptr;
        netPlayer->lastMessagedPlayer = msgTarget;
    }

    tString messageToSend;
    messageToSend << "/msg " << args;

    if (!ePlayerNetID::canChatWithMsg(messageToSend))
        return true;

    if (se_chatLog)
    {
        tString logOutput;

        if (se_chatTimeStamp)
            logOutput << st_GetCurrentTime("%H:%M:%S| ");

        int spacePos = args.StrPos(" ");

        if (spacePos != -1)
        {
            spacePos++;
            args.RemoveSubStr(0, spacePos);
        }

        logOutput << netPlayer->GetName()
                  << " --> "
                  << (msgTarget ? msgTarget->GetName() : "?")
                  << ": "
                  << args;

        se_SaveToChatLog(logOutput);
    }

    se_NewChatMessage(netPlayer, messageToSend)->BroadCast();
    return true;
}

bool ConsoleCommand::execute(tString args)
{
    tCurrentAccessLevel level(tAccessLevel_Owner, true);

    if (args.empty())
        return false;

    if (tRecorder::IsPlayingBack())
    {
        tConfItemBase::LoadPlayback();
    }
    else
    {
        std::stringstream s(static_cast<char const *>(args));
        tConfItemBase::LoadAll(s);
    }
    return true;
}

tColoredString ColorsCommand::cycleColorPreview(tColor rgb)
{
    return cycleColorPreview(rgb.r_, rgb.g_, rgb.b_);
}

tColoredString ColorsCommand::cycleColorPreview(REAL r, REAL g, REAL b, bool resetColor)
{
    r /= 15.0;
    g /= 15.0;
    b /= 15.0;

    REAL cycleR = r, cycleG = g, cycleB = b;

    se_MakeColorValid(r, g, b, 1.0f);

    while( cycleR > 1.f ) cycleR -= 1.f;
    while( cycleG > 1.f ) cycleG -= 1.f;
    while( cycleB > 1.f ) cycleB -= 1.f;

    // REAL tailR2 = cycleR, tailG2 = cycleG, tailB2 = cycleB;
    // while( tailR2 > 1.f ) tailR2 -= 1.f;
    // while( tailG2 > 1.f ) tailG2 -= 1.f;
    // while( tailB2 > 1.f ) tailB2 -= 1.f;

    tColoredString cyclePreview;
    cyclePreview << tColoredString::ColorString(cycleR, cycleG, cycleB) << "<"
                 << tColoredString::ColorString(r, g, b) << "==";
    if (resetColor)
        cyclePreview << tThemedTextBase.MainColor();

                //  << tColoredString::ColorString(tailR2, tailG2, tailB2) << "="

    return cyclePreview;
}

tString ColorsCommand::localPlayerMode(ePlayer *local_p)
{
    tString mode;
    switch (local_p->colorMode)
    {
    case PlayerColorMode::OFF:
        mode = "off";
        break;
    case PlayerColorMode::RANDOM:
        mode = "random";
        break;
    case PlayerColorMode::UNIQUE:
        mode = "unique";
        break;
    case PlayerColorMode::RAINBOW:
        mode = "rainbow";
        break;
    case PlayerColorMode::CROSSFADE:
        mode = "crossfade";
        break;
    default:
        mode = "?";
        break;
    }
    return mode;
}

tColoredString ColorsCommand::localPlayerPreview(ePlayer *local_p)
{
    int r = local_p->rgb[0];
    int g = local_p->rgb[1];
    int b = local_p->rgb[2];


    tColoredString output;
    output << tColoredString::ColorString(r/15, g/15, b/15)
           << local_p->Name()
           << tThemedTextBase.MainColor()
           << " ("
           << tThemedTextBase.ItemColor() << r << tThemedTextBase.MainColor() << ", "
           << tThemedTextBase.ItemColor() << g << tThemedTextBase.MainColor() << ", "
           << tThemedTextBase.ItemColor() << b << tThemedTextBase.MainColor() << ") "
           << cycleColorPreview(r, g, b)
           << " (mode: "
           << tThemedTextBase.ItemColor() << localPlayerMode(local_p) << tThemedTextBase.MainColor() << ") ";

    return output;
}

tColoredString ColorsCommand::gatherPlayerColor(tString playerName, tColor rgb, tString mode, bool showReset)
{
    tColoredString listColors, cyclePreview;

    if (showReset)
        listColors << playerName << tThemedTextBase.MainColor() << " (";
    else
        listColors << playerName.StripWhitespace() << tThemedTextBase.MainColor() << " (";

    listColors << tThemedTextBase.ItemColor() << rgb.r_ << tThemedTextBase.MainColor() << ", "
               << tThemedTextBase.ItemColor() << rgb.g_ << tThemedTextBase.MainColor() << ", "
               << tThemedTextBase.ItemColor() << rgb.b_ << tThemedTextBase.MainColor() << ") "
               << cycleColorPreview(rgb);

    if (!mode.empty())
    {
        listColors << " (mode: "
                   << tThemedTextBase.ItemColor()
                   << mode
                   << tThemedTextBase.MainColor()
                   << ") ";
}

    return listColors;
}

tColoredString ColorsCommand::gatherPlayerColor(ePlayerNetID *p, bool showReset)
{
    tString mode;
    if (p->isLocal())
    {
        ePlayer *local_p = ePlayer::NetToLocalPlayer(p);
        mode = localPlayerMode(local_p);
    }

    return gatherPlayerColor(p->GetColoredName(), tColor(p->r,p->g,p->b), mode, showReset);
}

bool ColorsCommand::execute(tString args)
{
    if (se_PlayerNetIDs.Len() > 0)
    {
        if (args.empty())
        {
            con << CommandLabel() << tOutput("$player_colors_text") << MainColor();
            for (int i = 0; i <= se_PlayerNetIDs.Len() - 1; i++)
                con << ItemColor() << (i + 1) << MainColor() << ") " << gatherPlayerColor(se_PlayerNetIDs(i)) << "\n";
        }
        else
        {
            bool playerFound = false;
            tArray<tString> searchWords = args.Split(" ");

            con << CommandLabel() << tOutput("$player_colors_text") << MainColor();

            int j = 0;
            for (int i = 0; i < searchWords.Len(); i++)
            {
                ePlayerNetID *p = ePlayerNetID::GetPlayerByName(searchWords[i], false);
                if (p)
                {
                    playerFound = true;
                    j++;
                    con << ItemColor() << j << MainColor() << ") " << gatherPlayerColor(p) << "\n";
                }
            }
            // No one found.
            if (!playerFound)
                con << ErrorColor() << tOutput("$player_colors_not_found", searchWords[1]) << MainColor();
        }
    }

    return true;
}

bool listPlayerInfoCommand::execute(tString args)
{
    if (se_PlayerNetIDs.Len() > 0)
    {
        if (args.empty())
        {
            if (netPlayer)
                con << CommandLabel()
                    << tOutput("$player_info_text")
                    << "\n"
                    << gatherPlayerInfo(netPlayer);
            else
                con << CommandLabel()
                    << ErrorColor()
                    << "net player does not exist\n";
        }
        else
        {
            bool playerFound = false;
            tArray<ePlayerNetID *> foundPlayers;
            tArray<tString> msgsExt = args.Split(" ");

            con << CommandLabel()
                << tOutput("$player_info_text");

            int j = 0;
            for (int i = 0; i < msgsExt.Len(); i++)
            {
                tString word = msgsExt[i];
                ePlayerNetID *p = ePlayerNetID::GetPlayerByName(word, false);
                if (p)
                {
                    playerFound = true;
                    j++;
                    con << ItemColor() << j << MainColor() << ") " << gatherPlayerInfo(p) << "\n";
                }
            }

            if (!playerFound)
                con << CommandLabel()
                    << ErrorColor()
                    << tOutput("$player_not_found_text", args);
        }
    }
    return true;
}

tColoredString listPlayerInfoCommand::gatherPlayerInfo(ePlayerNetID *p)
{


    tColoredString listInfo;
    listInfo << tThemedTextBase.MainColor() << "Results for " << p->GetColoredName() << tThemedTextBase.MainColor() << ":\n";

    // Player Info
    listInfo << tThemedTextBase.MainColor() << "Player Info: \n";

    tString nameHistory;

    for (int i = 0; i < p->nameHistory.Len(); i++)
    {
        if (nameHistory.empty())
            nameHistory = p->nameHistory[i];
        else {
            nameHistory = nameHistory + tThemedTextBase.MainColor() + ", " + p->nameHistory[i];
        }
    }

    listInfo << tThemedTextBase.MainColor() << " Color: " << ColorsCommand::gatherPlayerColor(p) << "\n";

    gRealColor color(p->r, p->g, p->b);
    se_MakeColorValid(color.r, color.g, color.b, 1.0f);
    se_removeDarkColors(color);
    listInfo << tThemedTextBase.MainColor()
             << " Filtered Color: "      << tThemedTextBase.MainColor()            << "("
             << tThemedTextBase.ItemColor() << color.r << tThemedTextBase.MainColor() << ", "
             << tThemedTextBase.ItemColor() << color.g << tThemedTextBase.MainColor() << ", "
             << tThemedTextBase.ItemColor() << color.b << tThemedTextBase.MainColor() << ")\n";

    listInfo << " Name History: "
             << nameHistory
             << "\n";

    listInfo << " Status: "
             << tThemedTextBase.ItemColor() << (p->IsHuman() ? "Human" : "Bot")
             << tThemedTextBase.MainColor() << ", "
             << tThemedTextBase.ItemColor() << (p->CurrentTeam() ? "Playing" : "Spectating")
             << (p->IsChatting() ? (tThemedTextBase.MainColor() << "," << tThemedTextBase.ItemColor() << " Chatting\n") : "\n");

    listInfo << " Ping: "
             << tThemedTextBase.ItemColor()
             << int(p->ping * 1000)
             << tThemedTextBase.MainColor()
             << " ("
             << tThemedTextBase.ItemColor()
             << p->ping
             << tThemedTextBase.MainColor()
             << ")\n";

    listInfo << " Created: "
             << tThemedTextBase.ItemColor() << getTimeStringBase(p->createTime_)
             << tThemedTextBase.MainColor() << "\n"
             << " Last Activity: "
             << tThemedTextBase.ItemColor() << st_GetFormatTime(p->LastActivity(), true)
             << tThemedTextBase.MainColor() << "\n";

    if (p->ChattingTime() > 0)
        listInfo << " Chatting For: "
                 << tThemedTextBase.ItemColor()
                 << st_GetFormatTime(p->ChattingTime(), true)
                 << tThemedTextBase.MainColor() << "\n";


    // Cycle Info

    if (p->CurrentTeam())
        listInfo << tThemedTextBase.MainColor()
                << "Cycle Info: \n";
    if (p->Object())
    {
        // Only grab this information if the player is an alive object.
        gCycle *pCycle = dynamic_cast<gCycle *>(p->Object());

        listInfo << " Status: "
                 << tThemedTextBase.ItemColor()
                 << (pCycle->Alive() ? "Alive" : "Dead")
                 << '\n'
                 << tThemedTextBase.MainColor()
                 << " Lag: "
                 << tThemedTextBase.ItemColor()
                 << int(pCycle->Lag()  * 1000)
                 << tThemedTextBase.MainColor()
                 << " ("
                 << tThemedTextBase.ItemColor()
                 << pCycle->Lag()
                 << tThemedTextBase.MainColor()
                 << ")\n";

        if (!pCycle->Alive() && pCycle->lastDeathTime > 0)
            listInfo << " Last Death: "
                     << ItemColor() << st_GetFormatTime(tSysTimeFloat() - pCycle->lastDeathTime, true)
                     << "\n";
        else
            listInfo << " Alive Time: "
                     << tThemedTextBase.ItemColor() << st_GetFormatTime(se_GameTime(), true)
                     << "\n";

        if (pCycle->Alive())
        {
            listInfo << tThemedTextBase.MainColor()
                     << " Position: x: "
                     << tThemedTextBase.ItemColor() << pCycle->Position().x
                     << tThemedTextBase.MainColor()
                     << ", y: "
                     << tThemedTextBase.ItemColor() << pCycle->Position().y
                     << tThemedTextBase.MainColor() << "\n"
                     << " Map Direction: x: "
                     << tThemedTextBase.ItemColor() << pCycle->Direction().x
                     << tThemedTextBase.MainColor()
                     << ", y: "
                     << tThemedTextBase.ItemColor() << pCycle->Direction().y
                     << tThemedTextBase.MainColor() << '\n'
                     << " Speed: "
                     << tThemedTextBase.ItemColor() << pCycle->verletSpeed_
                     << tThemedTextBase.MainColor() << '\n'
                     << " Rubber: "
                     << tThemedTextBase.ItemColor() << pCycle->GetRubber() << "/" << sg_rubberCycle
                     << tThemedTextBase.MainColor() << '\n';
        }
    }
    else if (p->CurrentTeam())
    {
        listInfo << " Status: "
                 << tThemedTextBase.ItemColor() << "Dead\n";

        listInfo << " Last Death: "
                    << ItemColor() << st_GetFormatTime(tSysTimeFloat() - p->lastCycleDeathTime, true)
                    << "\n";
    }

    return listInfo;
}

std::tuple<tString, int, int, int> RgbCommand::se_extractColorInfoFromLine(const tString &line)
{
    int pos = 0;

    tString name(line.ExtractNonBlankSubString(pos));
    tString color(line.ExtractNonBlankSubString(pos));
    color.RemoveSubStr(0, 1);

    int r = atoi(color);
    int g = atoi(line.ExtractNonBlankSubString(pos));
    int b = atoi(line.ExtractNonBlankSubString(pos));
    return std::make_tuple(name, r, g, b);
}

void RgbCommand::se_outputColorInfo(int index, const tString &name, REAL r, REAL g, REAL b)
{
    tColoredString output;

    output << (index + 1) << ") "
           << tColoredString::ColorString(r/15, g/15, b/15)
           << name << tThemedTextBase.MainColor()
           << " ("
           << tThemedTextBase.ItemColor() << r << tThemedTextBase.MainColor() << ", "
           << tThemedTextBase.ItemColor() << g << tThemedTextBase.MainColor() << ", "
           << tThemedTextBase.ItemColor() << b << tThemedTextBase.MainColor() << ") "
           << ColorsCommand::cycleColorPreview(r, g, b) << "\n";

    con << output;
}

bool RgbCommand::execute(tString args)
{
    ePlayer *local_p = player;
    ePlayerNetID *targetPlayer = netPlayer;
    tColor oldColor;
    tString oldMode;

    if (!local_p)
        return false;
    
    if (args.empty())
    {
        con << CommandLabel()
            << tOutput("$player_colors_current_text");
            
        for (auto localNetPlayer : se_GetLocalPlayers())
        {
            con << ColorsCommand::gatherPlayerColor(localNetPlayer) << "\n";
        }
    }
    else
    {
        tArray<tString> commandArgs = args.Split(" ");
        FileManager fileManager(se_colorVarFile, tDirectories::Var());
        tString command;
        bool correctParameters = false;

        bool removeSecondElement = false;
        bool commandIsNumber = commandArgs[0].isNumber();
        bool thirdArgIsNumber = (commandArgs.Len() >= 3) && (commandArgs[2].isNumber());

        // checks if command is for a specific player
        if (commandIsNumber && (!thirdArgIsNumber || (commandArgs.Len() == 4)))
        {
            local_p = ePlayer::PlayerConfig(atoi(commandArgs[0]) - 1);
            if (!local_p)
            {
                con << CommandLabel()
                    << ErrorColor()
                    << tOutput("$player_colors_changed_usage_error");
            }
            targetPlayer = nullptr;
            removeSecondElement = true;
        }

        oldColor = tColor(local_p->rgb[0], local_p->rgb[1], local_p->rgb[2]);
        oldMode = ColorsCommand::localPlayerMode(local_p);

        command = commandArgs[0];

        if (!(commandIsNumber && thirdArgIsNumber) || (commandIsNumber && commandArgs.Len() == 4))
            commandArgs.RemoveAtPreservingOrder(0);

        if (removeSecondElement && !(commandArgs[1].isNumber() && commandArgs[2].isNumber()) )
        {
            command = commandArgs[0];
            commandArgs.RemoveAtPreservingOrder(0);
        }


        if (command == "help")
        {
            con << CommandLabel() << "\n"
                << tOutput("$player_colors_command_help", se_colorVarFile);
            return true;
        }
        else if (command == "mode")
        {
            if (!(commandArgs.Len() >= 1))
            {
                correctParameters = false;
            }
            else
            {
                correctParameters = true;
                tString mode = commandArgs[0].ToLower();

                if (mode.empty())
                {
                    correctParameters = false;
                }
                else if (mode == "off")
                {
                    local_p->colorMode = PlayerColorMode::OFF;
                }
                else if (mode == "random")
                {
                    se_RandomizeColor(local_p);
                    local_p->colorMode = PlayerColorMode::RANDOM;
                }
                else if (mode == "unique")
                {
                    se_UniqueColor(local_p);
                    local_p->colorMode = PlayerColorMode::UNIQUE;
                }
                else if (mode == "rainbow")
                {
                    se_rainbowColor(local_p);
                    local_p->colorMode = PlayerColorMode::RAINBOW;
                }
                else if (mode == "crossfade")
                {
                    se_CrossFadeColor(local_p);
                    local_p->colorMode = PlayerColorMode::CROSSFADE;
                }
                else
                {
                    correctParameters = false;
                }
            }

            if (!correctParameters)
            {
                con << CommandLabel()
                    << "Mode Usage: "
                    << se_rgbCommand << " mode modename \n"
                    << "Available Modes: \n"
                    << " off\n"
                    << " random\n"
                    << " unique\n"
                    << " rainbow\n"
                    << " crossfade\n";
                return true;
            }
        }
        else if (command == "unique")
        {
            // Apply random color to the player who sent the command
            se_UniqueColor(local_p);
            correctParameters = true;
        }
        else if (command == "random")
        {
            // Apply random color to the player who sent the command
            se_RandomizeColor(local_p);
            correctParameters = true;
        }
        else if (command == "save")
        {
            if (commandArgs.Empty())
            {
                tString playerColorStr;

                con << CommandLabel()
                    << ErrorColor()
                    << tOutput("$player_colors_saved");


                if (targetPlayer != nullptr)
                    playerColorStr << ColorsCommand::gatherPlayerColor(targetPlayer);
                else
                    playerColorStr << ColorsCommand::localPlayerPreview(local_p);

                tString output;
                output << tColoredString::RemoveColors(playerColorStr);

                if (fileManager.Write(output))
                    con << CommandLabel()
                        << tOutput("$player_colors_saved")
                        << playerColorStr << "\n";
                else
                    con << CommandLabel()
                        << ErrorColor()
                        << tOutput("$players_color_error");
            }
            else // Save specific persons color
            {
                tString combinedName;
                for (int i = 0; i < commandArgs.Len(); ++i)
                {
                    if (i > 1)
                        combinedName << " ";

                    combinedName << commandArgs[i];
                }

                tString filteredName = combinedName.Filter();
                targetPlayer = ePlayerNetID::GetPlayerByName(combinedName,false);
                if (targetPlayer)
                {
                    tString playerColorStr;
                    playerColorStr << ColorsCommand::gatherPlayerColor(targetPlayer);
                    tString output;
                    output << tColoredString::RemoveColors(playerColorStr);

                    if (fileManager.Write(output))
                        con << CommandLabel()
                            << tOutput("$player_colors_saved")
                            << MainColor() << playerColorStr << "\n";
                    else
                        con << CommandLabel()
                            << ErrorColor()
                            << tOutput("$players_color_error");
                }
                else
                    con << CommandLabel()
                        << ErrorColor()
                        << tOutput("$player_colors_not_found", combinedName);
            }
            return true;
        }
        else if (command == "load")
        {
            int savedColorsCount = fileManager.NumberOfLines();

            if (commandArgs.Empty()) // No Line #
            {
                con << CommandLabel()
                    << ErrorColor()
                    << tOutput("$player_colors_changed_usage_error");
            }
            else if (commandArgs.Len() == 1) // Line # specified
            {
                correctParameters = true;
                int lineNumber = (atoi(commandArgs[0]) - 1);
                if ((lineNumber >= 0) && lineNumber <= savedColorsCount - 1)
                {
                    tArray<tString> colors = fileManager.Load();

                    if (lineNumber < colors.Len())
                    {
                        auto [name, r, g, b] = se_extractColorInfoFromLine(colors[lineNumber]);

                        con << tOutput("$player_colors_loading");
                        se_outputColorInfo(lineNumber, name, r, g, b);

                        local_p->rgb[0] = r;
                        local_p->rgb[1] = g;
                        local_p->rgb[2] = b;
                    }
                }
                else
                    con << CommandLabel()
                        << ErrorColor()
                        << tOutput("$players_color_line_not_found", se_colorVarFile, savedColorsCount, lineNumber + 1);
            }
            return true;
        }
        else if (command == "list")
        {
            int savedColorsCount = fileManager.NumberOfLines();

            if (savedColorsCount > 0)
            {
                con << tOutput("$players_color_list", savedColorsCount, se_colorVarFile);

                int index = 0;
                for (auto color : fileManager.Load())
                {
                    if (!color.empty())
                    {
                        auto [name, r, g, b] = se_extractColorInfoFromLine(color);
                        se_outputColorInfo(index++, name, r, g, b);
                    }
                }
            }
            else
                con << CommandLabel()
                    << ErrorColor()
                    << tOutput("$player_colors_empty");
            return true;
        }
        else if (command == "clear")
        {
            tString line = commandArgs[0];
            if (line.empty())
                correctParameters = false;
            else
            {
                fileManager.Clear(atoi(commandArgs[0]));
                con << CommandLabel() << "\n"
                    << tOutput("$player_colors_cleared", se_colorVarFile);
                return true;
            }
        }
        else if (command == "clearall")
        {
            fileManager.Clear();
            con << CommandLabel()
                << tOutput("$player_colors_cleared", se_colorVarFile);
            return true;
        }
        // Not really checking if the strings passed parameters are numbers,
        // but if someone did /rgb asd asd asd it would just make it 0 0 0.
        else if (commandArgs.Len() == 3) // Apply color to player who sent command
        {
            correctParameters = true;
            local_p->rgb[0] = atoi(commandArgs[0]); // r
            local_p->rgb[1] = atoi(commandArgs[1]); // g
            local_p->rgb[2] = atoi(commandArgs[2]); // b
        }

        // If the correct parameters are passed, display the changes.
        if (correctParameters)
        {
            tColor currentColor(local_p->rgb[0], local_p->rgb[1], local_p->rgb[2]);
            bool colorChanged = oldColor != currentColor;

            con << CommandLabel()
                << tOutput("$player_colors_current_text");

            tString listColors;
            listColors << local_p->ID() + 1 << ") ";


            if (targetPlayer != nullptr)
            {
                ePlayerNetID::ForcedUpdate();
                if (colorChanged)
                {
                    tColoredString oldColoredName;
                    oldColoredName << tColoredString::ColorString(oldColor.r_/15, oldColor.g_/15, oldColor.b_/15) << targetPlayer->GetName();

                    listColors << ColorsCommand::gatherPlayerColor(oldColoredName, oldColor, oldMode)
                               << "--> ";
                }
                listColors << ColorsCommand::gatherPlayerColor(targetPlayer);
            }
            else
            {
                if (colorChanged)
                {
                    tColoredString oldColoredName;
                    oldColoredName << tColoredString::ColorString(local_p->rgb[0]/15,local_p->rgb[1]/15, local_p->rgb[2]/15) << local_p->Name();

                    listColors << ColorsCommand::gatherPlayerColor(oldColoredName, oldColor, oldMode)
                               << "--> ";
                }
                listColors << ColorsCommand::localPlayerPreview(local_p);
            }
            con << listColors << "\n";
        }
        else // incorrect params, error
            con << CommandLabel()
                << ErrorColor()
                << tOutput("$player_colors_changed_usage_error");
    }
    return true;
}

bool BrowserCommand::execute(tString args)
{
    if(ePlayer::PlayerIsInGame(player->ID()))
        ret_to_MainMenu();
    gServerBrowser::BrowseMaster();
    return true;
}

bool SpeakCommand::execute(tString args)
{
    int pos = 0;
    tString PlayerStr = args.ExtractNonBlankSubString(pos);

    ePlayerNetID *targetPlayer = ePlayerNetID::FindPlayerByName(PlayerStr);

    REAL delay = se_speakCommandDelay;
    bool flag = delay > 0;

    if (targetPlayer && targetPlayer->isLocal())
    {
        tString chatString = args.SubStr(pos + 1);

        if (chatString.StartsWith("/"))
            delay = flag = 0;

        if (delay > 0)
            con << CommandLabel()
                << "Sending message with delay: '"
                << ItemColor()
                << delay
                << HeaderColor() << "'\n";

        eChatBot::scheduleMessageTask(targetPlayer, chatString, flag, delay, delay * 0.5);
    }
    else if (targetPlayer && !targetPlayer->isLocal())
        con << CommandLabel()
            << ErrorColor()
            << "Not a local player.\n";
    return true;
}

bool RebuildCommand::execute(tString args)
{
    int pos = 0;
    tString PlayerNumb = args.ExtractNonBlankSubString(pos);

    if (PlayerNumb.empty())
    {
        con << CommandLabel()
            << "Rebuilding all players...\n";
        ePlayerNetID::CompleteRebuild();
    }
    else
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(PlayerNumb) - 1);
        if (local_p)
        {
            con << CommandLabel()
                << "Rebuilding player '"
                << ItemColor()
                << local_p->Name()
                << MainColor() << "'\n";

            netPlayer->Clear(local_p);
            netPlayer->ForcedUpdate();
        }
    }
    return true;
}

bool WatchCommand::execute(tString args)
{
    ePlayer *localPlayer = player;
    if (!localPlayer || !localPlayer->cam)
        return false;

    // Extract the target player name from the input string
    int pos = 0;
    tString targetPlayerName = args.ExtractNonBlankSubString(pos);

    // Find the player by name, if specified
    ePlayerNetID *targetPlayer = nullptr;
    if (!targetPlayerName.empty())
    {
        targetPlayer = ePlayerNetID::FindPlayerByName(targetPlayerName);
        if (!targetPlayer || !targetPlayer->Object() || !targetPlayer->CurrentTeam())
        {
            con << CommandLabel()
                << ErrorColor()
                << "Player not found or is spectating.\n";
            return true;
        }
        else if (targetPlayer != nullptr && localPlayer->watchPlayer == targetPlayer)
        {
            con << CommandLabel()
                << ErrorColor()
                << "Player '"
                << targetPlayer->GetColoredName()
                << MainColor()
                << "' "
                << "already set to the watch player.\n";
            return true;
        }
        else
        {
            con << CommandLabel()
                << "Watch Player set to '"
                << targetPlayer->GetColoredName()
                << MainColor() << "'\n";
            localPlayer->watchPlayer = targetPlayer;
        }
    }
    else
    {
        con << CommandLabel() << "Watch Player removed.\n";
        localPlayer->watchPlayer = nullptr;
    }
    return true;
}


bool ActiveStatusCommand::execute(tString args)
{
    int pos = 0;

    tString PlayerStr = args.ExtractNonBlankSubString(pos);

    ePlayerNetID *p = nullptr;
    if (!PlayerStr.empty())
        p = ePlayerNetID::FindPlayerByName(PlayerStr);
    else if (netPlayer)
        p = netPlayer;

    if (p == nullptr)
        return false;

    REAL chattingTime = p->ChattingTime();


    tColoredString listInfo;
    listInfo << CommandLabel()
             << "Status for " << p->GetColoredName()
             << MainColor() << ":\n"
             << "Created: "
             << ItemColor() << getTimeStringBase(p->createTime_)
             << MainColor() << "\n"
             << "Last Activity: "
             << ItemColor() << st_GetFormatTime(p->LastActivity(), true)
             << MainColor() << "\n"
             << "Chatting For: "
             << ItemColor() << st_GetFormatTime(chattingTime, true)
             << MainColor() << "\n";


    gCycle *cycle = p->NetPlayerToCycle();
    if (cycle)
    {
        if (!cycle->Alive() && cycle->lastDeathTime > 0)
        {
            listInfo << "Last Death: "
                     << ItemColor() << st_GetFormatTime(tSysTimeFloat() - cycle->lastDeathTime, true)
                     << "\n";
        }
        else
        {
            listInfo << "Alive Time: "
                     << ItemColor() << st_GetFormatTime(se_GameTime(), true)
                     << "\n";
        }
    }
    else if (p->CurrentTeam())
    {
        listInfo << "Last Death: "
                    << ItemColor() << st_GetFormatTime(tSysTimeFloat() - p->lastCycleDeathTime, true)
                    << "\n";
    }

    con << listInfo;
    return true;
}

bool ReverseCommand::execute(tString args)
{
    netPlayer->Chat(args.Reverse());
    return true;
}

bool SpectateCommand::execute(tString args)
{
    if (helperConfig::sghuk)
    {
        tString id;
        tRemoveFromList(se_disableCreateSpecific, player->ID() + 1);
        se_forceJoinTeam = false;
    }

    ePlayer *local_p = player;
    bool spectating = local_p->spectate;

    if (!spectating || netPlayer)
    {
        con << CommandLabel()
            << "Spectating player '" << player->name << "'...\n";
        local_p->spectate = true;
        if (netPlayer)
        {

            if (helperConfig::sghuk && se_spectateCommandInstant)
                netPlayer->Clear(local_p);

            netPlayer->ForcedUpdate();
        }
    }
    else
    {
        con << CommandLabel()
            << ErrorColor()
            << "You are already spectating\n";

    }
    return true;
}

bool JoinCommand::execute(tString args)
{
    bool unspectate = true;

    if (helperConfig::sghuk)
    {
        tString id;
        tRemoveFromList(se_disableCreateSpecific,player->ID()+1);
        if (se_disableCreate && player->spectate)
            unspectate = false;
        se_disableCreate = 0;
    }

    if (unspectate)
    {
        player->spectate = false;
        if (!netPlayer)
            con << CommandLabel()
                << "No longer spectating...\n";
    }
    if (netPlayer && !bool(netPlayer->CurrentTeam()))
    {
        con << CommandLabel()
            << "Joining the game...\n";

        netPlayer->CreateNewTeamWish();
        netPlayer->ForcedUpdate();
    }
    else if (netPlayer)
    {
        con << CommandLabel()
            << ErrorColor()
            << "Already joined the game\n";

    }
    return true;
}

bool SearchCommand::execute(tString args)
{
    int pos = 0;

    tString fileName = args.ExtractNonBlankSubString(pos);
    tString output;
    if (fileName.empty())
    {
        output << CommandLabel()
               << "Available files to search:\n"
               << MainColor();

        int i = 1;
        for (const auto &searchableFile : searchableFiles)
        {
            output << i << ") "
                   << ItemColor()
                   << searchableFile.first
                   << MainColor()
                   << " (" << searchableFile.second
                   << ")\n";
            i++;
        }

        output << CommandLabel()
               << "Uses: \n"
               << MainColor()
               << ItemColor() << "/search chat hack the planet (by search phrase)\n"
               << ItemColor() << "/search chat #102 (by line number)\n"
               << ItemColor() << "/search chat #102-105 (by line number range)\n"
               << ItemColor() << "/search chat #102 copy (copy text by single line number)\n"
               << ItemColor() << "/search chat @5 copy (copy match #5 from last search)\n"
               << ItemColor() << "/search chat @5 (copy match #5 from last search)\n";

        con << output;
        return true;
    }
    else
    {

        tString searchPhrase;
        searchPhrase << args.SubStr(pos);
        searchPhrase = tColoredString::RemoveColors(searchPhrase.TrimWhitespace());

        REAL fileSizeMB;
        REAL fileSizeMaxMB;
        REAL numMatches = 0;
        bool fileFound = false;

        if (fileName.empty())
        {
            execute(args);
            return true;
        }

        tToLower(fileName);

        for (const auto &searchableFile : searchableFiles)
        {
            if (searchableFile.first == fileName) // searchable file name
            {
                fileName = searchableFile.second; // actual file name
                fileFound = true;
                break;
            }
        }

        if (!fileFound)
        {
            con << CommandLabel()
                << ErrorColor()
                << "File not found: '"
                << fileName << "'\n";
            return true;
        }

        FileManager fileManager(fileName, tDirectories::Log());
        auto lines = fileManager.Load();

        fileSizeMB = gHelperUtility::BytesToMB(fileManager.FileSize());
        fileSizeMaxMB = (fileName == tString("consolelog-limited.txt")) ? sr_consoleLogLimitedSize : se_searchCommandMaxFileSizeMB;

        if (fileSizeMB > fileSizeMaxMB)
        {
            con << CommandLabel()
                << ErrorColor()
                << "Error: File is too big: '" << fileName << "' ("
                << fileSizeMB << " MB /"
                << fileSizeMaxMB << " MB)\n";
            return true;
        }

        if (searchPhrase.empty())
        {
            netPlayer->lastSearch.Clear();
            int start = std::max(0, lines.Len() - se_searchCommandEmptySearchNumLines); // don't go below 0
            int count = 1;
            tString fileNameOut;
            fileNameOut << CommandLabel()
                        << "\nFile '" << ItemColor() << fileName      << MainColor()
                        << "- "       << ItemColor() << fileSizeMB    << MainColor()
                        << " MB / "   << ItemColor() << fileSizeMaxMB << MainColor() << " MB\n";

            con << fileNameOut << MainColor()
                << "Nothing to search. Showing last " << ItemColor()
                << se_searchCommandEmptySearchNumLines
                << MainColor() << " lines:\n";

            for (int i = start; i < lines.Len(); ++i)
            {
                netPlayer->lastSearch.Add(lines[i]);
                con << ItemColor()   << count++
                    << HeaderColor() <<  ") " << HeaderColor()
                    << "Line " << MainColor() << (i + 1)
                    << ": "    << lines[i]   << "\n";
            }
            return true;
        }

        tString output;
        int lineNumber = 1;
        bool found = false;
        bool copy = false;

        bool copySpecificLineNum = searchPhrase.StartsWith("#");
        bool copyNumMatch = searchPhrase.StartsWith("@");
        if (copySpecificLineNum || copyNumMatch)
        {
            tString actualSearchPhrase = searchPhrase.SubStr(1);
            int copyPos = actualSearchPhrase.StrPos(" copy");
            copy = copyNumMatch || (copyPos != -1);

            if (!copyNumMatch && copy)
                actualSearchPhrase = actualSearchPhrase.SubStr(0, copyPos); // Remove " copy" from the search phrase

#ifdef MACOSX
            copy = false;
            copyNumMatch = false;
#endif

            int startLineNumber = -1;
            int endLineNumber = -1;

            int dashPos = actualSearchPhrase.StrPos("-");
            if (dashPos != -1)
            {
                startLineNumber = atoi(actualSearchPhrase.SubStr(0, dashPos));
                endLineNumber = atoi(actualSearchPhrase.SubStr(dashPos + 1));
            }
            else
            {
                startLineNumber = atoi(actualSearchPhrase);
                endLineNumber = startLineNumber;
            }

            if (copyNumMatch && startLineNumber == endLineNumber)
            {
                if (!netPlayer->lastSearch.Len() > 0 || netPlayer->lastSearch.Len() < endLineNumber)
                {
                    con << CommandLabel()
                        << ErrorColor()
                        << "Nothing to copy from.\n";
                }
                else
                {
                    tString message(netPlayer->lastSearch[endLineNumber - 1]);
                    if (copyToClipboard(message))
                    {
                        con << CommandLabel()
                            << "Copied content to clipboard.\n";
                        numMatches++;
                        output << endLineNumber << " " << MainColor() << message << "\n";
                        found = true;
                    }
                }
            }
            else
            {
                for (tString line : lines)
                {
                    if ((lineNumber >= startLineNumber && lineNumber <= endLineNumber))
                    {
                        numMatches++;
                        output << MainColor()
                               << numMatches << ") "
                               << HeaderColor()
                               << "Line " << ItemColor() << lineNumber << MainColor()
                               << ": " << line << "\n";

                        if (copy && startLineNumber == endLineNumber) // only copy for one line
                        {
                            tString lineToCopy;
                            lineToCopy = output.SubStr(output.StrPos(": ") + 2); // Remove everything before the line content

                            if (copyToClipboard(lineToCopy))
                                con << HeaderColor()
                                    << "Copied content to clipboard.\n";
                            found = true;
                            break;
                        }
                    }

                    if (lineNumber > endLineNumber)
                    {
                        found = true;
                        break;
                    }

                    lineNumber++;
                }
            }
            if (!found)
            {
                if (!copyNumMatch)
                    con << CommandLabel()
                        << ErrorColor()
                        << "Line Range: " << ItemColor()
                        << startLineNumber << "-" << endLineNumber
                        << ErrorColor() << " not found.\n";
                return true;
            }
        }

        if (!copy)
        {
            netPlayer->lastSearch.Clear();
            tString searchValue(searchPhrase);

            if (!se_searchCommandCaseSensitive)
                tToLower(searchValue);

            for (tString line : lines)
            {
                tString lineComparison(line);

                if (!se_searchCommandCaseSensitive)
                    tToLower(lineComparison);

                if (lineComparison.Contains(searchValue))
                {
                    found = true;
                    numMatches++;
                    output << numMatches << ") " << HeaderColor() << "Line "
                           << ItemColor() << lineNumber
                           << MainColor()
                           << ": "       << line << "\n";
                    netPlayer->lastSearch.Add(line);
                }
                lineNumber++;
            }
        }

        if (!found && !copy)
        {
            tString fileNameOut;
            fileNameOut << "\n"     << HeaderColor()
                        << "File '" << ItemColor() << fileName      << MainColor()
                        << "- "     << ItemColor() << fileSizeMB    << MainColor()
                        << " MB / " << ItemColor() << fileSizeMaxMB << MainColor() << " MB\n";

            con << CommandLabel()
                << fileNameOut
                << "No matches found for the search phrase: '" << ItemColor()
                << searchPhrase << MainColor() << "'\n";
        }
        else
        {
            tString fileNameOut;
            fileNameOut << HeaderColor()
                        << "\nFile '" << ItemColor() << fileName << MainColor()
                        << "- " << ItemColor() << fileSizeMB << MainColor()
                        << " MB / " << ItemColor() << fileSizeMaxMB << MainColor() << " MB\n";

            tString matches;
            matches << fileNameOut << "Found "
                    << ItemColor() << numMatches
                    << MainColor() << " matches for: ";

            output = CommandLabel()
                     << matches << tString("'") << ItemColor()
                     << searchPhrase << MainColor() << tString("'\n")
                     << output;

            con << output;
        }
    }
    return true;
}

bool NameSpeakCommand::execute(tString args)
{
    if (!helperConfig::sghuk)
        return true;

    if (args.empty())
    {
        con << CommandLabel()
            << ErrorColor()
            << "Usage: " << se_nameSpeakCommand << " <name> <message>\n";
        return true;
    }

    if (se_nameSpeakCommandSplitByNameSize)
        ePlayerNetID::nameSpeakWords = args.SplitBySize(15, true);
    else
        ePlayerNetID::nameSpeakWords = args.Split(" ");

    int playerID = -1;
    ePlayer *local_p = nullptr;
    bool found = false;
    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        local_p = ePlayer::PlayerConfig(i);
        if (!bool(local_p->netPlayer.get()) && !tIsInList(se_disableCreateSpecific, i + 1))
        {
            local_p = ePlayer::PlayerConfig(i);
            playerID = i;
            found = true;
            break;
        }
    }

    if (!found || !local_p)
    {
        con << CommandLabel()
            << ErrorColor()
            << "No usable players!\n";
        ePlayerNetID::nameSpeakWords.Clear();
        return true;
    }

    con << CommandLabel()
        << "\n  - Using Player '"
        << ItemColor() << ePlayerNetID::nameSpeakPlayerID + 1
        << MainColor()
        << "'. Message: '" << ItemColor() << args
        << MainColor() << "'\n";

    ePlayerNetID::nameSpeakForceUpdate = true;
    ePlayerNetID::nameSpeakCheck = true;
    ePlayerNetID::nameSpeakIndex = 0;
    ePlayerNetID::nameSpeakPlayerID = playerID;
    ePlayerNetID::playerUpdateIteration = 0;

    return true;
}

bool RespawnCommand::execute(tString args)
{
    con << CommandLabel()
        << "Respawning player '" << netPlayer->GetName() << "'\n";
    netPlayer->RespawnPlayer(true);
    return true;
}

bool RebuildGridCommand::execute(tString args)
{
    tArray<tString> passedString = args.Split(" ");
    if (args.empty())
    {
        con << CommandLabel()
            << ErrorColor()
            << "Usage: " << se_rebuildCommand << " <#state>\n";
        return true;
    }
    if (gGame::CurrentGame())
        gGame::CurrentGame()->RebuildGrid(atoi(passedString[1]));

    return true;
}

bool SaveConfigCommand::execute(tString args)
{
    con << st_AddToUserExt(args.Split(" "));
    return true;
}

bool ReplyCommand::execute(tString args)
{
    if (netPlayer->lastMessagedByPlayer == nullptr && netPlayer->lastMessagedPlayer == nullptr)
    {
        con << CommandLabel()
            << ErrorColor()
            << "You have not messaged anyone yet!\n";
        return false;
    }
    ePlayerNetID *targetPlayer = netPlayer->lastMessagedByPlayer;

    if (targetPlayer == nullptr)
        targetPlayer = netPlayer->lastMessagedPlayer;

    tString name = targetPlayer->GetName().Filter();

    if (se_chatLog)
    {
        tString logOutput;

        if (se_chatTimeStamp)
            logOutput << st_GetCurrentTime("%H:%M:%S| ");

        tString logArgs;
        int spacePos = logArgs.StrPos(" ");

        if (spacePos != -1)
        {
            spacePos++;
            logArgs.RemoveSubStr(0, spacePos);
        }

        logOutput << netPlayer->GetName()
                  << " --> "
                  << targetPlayer->GetName()
                  << ": "
                  << logArgs;

        se_SaveToChatLog(logOutput);
    }

    tString messageToSend;
    messageToSend << "/msg " << name << " " << args;
    netPlayer->Chat(messageToSend);
    return true;
}

bool NicknameCommand::execute(tString args)
{
    if (args.empty())
    {
        // Clear all nicknames
        for (ePlayerNetID *p : se_PlayerNetIDs)
        {
            p->nickname.Clear();
            p->coloredNickname.Clear();
        }
        con << CommandLabel()
            << "All nicknames have been cleared.\n";
        return true;
    }

    int posSpace = args.StrPos(" ");
    if (posSpace == -1)
    {
        // No space found, assume only a name is provided
        tString targetName = args;
        ePlayerNetID *target = ePlayerNetID::FindPlayerByName(targetName);
        if (target)
        {
            con << CommandLabel()
                << target->coloredName_
                << MainColor()
                << "'s nickname has been cleared.\n";
            target->nickname.Clear();
            target->coloredNickname.Clear();
            return true;
        }
    }
    else
    {
        // Extract name and nickname parts
        tString targetName = args.SubStr(0, posSpace);
        tString nickname = args.SubStr(posSpace + 1);

        ePlayerNetID *target = ePlayerNetID::FindPlayerByName(targetName);
        if (target)
        {
            target->nickname = nickname;
            target->UpdateName();

            con << CommandLabel()
                << target->coloredName_
                << MainColor()
                << " is now nicknamed '" << target->GetColoredNickName()
                << MainColor() << "'\n";

            return true;
        }
    }

    con << CommandLabel()
        << ErrorColor()
        << "Could not find player '" << ItemColor()
        << args << ErrorColor() << "'\n";
    return false;
}

bool StatsCommand::execute(tString args)
{

    nServerInfoBase *connectedServer = CurrentServer();
    if (connectedServer)
        con << CommandLabel()
            << "Current Server = "
            << connectedServer->GetName()
            << "\n";

    int pos = 0;

    tString PlayerStr = args.ExtractNonBlankSubString(pos);

    ePlayerNetID *p = nullptr;
    if (!PlayerStr.empty())
        p = ePlayerNetID::FindPlayerByName(PlayerStr);
    else if (netPlayer)
        p = netPlayer;

    if (p == nullptr)
        return false;


    if (!se_playerStats)
    {
        con << CommandLabel()
            << ErrorColor()
            << "PlayerStats are disabled!\n";
        return true;
    }

    PlayerData playerData = ePlayerStats::getStats(p->GetName());

    tColoredString statsInfo;
    statsInfo << CommandLabel()
              << "\nStats for " << ItemColor() << p->GetColoredName() << "\n"
              << MainColor()
              << "Kills: " << ItemColor() << playerData.kills << "\n"
              << MainColor()
              << "Deaths: " << ItemColor() << playerData.deaths << "\n"
              << MainColor()
              << "Match Wins: " << ItemColor() << playerData.match_wins << "\n"
              << MainColor()
              << "Match Losses: " << ItemColor() << playerData.match_losses << "\n"
              << MainColor()
              << "K/D Ratio: " << ItemColor() << playerData.getKDRatio() << "\n";

    con << statsInfo;
    return true;
}

bool ReconnectCommand::execute(tString args)
{
    nServerInfoBase *connectedServer = CurrentServer();
    if (connectedServer)
    {
        con << CommandLabel()
            << "Reconnecting to '" << connectedServer->GetName() << "'...\n";

    if(ePlayer::PlayerIsInGame(player->ID()))
        ret_to_MainMenu();

        ConnectToServer(connectedServer);
    }
    else
    {
        if (!ConnectToLastServer())
            con << CommandLabel()
                << ErrorColor()
                << "You are not connected to a server!\n";
    }
    return true;
}

#include <stack>
#include <queue>
bool CalculateCommand::execute(tString args)
{
    con << CommandLabel()
        << "Performing calculation: '" << ItemColor()
        << args << MainColor() << "'\n";
    std::stack<double> values;
    std::queue<tString> postfix = infixToPostfix(preprocess(args));

    tString token;
    while (!postfix.empty())
    {
        token = postfix.front();
        postfix.pop();
        if (std::isdigit(token[0]))
        {
            values.push(std::stod(token.c_str()));
        }
        else
        {
            if (values.size() < 2)
            {
                con << ErrorColor()
                    << "Error: Not enough values for operation\n";
                return false;
            }

            double rhs = values.top();
            values.pop();
            double lhs = values.top();
            values.pop();

            if (token == "+")
                values.push(lhs + rhs);
            else if (token == "-")
                values.push(lhs - rhs);
            else if (token == "*")
                values.push(lhs * rhs);
            else if (token == "/")
            {
                if (rhs == 0)
                {
                    con << ErrorColor()
                        << "Error: Division by zero\n";
                    return false;
                }
                values.push(lhs / rhs);
            }
            else if (token == "^")
                values.push(std::pow(lhs, rhs));
        }
    }

    if (values.size() != 1)
    {
        con << ErrorColor()
            << "Error: Too many values\n";
        return false;
    }

    std::ostringstream strs;
    strs << std::fixed << std::setprecision(6) << values.top();
    std::string str = strs.str();

    std::size_t lastNonZero = str.find_last_not_of('0');
    if (lastNonZero != std::string::npos)
    {
        if (str[lastNonZero] == '.')
            str.erase(lastNonZero, str.length() - lastNonZero);
        else
            str.erase(lastNonZero + 1, str.length() - lastNonZero - 1);
    }

con << MainColor()
    << "Result: '" << ItemColor()
    << str
    << MainColor() << "'\n";

    return true;
}

tString CalculateCommand::preprocess(const tString &input)
{
    std::string str(input.c_str());
    std::string processed;
    for (char &ch : str)
    {
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^')
        {
            processed += ' ';
            processed += ch;
            processed += ' ';
        }
        else
        {
            processed += ch;
        }
    }
    return tString(processed.c_str());
}

std::queue<tString> CalculateCommand::infixToPostfix(const tString &infix)
{
    std::map<char, int> prec;
    prec['^'] = 3;
    prec['*'] = 2;
    prec['/'] = 2;
    prec['+'] = 1;
    prec['-'] = 1;

    std::queue<tString> queue;
    std::stack<tString> stack;

    std::istringstream tokens(infix.c_str());
    std::string token;
    while (tokens >> token)
    {
        tString tToken(token.c_str());
        if (std::isdigit(tToken[0]))
        {
            queue.push(tToken);
        }
        else if (tToken == "+" || tToken == "-" || tToken == "*" || tToken == "/" || tToken == "^")
        {
            while (!stack.empty() && prec[tToken[0]] <= prec[stack.top()[0]])
            {
                queue.push(stack.top());
                stack.pop();
            }

            stack.push(tToken);
        }
    }

    while (!stack.empty())
    {
        queue.push(stack.top());
        stack.pop();
    }

    return queue;
}

bool UpdateCommand::execute(tString args)
{
    int pos = 0;
    tString PlayerNumb = args.ExtractNonBlankSubString(pos);

    if (PlayerNumb.empty())
    {
        con << CommandLabel()
            << "Updating all players...\n";
        ePlayerNetID::ForcedUpdate();
    }
    else
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(PlayerNumb) - 1);
        if (local_p)
        {
            con << CommandLabel()
                << "Updating player '"
                << ItemColor()
                << local_p->Name()
                << MainColor() << "'\n";

            ePlayerNetID::Update(local_p);
        }
        else
        {
            con << CommandLabel()
                << ErrorColor()
                << "No player for ID '"
                << ItemColor()
                << PlayerNumb
                << ErrorColor()
                << "'\n";
        }
    }
    return true;
}

REAL EncryptCommand::getEncryptLocaltime()
{
    return getCurrentLocalTime().tm_sec;
}

tString EncryptCommand::GenerateHash(double time)
{
    std::stringstream ss;
    ss << time;

    std::hash<std::string> hash_fn;
    size_t hash_value = hash_fn(ss.str());

    std::ostringstream os;
    os << hash_value;

    std::string hash = os.str();

    size_t length = (hash.size() > se_encryptCommandLength) ? se_encryptCommandLength : hash.size();
    return tString(hash.substr(0, length));
}

bool EncryptCommand::ValidateHash(tString givenHash, double time)
{
    if (GenerateHash(time) == givenHash)
        return true;

    for (int i = 1; i <= se_encryptCommandWatchValidateWindow; i++)
    {
        if ((GenerateHash(time - i) == givenHash) || (GenerateHash(time + i) == givenHash))
            return true;
    }
    return false;
}

bool EncryptCommand::handleEncryptCommandAction(ePlayerNetID *player, tString message)
{
    if (!message.StartsWith(se_encryptCommandPrefix))
        return false;

    int startIndex = message.StrPos(se_encryptCommandPrefix) + se_encryptCommandPrefix.Len() - 1;

    tString hashStr = message.SubStr(startIndex, se_encryptCommandLength);

    REAL currentTime = getEncryptLocaltime();
    bool valid = ValidateHash(hashStr, currentTime);

    if (!valid)
    {
        con << "EncryptCommand: Hash Invalid '" << hashStr << "' at time: " << currentTime << "\n";
        return true;
    }

    player->encryptVerified = true;

    int argsStartIndex = startIndex + se_encryptCommandLength + 1;

    if (argsStartIndex > message.Len())
    {
        con << "EncryptCommand: No arguments found after the hash.\n";
        return true;
    }

    tConfItemBase::lastLoadOutput = tString("");

    tString args = message.SubStr(argsStartIndex);
    con << "EncryptCommand: Loading command '" << args << "'\n";

    ePlayerNetID *consoleUser = player->lastMessagedPlayer;
    if (!consoleUser)
    {
        tConfItemBase::lastLoadOutput << "This player has not messaged you\n";
        con << tConfItemBase::lastLoadOutput;
    }
    else
    {
        ePlayer *local_p = ePlayer::NetToLocalPlayer((player->lastMessagedPlayer));
        if (!local_p)
        {
            tConfItemBase::lastLoadOutput << "No local console user available to run this command.\n";
            con << tConfItemBase::lastLoadOutput;
        }
        else
        {
            sn_consoleUser(local_p);
            tCurrentAccessLevel level(tAccessLevel_Owner, true);
            std::stringstream s(static_cast<char const *>(args));
            tConfItemBase::LoadAll(s);
        }
    }

    if (se_encryptCommandWatchFeedback)
    {
        tString feedback;
        tString messageToSend;

        if (tConfItemBase::lastLoadOutput.empty())
            feedback << "Line loaded: '" << args << "'";
        else
            feedback << tConfItemBase::lastLoadOutput;

        messageToSend << "/msg " << player->GetName().Filter() << " " << feedback << "\n";
        for (auto localNetPlayer : se_GetLocalPlayers())
        {
            se_NewChatMessage(localNetPlayer, messageToSend)->BroadCast();
            break;            
        }
    }

    return true;
}

bool EncryptCommand::execute(tString args)
{
    if (!helperConfig::sghuk)
        return true;

    int pos = 0;
    tString name = args.ExtractNonBlankSubString(pos);
    ePlayerNetID *encTarget = ePlayerNetID::GetPlayerByName(name, false);

    if (encTarget)
    {
        REAL currentTime = getEncryptLocaltime();

        con << CommandLabel()
            << "Creating hash at time: '"
            << ItemColor()
            << currentTime
            << MainColor() << "'\n";

        tString hash = GenerateHash(currentTime);

        tString messageToSend;
        messageToSend << "/msg " << name << " " << se_encryptCommandPrefix << hash;

        if (pos < args.Len())
            messageToSend << args.SubStr(pos);

        se_NewChatMessage(netPlayer, messageToSend)->BroadCast();
    }
    else
        con << CommandLabel()
            << ErrorColor()
            << "Player not found for '"
            << args
            << "'\n";

    return true;
}

#include "eVoter.h"
bool VoteCommand::execute(tString args)
{
    if (args.empty())
    {
        displayPollsMenu();
        return true;
    }

    tArray<tString> params = args.Split(" ");
    int length = params.Len();

    if (length == 2)
    {
        if (params[0].isNumber())
            processVote(params);
        else
            sendChatMessage(netPlayer, args);
    }
    else
        con << "Invalid command format.\n";

    return true;
}

void VoteCommand::displayPollsMenu()
{
    con << CommandLabel() << "Polls:\n";

    if (eVoter::ChatDisplayVotes())
    {
        con << "0x888888 - Example Usage: (/vote ID yes)" << MainColor() << "\n";
    }
}

void VoteCommand::sendChatMessage(ePlayerNetID *player, tString &message)
{
    tString voteStr;
    voteStr << "/vote "
            << message;
    se_NewChatMessage(player, voteStr)->BroadCast();
}

void VoteCommand::processVote(const tArray<tString> &params)
{
    tString acceptPoll = params[1].ToLower();

    if (acceptPoll == "yes")
        eVoter::ChatSubmitPoll(atoi(params[0]), true);
    else if (acceptPoll == "no")
        eVoter::ChatSubmitPoll(atoi(params[0]), false);
    else
        con << "Invalid option: '" << acceptPoll << "'. Valid options: yes/no\n";
}

bool RenameCommand::execute(tString args)
{
    con << CommandLabel()
        << "Renaming player '" << player->name << "' to '" << args << "'...\n";

    player->name = args;
    if (netPlayer)
        netPlayer->ForcedUpdate();
    return true;
}

bool LeaveCommand::execute(tString args)
{
    tString server;

    nServerInfoBase *connectedServer = CurrentServer();
    if (connectedServer)
        server << connectedServer->GetName();

    con << CommandLabel()
        << "Leaving server '" << server << "'";

    ret_to_MainMenu();

    return true;
}

bool QuitCommand::execute(tString args)
{

    int quitTime = se_quitCommandTime;
    int pos = 0;

    if (!args.empty())
        quitTime = atoi(args.ExtractNonBlankSubString(pos));

    st_SaveConfig();

    con << CommandLabel()
        << "Quiting game in '" << quitTime << "' seconds...\n";

    gTaskScheduler.schedule("QuitCommand", quitTime, []
    {
        uMenu::quickexit = uMenu::QuickExit_Total;
    });
    return true;
}

bool BookmarksCommand::execute(tString args)
{
    tString input(args);
    input = input.ToLower();

    if (!CurrentServer() || (input == "poll" || input == "1" || input == "yes" || input == "true"))
    {
        con << CommandLabel()
            << "Disconnecting and loading bookmark menu.\n";
        gServerFavorites::FavoritesMenuForceQuery(true);
    }
    else
    {
        con << CommandLabel()
            << "Loading bookmark menu. (Polling a server will disconnect you from the game).\n";
        gServerFavorites::FavoritesMenuForceQuery(false);
    }
    return true;
}
