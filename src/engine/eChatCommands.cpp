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
                  << msgTarget->GetName()
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

tColoredString ColorsCommand::cycleColorPreview(REAL r, REAL g, REAL b)
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
                 << tColoredString::ColorString(r, g, b) << "=="
                //  << tColoredString::ColorString(tailR2, tailG2, tailB2) << "="
                 << eChatCommand::MainText();

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
           << eChatCommand::MainText()
           << " ("
           << eChatCommand::ItemText() << r << eChatCommand::MainText() << ", "
           << eChatCommand::ItemText() << g << eChatCommand::MainText() << ", "
           << eChatCommand::ItemText() << b << eChatCommand::MainText() << ") "
           << cycleColorPreview(r, g, b)
           << " (mode: "
           << eChatCommand::ItemText() << localPlayerMode(local_p) << eChatCommand::MainText() << ") ";

    return output;
}

tColoredString ColorsCommand::gatherPlayerColor(ePlayerNetID *p, bool showReset)
{
    tColoredString listColors, cyclePreview;

    if (showReset)
        listColors << p->GetColoredName() << eChatCommand::MainText() << " (";
    else
        listColors << p->GetColoredName().StripWhitespace() << eChatCommand::MainText() << " (";

    listColors << eChatCommand::ItemText() << p->r << eChatCommand::MainText() << ", "
               << eChatCommand::ItemText() << p->g << eChatCommand::MainText() << ", "
               << eChatCommand::ItemText() << p->b << eChatCommand::MainText() << ") "
               << cycleColorPreview(p->r, p->g, p->b);

    if (p->isLocal())
    {
        ePlayer *local_p = ePlayer::NetToLocalPlayer(p);
        listColors << " (mode: "
                   << eChatCommand::ItemText()
                   << localPlayerMode(local_p)
                   << eChatCommand::MainText()
                   << ") ";
}

    return listColors;
}

bool ColorsCommand::execute(tString args)
{
    if (se_PlayerNetIDs.Len() > 0)
    {
        if (args.empty())
        {
            con << CommandText() << tOutput("$player_colors_text") << MainText();
            for (int i = 0; i <= se_PlayerNetIDs.Len() - 1; i++)
                con << ItemText() << (i + 1) << MainText() << ") " << gatherPlayerColor(se_PlayerNetIDs(i)) << "\n";
        }
        else
        {
            bool playerFound = false;
            tArray<tString> searchWords = args.Split(" ");

            con << CommandText() << tOutput("$player_colors_text") << MainText();

            int j = 0;
            for (int i = 0; i < searchWords.Len(); i++)
            {
                ePlayerNetID *p = ePlayerNetID::GetPlayerByName(searchWords[i], false);
                if (p)
                {
                    playerFound = true;
                    j++;
                    con << ItemText() << j << MainText() << ") " << gatherPlayerColor(p) << "\n";
                }
            }
            // No one found.
            if (!playerFound)
                con << ErrorText() << tOutput("$player_colors_not_found", searchWords[1]) << MainText();
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
                con << CommandText()
                    << tOutput("$player_info_text")
                    << "\n"
                    << gatherPlayerInfo(netPlayer);
            else
                con << CommandText()
                    << ErrorText()
                    << "net player does not exist\n";
        }
        else
        {
            bool playerFound = false;
            tArray<ePlayerNetID *> foundPlayers;
            tArray<tString> msgsExt = args.Split(" ");

            con << CommandText()
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
                    con << ItemText() << j << MainText() << ") " << gatherPlayerInfo(p) << "\n";
                }
            }

            if (!playerFound)
                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_not_found_text", args);
        }
    }
    return true;
}

