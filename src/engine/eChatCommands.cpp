#include "eChatCommands.h"

#include "ePlayer.h"
#include "tRecorder.h"
#include "eFloor.h"
#include "rConsole.h"
#include "nServerInfo.h"
#include "../tron/gCycle.h"
#include "../tron/gServerBrowser.h"
#include "../tron/gGame.h"
#include "../tron/gHelper/gHelperUtilities.h"
#include "../tron/gMenus.h"

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

std::unordered_map<tString, std::function<std::unique_ptr<ChatCommand>()>> CommandFactory()
{
    std::unordered_map<tString, std::function<std::unique_ptr<ChatCommand>()>> commandFactories;

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

    return commandFactories;
}

bool LocalChatCommands(ePlayer *player, tString args, const std::unordered_map<tString, std::function<std::unique_ptr<ChatCommand>()>> &commandFactories)
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
    ePlayerNetID *msgTarget = ePlayerNetID::GetPlayerByName(args.ExtractNonBlankSubString(pos), false);
    if (msgTarget)
        netPlayer->lastMessagedPlayer = msgTarget;

    tString messageToSend;
    messageToSend << "/msg " << args;
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

tColoredString ColorsCommand::cycleColorPreview(int r, int g, int b)
{
    r /= 15;
    g /= 15;
    b /= 15;

    tColoredString cyclePreview;
    REAL sr = r, sg = g, sb = b;
    while (sr > 1.f)
        sr -= 1.f;
    while (sg > 1.f)
        sg -= 1.f;
    while (sb > 1.f)
        sb -= 1.f;
    cyclePreview << tColoredString::ColorString(r, g, b) << "<" << tColoredString::ColorString(sr, sg, sb) << "==" << ChatCommand::MainText();
    return cyclePreview;
}

tColoredString ColorsCommand::localPlayerPreview(ePlayer *local_p)
{
    int r = local_p->rgb[0];
    int g = local_p->rgb[1];
    int b = local_p->rgb[2];

    tColoredString output;
    output << tColoredString::ColorString(r, g, b)
           << local_p->Name()
           << ChatCommand::MainText()
           << " ("
           << ChatCommand::ItemText() << r << ChatCommand::MainText() << ", "
           << ChatCommand::ItemText() << g << ChatCommand::MainText() << ", "
           << ChatCommand::ItemText() << b << ChatCommand::MainText() << ") "
           << cycleColorPreview(r, g, b);

    return output;
}

tColoredString ColorsCommand::gatherPlayerColor(ePlayerNetID *p, bool showReset)
{
    tColoredString listColors, cyclePreview;

    if (showReset)
        listColors << p->GetColoredName() << ChatCommand::MainText() << " (";
    else
        listColors << p->GetColoredName().StripWhitespace() << ChatCommand::MainText() << " (";

    listColors << ChatCommand::ItemText() << p->r << ChatCommand::MainText() << ", "
               << ChatCommand::ItemText() << p->g << ChatCommand::MainText() << ", "
               << ChatCommand::ItemText() << p->b << ChatCommand::MainText() << ") "
               << cycleColorPreview(p->r, p->g, p->b);

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
            con << CommandText()
                << tOutput("$player_info_text")
                << "\n";
            ePlayerNetID *p = se_GetLocalPlayer();
            con << gatherPlayerInfo(p);
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
    tColoredString listinfo;
    listinfo << ChatCommand::MainText() << "Results for " << p->GetColoredName() << ChatCommand::MainText() << ":\n"
             << ChatCommand::MainText() << "Color: " << ColorsCommand::gatherPlayerColor(p) << "\n";

    gRealColor color(p->r, p->g, p->b);
    // p->Color(color);
    se_MakeColorValid(color.r, color.g, color.b, 1.0f);
    se_removeDarkColors(color);
    listinfo << ChatCommand::MainText()
             << "Filtered Color: " << ChatCommand::MainText() << "("
             << ChatCommand::ItemText() << color.r << ChatCommand::MainText() << ", "
             << ChatCommand::ItemText() << color.g << ChatCommand::MainText() << ", "
             << ChatCommand::ItemText() << color.b << ChatCommand::MainText() << ")\n";

    // Status. Includes player type, spectating or playing, and if the player is chatting.
    listinfo << ChatCommand::MainText()
             << "Status: " << ChatCommand::ItemText() << (p->IsHuman() ? "Human" : "Bot")
             << ChatCommand::MainText()
             << ", " << ChatCommand::ItemText()
             << (p->CurrentTeam() ? "Playing" : "Spectating")
             << (p->IsChatting() ? (ChatCommand::MainText() << "," << ChatCommand::ItemText() << " Chatting") : "");

    // Only grab this information if the player is an active object.
    if (p->Object() && p->CurrentTeam())
    {
        // If the player is an active object, are they alive?
        listinfo << (p->Object()->Alive() ? (ChatCommand::MainText() << "," << ChatCommand::ItemText() << " Alive") : ", Dead") << '\n'
                 << ChatCommand::MainText() << "Lag: " << ChatCommand::ItemText() << p->Object()->Lag() << ChatCommand::MainText() << "\n";

        // Only grab this information if the player is an alive object.
        gCycle *pCycle = dynamic_cast<gCycle *>(p->Object());
        if (p->Object()->Alive())
        {

            listinfo << ChatCommand::MainText()
                     << "Position: x: " << ChatCommand::ItemText() << pCycle->Position().x << ChatCommand::MainText()
                     << ", y: " << ChatCommand::ItemText() << pCycle->Position().y << ChatCommand::MainText() << '\n'
                     << "Map Direction: x: " << ChatCommand::ItemText() << pCycle->Direction().x << ChatCommand::MainText()
                     << ", y: " << ChatCommand::ItemText() << pCycle->Direction().y << ChatCommand::MainText() << '\n'
                     << "Speed: " << ChatCommand::ItemText() << pCycle->verletSpeed_ << ChatCommand::MainText() << '\n'
                     << "Rubber: " << ChatCommand::ItemText() << pCycle->GetRubber() << "/"
                     << sg_rubberCycle << ChatCommand::MainText() << '\n';
        }
    }

    return listinfo << '\n';
}

