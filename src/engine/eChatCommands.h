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
extern tString se_playerTriggerMessagesIgnoreList;

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
extern int     se_nameSpeakCommandInterval;
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


class ChatCommand
{
public:
    virtual bool execute(ePlayerNetID* player, tString args) = 0;

    static tString HeaderText() { return se_chatCommandsThemeHeader;}
    static tString MainText() { return se_chatCommandsThemeMain;}
    static tString ItemText() { return se_chatCommandsThemeItem;}
    static tString ErrorText() { return se_chatCommandsThemeError;}

    tString CommandText()
    {
        tString output;

        output << HeaderText()
               << commandName
               << " - "
               << MainText();
        return output;
    }

    const std::string& getCommandName() const {
        return commandName;
    }

protected:
    ChatCommand(const std::string& name) : commandName(name) {}

private:
    std::string commandName;
};

extern bool LocalChatCommands(ePlayerNetID *p, tString command);
extern bool LocalChatCommands(ePlayerNetID *p, tString command, std::unordered_map<tString, std::function<std::unique_ptr<ChatCommand>()>>& commandFactories);
extern bool RunChatCommand(ePlayerNetID *p, ChatCommand & command, tString args);


// Gather all the rgb colors and put them in a nice list.
// Usage /colors with no parameters returns all players and their colors.
//       /colors playername returns that specific players color or more depending if the search term is found in other player names
class ColorsCommand : public ChatCommand
{
public:
    ColorsCommand() : ChatCommand("ColorsCommand") {}

    static tColoredString cycleColorPreview(int r, int g, int b)
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

    static tColoredString localPlayerPreview(ePlayer *local_p)
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

    static tColoredString gatherPlayerColor(ePlayerNetID *p, bool showReset = true)
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

    bool execute(ePlayerNetID *player, tString args) override
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
class listPlayerInfoCommand : public ChatCommand
{
public:
    listPlayerInfoCommand() : ChatCommand("listPlayerInfoCommand") {}

    tColoredString gatherPlayerInfo(ePlayerNetID *p)
    {
        tColoredString listinfo;
        listinfo << ChatCommand::MainText() << "Results for " << p->GetColoredName()  << ChatCommand::MainText() << ":\n"
                << ChatCommand::MainText() << "Color: "      << ColorsCommand::gatherPlayerColor(p) << "\n";

        gRealColor color(p->r, p->g, p->b);
        // p->Color(color);
        se_MakeColorValid(color.r,color.g,color.b,1.0f);
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
                << (p->IsChatting() ? (ChatCommand::MainText() << "," <<  ChatCommand::ItemText() << " Chatting") : "");

        // Only grab this information if the player is an active object.
        if (p->Object() && p->CurrentTeam())
        {
            // If the player is an active object, are they alive?
            listinfo << (p->Object()->Alive() ? (ChatCommand::MainText() << "," <<  ChatCommand::ItemText() << " Alive"): ", Dead") << '\n'
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

    bool execute(ePlayerNetID *player, tString args) override
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
class RgbCommand : public ChatCommand
{
public:
    RgbCommand() : ChatCommand("RgbCommand") {}

    static std::tuple<tString, int, int, int> se_extractColorInfoFromLine(const tString &line)
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

    static void se_outputColorInfo(int index, const tString &Name, REAL r, REAL g, REAL b)
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

    static void se_loadSavedColor(ePlayer *local_p, int lineNumber)
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

    static void se_listSavedColors(int savedColorsCount)
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

    bool execute(ePlayerNetID *player, tString args) override
    {
        ePlayer *local_p = ePlayer::NetToLocalPlayer(player);
        ePlayerNetID *targetPlayer = player;
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
                    {
                        con << CommandText()
                            << ErrorText()
                            << tOutput("$player_colors_not_found", passedString[1]);
                    }
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
                {
                    correctParameters = false;
                }
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
                {
                    listColors << ColorsCommand::localPlayerPreview(local_p);
                }
                con << listColors << "\n";
            }
            else // display the error message.
                con << CommandText()
                    << ErrorText()
                    << tOutput("$player_colors_changed_usage_error");
        }
        return true;
    }
};


class EncryptCommand : public ChatCommand
{
public:
    EncryptCommand() : ChatCommand("EncryptCommand") {}

    static REAL getEncryptLocaltime()
    {
        return getCurrentLocalTime()->tm_sec;
    }

    static tString GenerateHash(double time)
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

    static bool ValidateHash(tString givenHash, double time)
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

    static bool handleEncryptCommandAction(ePlayerNetID *player, tString message)
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

        int startIndex = actualMessage.StrPos(se_encryptCommandPrefix) + se_encryptCommandPrefix.Len()-1;

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


    bool execute(ePlayerNetID *player, tString args) override
    {
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

            se_NewChatMessage(player, messageToSend)->BroadCast();
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
};


#endif