tColoredString listPlayerInfoCommand::gatherPlayerInfo(ePlayerNetID *p)
{


    tColoredString listInfo;
    listInfo << eChatCommand::MainText() << "Results for " << p->GetColoredName() << eChatCommand::MainText() << ":\n";

    // Player Info
    listInfo << eChatCommand::MainText() << "Player Info: \n";

    tString nameHistory;

    for (int i = 0; i < p->nameHistory.Len(); i++)
    {
        if (nameHistory.empty())
            nameHistory = p->nameHistory[i];
        else {
            nameHistory = nameHistory + eChatCommand::MainText() + ", " + p->nameHistory[i];
        }
    }

    listInfo << eChatCommand::MainText() << " Color: " << ColorsCommand::gatherPlayerColor(p) << "\n";

    gRealColor color(p->r, p->g, p->b);
    se_MakeColorValid(color.r, color.g, color.b, 1.0f);
    se_removeDarkColors(color);
    listInfo << eChatCommand::MainText()
             << " Filtered Color: "      << eChatCommand::MainText()            << "("
             << eChatCommand::ItemText() << color.r << eChatCommand::MainText() << ", "
             << eChatCommand::ItemText() << color.g << eChatCommand::MainText() << ", "
             << eChatCommand::ItemText() << color.b << eChatCommand::MainText() << ")\n";

    listInfo << " Name History: "
             << nameHistory
             << "\n";

    listInfo << " Status: "
             << eChatCommand::ItemText() << (p->IsHuman() ? "Human" : "Bot")
             << eChatCommand::MainText() << ", "
             << eChatCommand::ItemText() << (p->CurrentTeam() ? "Playing" : "Spectating")
             << (p->IsChatting() ? (eChatCommand::MainText() << "," << eChatCommand::ItemText() << " Chatting\n") : "\n");

    listInfo << " Ping: "
             << eChatCommand::ItemText()
             << int(p->ping * 1000)
             << eChatCommand::MainText()
             << " ("
             << eChatCommand::ItemText()
             << p->ping
             << eChatCommand::MainText()
             << ")\n";

    listInfo << " Created: "
             << eChatCommand::ItemText() << getTimeStringBase(p->createTime_)
             << eChatCommand::MainText() << "\n"
             << " Last Activity: "
             << eChatCommand::ItemText() << st_GetFormatTime(p->LastActivity(), true)
             << eChatCommand::MainText() << "\n";

    if (p->ChattingTime() > 0)
        listInfo << " Chatting For: "
                 << eChatCommand::ItemText()
                 << st_GetFormatTime(p->ChattingTime(), true)
                 << eChatCommand::MainText() << "\n";


    // Cycle Info

    if (p->CurrentTeam())
        listInfo << eChatCommand::MainText()
                << "Cycle Info: \n";
    if (p->Object())
    {
        // Only grab this information if the player is an alive object.
        gCycle *pCycle = dynamic_cast<gCycle *>(p->Object());

        listInfo << " Status: "
                 << eChatCommand::ItemText()
                 << (pCycle->Alive() ? "Alive" : "Dead")
                 << '\n'
                 << eChatCommand::MainText()
                 << " Lag: "
                 << eChatCommand::ItemText()
                 << int(pCycle->Lag()  * 1000)
                 << eChatCommand::MainText()
                 << " ("
                 << eChatCommand::ItemText()
                 << pCycle->Lag()
                 << eChatCommand::MainText()
                 << ")\n";

        if (!pCycle->Alive() && pCycle->lastDeathTime > 0)
            listInfo << " Last Death: "
                     << ItemText() << st_GetFormatTime(tSysTimeFloat() - pCycle->lastDeathTime, true)
                     << "\n";
        else
            listInfo << " Alive Time: "
                     << eChatCommand::ItemText() << st_GetFormatTime(se_GameTime(), true)
                     << "\n";

        if (pCycle->Alive())
        {
            listInfo << eChatCommand::MainText()
                     << " Position: x: "
                     << eChatCommand::ItemText() << pCycle->Position().x
                     << eChatCommand::MainText()
                     << ", y: "
                     << eChatCommand::ItemText() << pCycle->Position().y
                     << eChatCommand::MainText() << "\n"
                     << " Map Direction: x: "
                     << eChatCommand::ItemText() << pCycle->Direction().x
                     << eChatCommand::MainText()
                     << ", y: "
                     << eChatCommand::ItemText() << pCycle->Direction().y
                     << eChatCommand::MainText() << '\n'
                     << " Speed: "
                     << eChatCommand::ItemText() << pCycle->verletSpeed_
                     << eChatCommand::MainText() << '\n'
                     << " Rubber: "
                     << eChatCommand::ItemText() << pCycle->GetRubber() << "/" << sg_rubberCycle
                     << eChatCommand::MainText() << '\n';
        }
    }
    else if (p->CurrentTeam())
    {
        listInfo << " Status: "
                 << eChatCommand::ItemText() << "Dead\n";

        listInfo << " Last Death: "
                    << ItemText() << st_GetFormatTime(tSysTimeFloat() - p->lastCycleDeathTime, true)
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
           << name << eChatCommand::MainText()
           << " ("
           << eChatCommand::ItemText() << r << eChatCommand::MainText() << ", "
           << eChatCommand::ItemText() << g << eChatCommand::MainText() << ", "
           << eChatCommand::ItemText() << b << eChatCommand::MainText() << ") "
           << ColorsCommand::cycleColorPreview(r, g, b) << "\n";

    con << output;
}

bool RgbCommand::execute(tString args)
{
    ePlayer *local_p = player;
    ePlayerNetID *targetPlayer = netPlayer;
    if (!local_p)
        return false;

    if (args.empty())
    {
        con << CommandText()
            << tOutput("$player_colors_current_text");
        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            ePlayer *loc_p = ePlayer::PlayerConfig(i);
            ePlayerNetID *net_p = loc_p->netPlayer.get();
            if (bool(net_p))
                con << ColorsCommand::gatherPlayerColor(net_p) << "\n";
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
                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_colors_changed_usage_error");
            }
            targetPlayer = nullptr;
            removeSecondElement = true;
        }

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
            con << CommandText() << "\n"
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
                con << CommandText()
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

                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_colors_saved");


                if (targetPlayer != nullptr)
                    playerColorStr << ColorsCommand::gatherPlayerColor(targetPlayer);
                else
                    playerColorStr << ColorsCommand::localPlayerPreview(local_p);

                tString output;
                output << tColoredString::RemoveColors(playerColorStr);

                if (fileManager.Write(output))
                    con << CommandText()
                        << tOutput("$player_colors_saved")
                        << playerColorStr << "\n";
                else
                    con << CommandText()
                        << ErrorText()
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
                        con << CommandText()
                            << tOutput("$player_colors_saved")
                            << MainText() << playerColorStr << "\n";
                    else
                        con << CommandText()
                            << ErrorText()
                            << tOutput("$players_color_error");
                }
                else
                    con << CommandText()
                        << ErrorText()
                        << tOutput("$player_colors_not_found", combinedName);
            }
            return true;
        }
        else if (command == "load")
        {
            int savedColorsCount = fileManager.NumberOfLines();

            if (commandArgs.Empty()) // No Line #
            {
                con << CommandText()
                    << ErrorText()
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
                    con << CommandText()
                        << ErrorText()
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
                con << CommandText()
                    << ErrorText()
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
                con << CommandText() << "\n"
                    << tOutput("$player_colors_cleared", se_colorVarFile);
                return true;
            }
        }
        else if (command == "clearall")
        {
            fileManager.Clear();
            con << CommandText()
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
            con << CommandText()
                << tOutput("$player_colors_current_text");

            tString listColors;
            listColors << local_p->ID() + 1 << ") ";

            if (targetPlayer != nullptr)
            {
                ePlayerNetID::ForcedUpdate();
                listColors << ColorsCommand::gatherPlayerColor(targetPlayer);
            }
            else
                listColors << ColorsCommand::localPlayerPreview(local_p);

            con << listColors << "\n";
        }
        else // display the error message.
            con << CommandText()
                << ErrorText()
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
            con << CommandText()
                << "Sending message with delay: '"
                << ItemText()
                << delay
                << HeaderText() << "'\n";

        eChatBot::scheduleMessageTask(targetPlayer, chatString, flag, delay, delay * 0.5);
    }
    else if (targetPlayer && !targetPlayer->isLocal())
        con << CommandText()
            << ErrorText()
            << "Not a local player.\n";
    return true;
}