std::tuple<tString, int, int, int> RgbCommand::se_extractColorInfoFromLine(const tString &line)
{
    int pos = 0;
    tString Name = line.ExtractNonBlankSubString(pos);
    tString ColorOne = line.ExtractNonBlankSubString(pos);
    ColorOne.RemoveSubStr(0, 1);
    int Color1 = atoi(ColorOne);
    int Color2 = atoi(line.ExtractNonBlankSubString(pos));
    int Color3 = atoi(line.ExtractNonBlankSubString(pos));
    return std::make_tuple(Name, Color1, Color2, Color3);
}

void RgbCommand::se_outputColorInfo(int index, const tString &Name, REAL r, REAL g, REAL b)
{
    if (tColoredString::HasColors(Name))
    {
        con << (index + 1) << ") "
            << Name << ChatCommand::MainText()
            << " ("
            << ChatCommand::ItemText() << r << ChatCommand::MainText() << ", "
            << ChatCommand::ItemText() << g << ChatCommand::MainText() << ", "
            << ChatCommand::ItemText() << b << ChatCommand::MainText() << ") "
            << ColorsCommand::cycleColorPreview(r, g, b) << "\n";
    }
    else
    {
        con << (index + 1) << ") "
            << tColoredString::ColorString(r, g, b)
            << Name << ChatCommand::MainText()
            << " ("
            << ChatCommand::ItemText() << r << ChatCommand::MainText() << ", "
            << ChatCommand::ItemText() << g << ChatCommand::MainText() << ", "
            << ChatCommand::ItemText() << b << ChatCommand::MainText() << ") "
            << ColorsCommand::cycleColorPreview(r, g, b) << "\n";
    }
}

void RgbCommand::se_loadSavedColor(ePlayer *local_p, int lineNumber)
{
    tArray<tString> colors = FileManager(se_colorVarFile).Load();

    if (lineNumber < colors.Len())
    {
        auto [Name, r, g, b] = se_extractColorInfoFromLine(colors[lineNumber]);

        con << tOutput("$player_colors_loading");
        se_outputColorInfo(lineNumber, Name, r, g, b);

        local_p->rgb[0] = r;
        local_p->rgb[1] = g;
        local_p->rgb[2] = b;
    }
}