bool RebuildCommand::execute(tString args)
{
    int pos = 0;
    tString PlayerNumb = args.ExtractNonBlankSubString(pos);

    if (PlayerNumb.empty())
    {
        con << CommandText()
            << "Rebuilding all players...\n";
        ePlayerNetID::CompleteRebuild();
    }
    else
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(PlayerNumb) - 1);
        if (local_p)
        {
            con << CommandText()
                << "Rebuilding player '"
                << ItemText()
                << local_p->Name()
                << MainText() << "'\n";

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
            con << CommandText()
                << ErrorText()
                << "Player not found or is spectating.\n";
            return true;
        }
        else if (targetPlayer != nullptr && localPlayer->watchPlayer == targetPlayer)
        {
            con << CommandText()
                << ErrorText()
                << "Player '"
                << targetPlayer->GetColoredName()
                << MainText()
                << "' "
                << "already set to the watch player.\n";
            return true;
        }
        else
        {
            con << CommandText()
                << "Watch Player set to '"
                << targetPlayer->GetColoredName()
                << MainText() << "'\n";
            localPlayer->watchPlayer = targetPlayer;
        }
    }
    else
    {
        con << CommandText() << "Watch Player removed.\n";
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
    listInfo << CommandText()
             << "Status for " << p->GetColoredName()
             << MainText() << ":\n"
             << "Created: "
             << ItemText() << getTimeStringBase(p->createTime_)
             << MainText() << "\n"
             << "Last Activity: "
             << ItemText() << st_GetFormatTime(p->LastActivity(), true)
             << MainText() << "\n"
             << "Chatting For: "
             << ItemText() << st_GetFormatTime(chattingTime, true)
             << MainText() << "\n";


    gCycle *cycle = p->NetPlayerToCycle();
    if (cycle)
    {
        if (!cycle->Alive() && cycle->lastDeathTime > 0)
        {
            listInfo << "Last Death: "
                     << ItemText() << st_GetFormatTime(tSysTimeFloat() - cycle->lastDeathTime, true)
                     << "\n";
        }
        else
        {
            listInfo << "Alive Time: "
                     << ItemText() << st_GetFormatTime(se_GameTime(), true)
                     << "\n";
        }
    }
    else if (p->CurrentTeam())
    {
        listInfo << "Last Death: "
                    << ItemText() << st_GetFormatTime(tSysTimeFloat() - p->lastCycleDeathTime, true)
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
        con << CommandText()
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
        con << CommandText()
            << ErrorText()
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
        player->spectate = false;

    if (netPlayer && !bool(netPlayer->CurrentTeam()))
    {
        con << CommandText()
            << "Joining the game...\n";

        netPlayer->CreateNewTeamWish();
        netPlayer->ForcedUpdate();
    } 
    else if (netPlayer)
    {
        con << CommandText()
            << ErrorText()
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
        output << CommandText()
               << "Available files to search:\n"
               << MainText();

        int i = 1;
        for (const auto &searchableFile : searchableFiles)
        {
            output << i << ") "
                   << ItemText()
                   << searchableFile.first
                   << MainText()
                   << " (" << searchableFile.second
                   << ")\n";
            i++;
        }

        output << CommandText()
               << "Uses: \n"
               << MainText()
               << ItemText() << "/search chat hack the planet (by search phrase)\n"
               << ItemText() << "/search chat #102 (by line number)\n"
               << ItemText() << "/search chat #102-105 (by line number range)\n"
               << ItemText() << "/search chat #102 copy (copy text by single line number)\n"
               << ItemText() << "/search chat @5 copy (copy match #5 from last search)\n"
               << ItemText() << "/search chat @5 (copy match #5 from last search)\n";

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
            con << CommandText()
                << ErrorText()
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
            con << CommandText()
                << ErrorText()
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
            fileNameOut << CommandText()
                        << "\nFile '" << ItemText() << fileName      << MainText()
                        << "- "       << ItemText() << fileSizeMB    << MainText()
                        << " MB / "   << ItemText() << fileSizeMaxMB << MainText() << " MB\n";

            con << fileNameOut << MainText()
                << "Nothing to search. Showing last " << ItemText()
                << se_searchCommandEmptySearchNumLines
                << MainText() << " lines:\n";

            for (int i = start; i < lines.Len(); ++i)
            {
                netPlayer->lastSearch.Add(lines[i]);
                con << ItemText()   << count++
                    << HeaderText() <<  ") " << HeaderText()
                    << "Line " << MainText() << (i + 1)
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
                    con << CommandText()
                        << ErrorText()
                        << "Nothing to copy from.\n";
                }
                else
                {
                    tString message(netPlayer->lastSearch[endLineNumber - 1]);
                    if (copyToClipboard(message))
                    {
                        con << CommandText()
                            << "Copied content to clipboard.";
                        numMatches++;
                        output << endLineNumber << " " << MainText() << message << "\n";
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
                        output << MainText()
                               << numMatches << ") "
                               << HeaderText()
                               << "Line " << ItemText() << lineNumber << MainText()
                               << ": " << line << "\n";

                        if (copy && startLineNumber == endLineNumber) // only copy for one line
                        {
                            tString lineToCopy;
                            lineToCopy = output.SubStr(output.StrPos(": ") + 2); // Remove everything before the line content

                            if (copyToClipboard(lineToCopy))
                                con << HeaderText()
                                    << "Copied content to clipboard.";
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
                    con << CommandText()
                        << ErrorText()
                        << "Line Range: " << ItemText()
                        << startLineNumber << "-" << endLineNumber
                        << ErrorText() << " not found.\n";
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
                    output << numMatches << ") " << HeaderText() << "Line "
                           << ItemText() << lineNumber
                           << MainText()
                           << ": "       << line << "\n";
                    netPlayer->lastSearch.Add(line);
                }
                lineNumber++;
            }
        }

        if (!found && !copy)
        {
            tString fileNameOut;
            fileNameOut << "\n"     << HeaderText()
                        << "File '" << ItemText() << fileName      << MainText()
                        << "- "     << ItemText() << fileSizeMB    << MainText()
                        << " MB / " << ItemText() << fileSizeMaxMB << MainText() << " MB\n";

            con << CommandText()
                << fileNameOut
                << "No matches found for the search phrase: '" << ItemText()
                << searchPhrase << MainText() << "'\n";
        }
        else
        {
            tString fileNameOut;
            fileNameOut << HeaderText()
                        << "\nFile '" << ItemText() << fileName << MainText()
                        << "- " << ItemText() << fileSizeMB << MainText()
                        << " MB / " << ItemText() << fileSizeMaxMB << MainText() << " MB\n";

            tString matches;
            matches << fileNameOut << "Found "
                    << ItemText() << numMatches
                    << MainText() << " matches for: ";

            output = CommandText()
                     << matches << tString("'") << ItemText()
                     << searchPhrase << MainText() << tString("'\n")
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
        con << CommandText()
            << ErrorText()
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
        con << CommandText()
            << ErrorText()
            << "No usable players!\n";
        ePlayerNetID::nameSpeakWords.Clear();
        return true;
    }

    con << CommandText()
        << "\n  - Using Player '"
        << ItemText() << ePlayerNetID::nameSpeakPlayerID + 1
        << MainText()
        << "'. Message: '" << ItemText() << args
        << MainText() << "'\n";

    ePlayerNetID::nameSpeakForceUpdate = true;
    ePlayerNetID::nameSpeakCheck = true;
    ePlayerNetID::nameSpeakIndex = 0;
    ePlayerNetID::nameSpeakPlayerID = playerID;
    ePlayerNetID::playerUpdateIteration = 0;

    return true;
}

bool RespawnCommand::execute(tString args)
{
    con << CommandText()
        << "Respawning player '" << netPlayer->GetName() << "'\n";
    netPlayer->RespawnPlayer(true);
    return true;
}

bool RebuildGridCommand::execute(tString args)
{
    tArray<tString> passedString = args.Split(" ");
    if (args.empty())
    {
        con << CommandText()
            << ErrorText()
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
        con << CommandText()
            << ErrorText()
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
        con << CommandText()
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
            con << CommandText()
                << target->coloredName_
                << MainText()
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

            con << CommandText()
                << target->coloredName_
                << MainText()
                << " is now nicknamed '" << target->GetColoredName()
                << MainText() << "'\n";

            return true;
        }
    }

    con << CommandText()
        << ErrorText()
        << "Could not find player '" << ItemText()
        << args << ErrorText() << "'\n";
    return false;
}

bool StatsCommand::execute(tString args)
{

    nServerInfoBase *connectedServer = CurrentServer();
    if (connectedServer)
        con << CommandText()
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
        con << CommandText()
            << ErrorText()
            << "PlayerStats are disabled!\n";
        return true;
    }

    PlayerData playerData = ePlayerStats::getStats(p->GetName());

    tColoredString statsInfo;
    statsInfo << CommandText()
              << "\nStats for " << ItemText() << p->GetColoredName() << "\n"
              << MainText()
              << "Kills: " << ItemText() << playerData.kills << "\n"
              << MainText()
              << "Deaths: " << ItemText() << playerData.deaths << "\n"
              << MainText()
              << "Match Wins: " << ItemText() << playerData.match_wins << "\n"
              << MainText()
              << "Match Losses: " << ItemText() << playerData.match_losses << "\n"
              << MainText()
              << "K/D Ratio: " << ItemText() << playerData.getKDRatio() << "\n";

    con << statsInfo;
    return true;
}

bool ReconnectCommand::execute(tString args)
{
    nServerInfoBase *connectedServer = CurrentServer();
    if (connectedServer)
    {
        con << CommandText()
            << "Reconnecting to '" << connectedServer->GetName() << "'...\n";

    if(ePlayer::PlayerIsInGame(player->ID()))
        ret_to_MainMenu();

        ConnectToServer(connectedServer);
    }
    else
    {
        if (!ConnectToLastServer())
            con << CommandText()
                << ErrorText()
                << "You are not connected to a server!\n";
    }
    return true;
}

#include <stack>
#include <queue>
bool CalculateCommand::execute(tString args)
{
    con << CommandText()
        << "Performing calculation: '" << ItemText()
        << args << MainText() << "'\n";
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
                con << ErrorText()
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
                    con << ErrorText()
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
        con << ErrorText()
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

con << MainText()
    << "Result: '" << ItemText()
    << str
    << MainText() << "'\n";

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
        con << CommandText()
            << "Updating all players...\n";
        ePlayerNetID::ForcedUpdate();
    }
    else
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(PlayerNumb) - 1);
        if (local_p)
        {
            con << CommandText()
                << "Updating player '"
                << ItemText()
                << local_p->Name()
                << MainText() << "'\n";

            ePlayerNetID::Update(local_p);
        }
        else
        {
            con << CommandText()
                << ErrorText()
                << "No player for ID '"
                << ItemText()
                << PlayerNumb
                << ErrorText()
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
        for (int i = MAX_PLAYERS - 1; i >= 0; i--)
        {
            ePlayer *local_p = ePlayer::PlayerConfig(i);
            if (local_p && local_p->netPlayer)
            {
                se_NewChatMessage(local_p->netPlayer, messageToSend)->BroadCast();
                break;
            }
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

        con << CommandText()
            << "Creating hash at time: '"
            << ItemText()
            << currentTime
            << MainText() << "'\n";

        tString hash = GenerateHash(currentTime);

        tString messageToSend;
        messageToSend << "/msg " << name << " " << se_encryptCommandPrefix << hash;

        if (pos < args.Len())
            messageToSend << args.SubStr(pos);

        se_NewChatMessage(netPlayer, messageToSend)->BroadCast();
    }
    else
        con << CommandText()
            << ErrorText()
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
    con << CommandText() << "Polls:\n";

    if (eVoter::ChatDisplayVotes())
    {
        con << "0x888888 - Example Usage: (/vote ID yes)" << MainText() << "\n";
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
    con << CommandText()
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

    con << CommandText()
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
    
    con << CommandText()
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
        con << CommandText()
            << "Disconnecting and loading bookmark menu.\n";
        gServerFavorites::FavoritesMenuForceQuery(true);
    }
    else
    {
        con << CommandText()
            << "Loading bookmark menu. (Polling a server will disconnect you from the game).\n";
        gServerFavorites::FavoritesMenuForceQuery(false);
    }
    return true;
}