void RgbCommand::se_listSavedColors(int savedColorsCount)
{
    tArray<tString> colors = FileManager(se_colorVarFile).Load();
    con << tOutput("$players_color_list", savedColorsCount, se_colorVarFile);

    int index = 0;
    for (auto color : colors)
    {
        if (!color.empty())
        {
            auto [Name, r, g, b] = se_extractColorInfoFromLine(color);
            se_outputColorInfo(index++, Name, r, g, b);
        }
    }
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
        tArray<tString> passedString = args.Split(" ");
        FileManager fileManager(se_colorVarFile);
        tString command = passedString[0];
        bool correctParameters = false;

        if (command.isNumber() && !(passedString.Len() == 3 && passedString[2].isNumber()))
        {
            local_p = ePlayer::PlayerConfig(atoi(command) - 1);
            if (!local_p)
            {
                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_colors_changed_usage_error");
                return true;
            }
            targetPlayer = nullptr;
            passedString.RemoveAt(0);
            command = passedString[0];
        }

        if (command == "help")
        {
            con << CommandText() << "\n"
                << tOutput("$player_colors_command_help", se_colorVarFile);
            return true;
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
            if (passedString.Len() == 1)
            {
                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_colors_saved");
                tString playerColorStr;

                if (targetPlayer != nullptr)
                    playerColorStr = ColorsCommand::gatherPlayerColor(targetPlayer);
                else
                    playerColorStr = ColorsCommand::localPlayerPreview(local_p);

                if (fileManager.Write(playerColorStr.Replace(MainText(), "")))
                    con << CommandText()
                        << tOutput("$player_colors_saved")
                        << playerColorStr << "\n";
                else
                    con << CommandText()
                        << ErrorText()
                        << tOutput("$players_color_error");
            }
            else if (passedString.Len() == 2) // Save specific persons color
            {
                targetPlayer = ePlayerNetID::GetPlayerByName(passedString[1].Filter());
                if (targetPlayer)
                {
                    tString playerColorStr;
                    playerColorStr = ColorsCommand::gatherPlayerColor(targetPlayer);

                    if (fileManager.Write(playerColorStr.Replace(MainText(), "")))
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
                        << tOutput("$player_colors_not_found", passedString[1]);
            }
            return true;
        }
        else if (command == "load")
        {
            int savedColorsCount = fileManager.NumberOfLines();

            if (passedString.Len() == 1) // No Line #
            {
                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_colors_changed_usage_error");
            }
            else if (passedString.Len() == 2) // Line # specified
            {
                correctParameters = true;
                int lineNumber = (atoi(passedString[1]) - 1);
                if ((lineNumber >= 0) && lineNumber <= savedColorsCount - 1)
                    se_loadSavedColor(local_p, lineNumber);
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
                se_listSavedColors(savedColorsCount);
            else
                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_colors_empty");
            return true;
        }
        else if (command == "clear")
        {
            tString line = passedString[1];
            if (line.empty())
                correctParameters = false;
            else
            {
                fileManager.Clear(atoi(passedString[0]));
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
        else if (passedString.Len() == 3) // Apply color to player who sent command
        {
            correctParameters = true;
            local_p->rgb[0] = atoi(passedString[0]); // r
            local_p->rgb[1] = atoi(passedString[1]); // g
            local_p->rgb[2] = atoi(passedString[2]); // b
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
                ePlayerNetID::Update();
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

    if (targetPlayer && targetPlayer->pID != -1)
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
    else if (targetPlayer && targetPlayer->pID == -1)
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
            netPlayer->Update();
        }
    }
    return true;
}

bool WatchCommand::execute(tString args)
{
    if (!eGrid::CurrentGrid())
    {
        con << CommandText()
            << ErrorText()
            << "Must be called while a grid exists!\n";
        return false;
    }

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
        if (!targetPlayer || !targetPlayer->Object() ||
            !targetPlayer->CurrentTeam() ||
            (targetPlayer != nullptr && localPlayer->watchPlayer == targetPlayer))
        {
            con << CommandText()
                << ErrorText()
                << "Player not found or already set to the watch player.\n";
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
             << "\nResults for " << p->GetColoredName() << MainText()
             << "\nStatus: \n"
             << "Created: " << ItemText() << p->createTime_ << MainText() << "\n"
             << "Last Activity: " << ItemText()
             << p->LastActivity() << MainText() << "\n"
             << "Chatting For: " << ItemText() << chattingTime << MainText() << "\n";

    if (chattingTime == 0)
        listInfo << "Last chat activity: " << ItemText()
                 << p->ChattingTime() << MainText() << " seconds ago.\n";

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
    con << CommandText()
        << "Spectating player '" << netPlayer->GetName() << "'...\n";

    ePlayer *local_p = player;
    local_p->spectate = true;
    if (helperConfig::sghuk)
        netPlayer->Clear(local_p);
    netPlayer->Update();
    return true;
}

bool JoinCommand::execute(tString args)
{
    if (netPlayer && !bool(netPlayer->CurrentTeam()))
    {
        con << CommandText()
            << "Joining the game...\n";

        player->spectate = false;
        netPlayer->CreateNewTeamWish();
        netPlayer->Update();
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
            output << i << ") " << ItemText()
                   << searchableFile.first << MainText()
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

        FileManager fileManager(fileName,false);
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

#ifndef WIN32
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
                               << numMatches << ") " << HeaderText()
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
    if (netPlayer->lastMessagedPlayer == nullptr)
    {
        con << CommandText()
            << ErrorText()
            << "You have not messaged anyone yet!\n";
        return false;
    }

    tString messageToSend;
    messageToSend << "/msg " << netPlayer->lastMessagedPlayer->GetName() << " " << args;
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

            REAL r, g, b;
            target->Color(r, g, b);
            tColoredString coloredNickname;
            coloredNickname << tColoredString::ColorString(r, g, b) << nickname;
            target->coloredNickname = coloredNickname;
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

    PlayerStats *playerStats = PlayerStats::getInstance();
    if (!playerStats)
    {
        con << CommandText()
            << ErrorText()
            << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return true;
    }

    PlayerData playerData = playerStats->getStats(p->GetName());

    tColoredString statsInfo;
    statsInfo << CommandText()
              << "\nStats for " << ItemText() << p->GetColoredName() << "\n"
              << MainText()
              << "Kills: " << ItemText() << playerData.kills << "\n"
              << MainText()
              << "Deaths: " << ItemText() << playerData.deaths << "\n"
              << MainText()
              << "Wins: " << ItemText() << playerData.wins << "\n"
              << MainText()
              << "Losses: " << ItemText() << playerData.losses << "\n"
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

    con << MainText()
        << "Result: '" << ItemText()
        << values.top()
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
        ePlayerNetID::Update();
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
            con << local_p->Name() << " is not in game.\n";
        }
    }
    return true;
}

REAL EncryptCommand::getEncryptLocaltime()
{
    return getCurrentLocalTime()->tm_sec;
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
    int colonPos = message.StrPos(":");

    if (colonPos == -1)
    {
        return false;
    }

    tString actualMessage = message.SubStr(colonPos + 2);
    if (!actualMessage.StartsWith(se_encryptCommandPrefix))
    {
        return false;
    }

    int startIndex = actualMessage.StrPos(se_encryptCommandPrefix) + se_encryptCommandPrefix.Len() - 1;

    tString hashStr = actualMessage.SubStr(startIndex, se_encryptCommandLength);

    REAL currentTime = getEncryptLocaltime();
    bool valid = ValidateHash(hashStr, currentTime);

    if (!valid)
    {
        con << "EncryptCommand: Hash Invalid '" << hashStr << "' at time: " << currentTime << "\n";
        return false;
    }

    int argsStartIndex = startIndex + se_encryptCommandLength + 1;
    if (argsStartIndex < actualMessage.Len())
    {
        tString args = actualMessage.SubStr(argsStartIndex);
        con << "EncryptCommand: Loading command '" << args << "'\n";

        tCurrentAccessLevel level(tAccessLevel_Owner, true);
        std::stringstream s(static_cast<char const *>(args));
        tConfItemBase::LoadAll(s);

        if (se_encryptCommandWatchFeedback)
        {
            tString feedback;
            tString messageToSend;

            if (tConfItemBase::lastLoadOutput.empty())
                feedback << "Line loaded: '" << args << "'";
            else
                feedback << tConfItemBase::lastLoadOutput;

            messageToSend << "/msg " << player->GetName() << " " << feedback << "\n";
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
    else
    {
        con << "EncryptCommand: No arguments found after the hash.\n";
        return true;
    }
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
        {
            messageToSend << args.SubStr(pos);
        }

        se_NewChatMessage(netPlayer, messageToSend)->BroadCast();
    }
    else
    {
        con << CommandText()
            << ErrorText()
            << "Player not found for '"
            << args
            << "'\n";
    }

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
    {
        con << "Invalid command format.\n";
    }

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
        << "Renaming player '" << netPlayer->GetName() << "' to '" << args << "'...\n";

    player->name = args;
    netPlayer->Update();
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
    con << CommandText()
        << "Quiting game in '" << se_quitCommandTime << "' seconds...\n";

    st_SaveConfig();

    gTaskScheduler.schedule("QuitCommand", se_quitCommandTime, []
    {
        throw 1;
    });
    return true;
}
