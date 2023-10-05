#include "eChatBot.h"
#include "ePlayer.h"
#include "ePlayerStats.h"
#include "eTimer.h"
#include "../tron/gGame.h"

#include "eChatCommands.h"

#include <stack>
#include <queue>

bool se_playerTriggerMessages = false;
static tConfItem<bool> se_playerTriggerMessagesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS", se_playerTriggerMessages);

tString se_playerTriggerMessagesFile("chattriggers.txt");
static tConfItem<tString> se_playerTriggerMessagesFileConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FILE", se_playerTriggerMessagesFile);

tString se_playerTriggerMessagesIgnoreList("");
static tConfItem<tString> se_playerTriggerMessagesIgnoreListConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_IGNORE_LIST", se_playerTriggerMessagesIgnoreList);

bool se_playerTriggerMessagesReactToSelf = false;
static tConfItem<bool> se_playerTriggerMessagesReactToSelfConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_REACT_TO_SELF", se_playerTriggerMessagesReactToSelf);

static tString se_playerTriggerMessagesKillVerifiedTriggers = tString("wd,nice,wp,gj,$diedother,annoying,n1");
static tConfItem<tString> se_playerTriggerMessagesKillVerifiedTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_KILLED_VERIFIED_TRIGGERS", se_playerTriggerMessagesKillVerifiedTriggers);

static tString se_playerTriggerMessagesDiedByVerifiedTriggers("$died");
static tConfItem<tString> se_playerTriggerMessagesDiedByVerifiedTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DIED_BY_VERIFIED_TRIGGERS", se_playerTriggerMessagesDiedByVerifiedTriggers);

static tString se_playerMessageTargetPlayer = tString("");
static tConfItem<tString> se_playerMessageTargetPlayerConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_ENABLED_PLAYERS", se_playerMessageTargetPlayer);

static REAL se_playerMessageDelayRandMult = 0;
static tConfItem<REAL> se_playerMessageDelayRandMultConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY_RANDOM_MULT", se_playerMessageDelayRandMult);
static REAL se_playerMessageDelay = 0;
static tConfItem<REAL> se_playerMessageDelayConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY", se_playerMessageDelay);
static bool se_playerMessageSmartDelay = false;
static tConfItem<bool> se_playerMessageSmartDelayConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY_SMART", se_playerMessageSmartDelay);
static REAL se_playerMessageSmartDelayTypingWPM = 80;
static tConfItem<REAL> se_playerMessageSmartDelayTypingWPMConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY_SMART_TYPING_WPM", se_playerMessageSmartDelayTypingWPM);
static REAL se_playerMessageSmartDelayReadingWPM = 100;
static tConfItem<REAL> se_playerMessageSmartDelayReadingWPMConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY_SMART_READING_WPM", se_playerMessageSmartDelayReadingWPM);

static REAL se_playerMessageSmartDelayReactionTime = 0.25;
static tConfItem<REAL> se_playerMessageSmartDelayReactionTimeConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY_SMART_REACTION_TIME", se_playerMessageSmartDelayReactionTime);

static bool se_playerMessageChatFlag = false;
static tConfItem<bool> se_playerMessageChatFlagConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CHATFLAG", se_playerMessageChatFlag);

static REAL se_playerMessageChatFlagStartMult = 0.5;
static tConfItem<REAL> se_playerMessageChatFlagStartMultConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CHATFLAG_START_MULT", se_playerMessageChatFlagStartMult);

static bool se_playerMessageDisplayScheduledMessages = false;
static tConfItem<bool> se_playerMessageDisplayScheduledMessagesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SHOW_SCHEDULED_MESSAGES", se_playerMessageDisplayScheduledMessages);

static int se_playerMessageTriggersFuncAdderVal = 1;
static tConfItem<int> se_playerMessageTriggersFuncAdderValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_ADDER_ADD_VAL", se_playerMessageTriggersFuncAdderVal);

tString stripNonOperatorsOrNumbers(const tString &input);
bool containsMath(const tString &input, bool exact);

/* Chat Trigger Functions */
tString numberAdderFunc(tString message)
{
    std::string input(message.stdString());

    bool isNegative = input[0] == '-';
    size_t idx = isNegative ? 1 : 0;

    std::string intPart, fracPart;
    while (idx < input.size() && input[idx] != '.')
        intPart += input[idx++];
    if (idx < input.size() && input[idx] == '.')
    {
        idx++;
        while (idx < input.size())
            fracPart += input[idx++];
    }

    int carry = isNegative ? -se_playerMessageTriggersFuncAdderVal : se_playerMessageTriggersFuncAdderVal;

    for (int i = fracPart.size() - 1; i >= 0 && carry != 0; i--)
    {
        int sum = (fracPart[i] - '0') + carry;
        carry = sum < 0 ? -1 : sum / 10;
        fracPart[i] = '0' + (sum < 0 ? 10 + sum : sum % 10);
    }

    for (int i = intPart.size() - 1; i >= 0 && carry != 0; i--)
    {
        int sum = (intPart[i] - '0') + carry;
        carry = sum < 0 ? -1 : sum / 10;
        intPart[i] = '0' + (sum < 0 ? 10 + sum : sum % 10);
    }

    if (carry != 0)
        intPart = (carry == -1 ? "-" : "") + std::to_string(std::abs(carry)) + intPart;

    if (intPart == "0")
        isNegative = false;

    std::string result = isNegative ? "-" : "";
    result += intPart;
    if (!fracPart.empty())
        result += '.' + fracPart;

    return tString(result);
}

tString numberCalcFunc(tString message)
{
    tString args = stripNonOperatorsOrNumbers(message);
    std::stack<double> values;
    std::queue<tString> postfix = CalculateCommand::infixToPostfix(CalculateCommand::preprocess(args));

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
                return tString("");

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
                    return tString("");

                values.push(lhs / rhs);
            }
            else if (token == "^")
                values.push(std::pow(lhs, rhs));
        }
    }

    if (values.size() != 1)
        return tString("");

    std::ostringstream strs;
    strs << std::fixed << std::setprecision(6) << values.top();
    std::string str = strs.str();

    std::size_t lastNonZero = str.find_last_not_of('0');
    if (lastNonZero != std::string::npos)
    {
        if (str[lastNonZero] == '.')
        {
            str.erase(lastNonZero, str.length() - lastNonZero);
        }
        else
        {
            str.erase(lastNonZero + 1, str.length() - lastNonZero - 1);
        }
    }

    return tString(str);
}

tString sayFunc(tString message)
{
    tString input = message;

    if (!input.empty() && input[0] == '/')
    {
        tString paddedString(" ");
        paddedString << input;
        input = paddedString;
    }

    return input;
}

tString reverseFunc(tString message)
{
    tString input = message;

    input = input.Reverse();

    if (!input.empty() && input[0] == '/')
    {
        tString paddedString(" ");
        paddedString << input;
        input = paddedString;
    }

    return input;
}

tString statsFunc(tString message)
{
    tString output;

    if (!message.empty())
    {
        ePlayerNetID *statsTargetPlayer = ePlayerNetID::GetPlayerByName(message, false);

        if (statsTargetPlayer)
        {
            tString playerName = statsTargetPlayer->GetName();
            output << playerName
                   << ": "
                   << "Created: "
                   << getTimeStringBase(statsTargetPlayer->createTime_)
                   << " EST, "
                   << "Last Active: "
                   << st_GetFormatTime(statsTargetPlayer->LastActivity(), false)
                   << ", "
                   << "RGB: ("
                   << statsTargetPlayer->r << ", "
                   << statsTargetPlayer->g << ", "
                   << statsTargetPlayer->b << ")";

            if (se_playerStats)
            {
            
                PlayerData &stats = ePlayerStats::getStats(statsTargetPlayer);
                output << ", Chats: " 
                       << stats.total_messages
                       << ", Play Time "
                       << st_GetFormatTime(stats.getTotalPlayTime(),false);
            } 

            if (statsTargetPlayer->Object())
            {
                gCycle *pCycle = dynamic_cast<gCycle *>(statsTargetPlayer->Object());

                if (!pCycle->Alive() && pCycle->lastDeathTime > 0)
                    output << ", Last Death: "
                           << st_GetFormatTime(tSysTimeFloat() - pCycle->lastDeathTime, false);
                else
                    output << ", Alive Time: "
                           << st_GetFormatTime(se_GameTime(), false);
            }
            else if (statsTargetPlayer->CurrentTeam())
            {

                output << ", Last Death: "
                       << st_GetFormatTime(tSysTimeFloat() - statsTargetPlayer->lastCycleDeathTime, false)
                       << "\n";
            }

            return tString(output);
        }
    }

    eChatBotStats &stats = eChatBot::getInstance().Stats();

    output << "Uptime: " << st_GetFormatTime(stats.UpTime())
           << ", "
           << "Messages Sent: "       << stats.messagesSent
           << ", "
           << "Messages Read: "       << stats.messagesRead
           << ", "
           << "Last triggerered by "  << (stats.lastTriggeredBy != nullptr ? stats.lastTriggeredBy->GetName() : "?" );


    return tString(output);
}

tString nicknameFunc(tString message)
{
    tString output;
    tString newNickname = message;

    ePlayerNetID *lastTriggeredBy = eChatBot::getInstance().Stats().lastTriggeredBy;

    if (lastTriggeredBy == nullptr)
        return output;

    if (newNickname.empty() && !lastTriggeredBy->nickname.empty())
    {

            lastTriggeredBy->nickname.Clear();
            lastTriggeredBy->UpdateName();

            output << lastTriggeredBy->GetName()
                   << "'s nickname has been cleared.\n";
            return output;
    }
    else
    {
        lastTriggeredBy->nickname = newNickname;
        lastTriggeredBy->UpdateName();
        output << lastTriggeredBy->name_
                << " is now nicknamed '" << lastTriggeredBy->GetName()
                << "'\n";

        return output;
    }

    return tString(output);
}

tString playerKDFunc(tString message)
{
    tString output;

    ePlayerNetID *statsTargetPlayer = nullptr;
    tString playerName;

    if (!se_playerStats)
    {
        output << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return output;
    }

    if (!message.empty())
    {
        playerName = message;
        statsTargetPlayer = ePlayerNetID::GetPlayerByName(message, false);
        if (statsTargetPlayer)
            playerName = statsTargetPlayer->GetName();
    }
    else
    {
        eChatBot &bot = eChatBot::getInstance();
        statsTargetPlayer = bot.stats.lastTriggeredBy;
        if (statsTargetPlayer)
            playerName = statsTargetPlayer->GetName();
    }

    if (!playerName.empty())
    {
        PlayerData playerData = ePlayerStats::getStats(playerName);

        output << playerName << ": "
               << "Kills: "        << playerData.kills
               << ", "
               << "Deaths: "        << playerData.deaths
               << ", "
               << "K/D: "    << playerData.getKDRatio()
               << " | Rounds: played: " << playerData.rounds_played
               << ", "
               << "Wins: "   << playerData.round_wins
               << ", "
               << "Losses: " << playerData.round_losses
               << " | Matches: played: " << playerData.matches_played
               << ", "
               << "Wins: "   << playerData.match_wins
               << ", "
               << "Losses: " << playerData.match_losses;
    }
    else
    {
        output << "Player not found!";
    }

    return tString(output);
}


void eChatBot::InitChatFunctions()
{
    RegisterFunction(tString("$numbadderfunc"), numberAdderFunc);
    RegisterFunction(tString("$numbcalcfunc"), numberCalcFunc);
    RegisterFunction(tString("$sayfunc"), sayFunc);
    RegisterFunction(tString("$revfunc"), reverseFunc);
    RegisterFunction(tString("$statsfunc"), statsFunc);
    RegisterFunction(tString("$nicknamefunc"), nicknameFunc);
    RegisterFunction(tString("$KDfunc"), playerKDFunc);
}

void eChatBot::LoadChatTriggers()
{
    FileManager fileManager(se_playerTriggerMessagesFile, tDirectories::Var());
    tArray<tString> lines = fileManager.Load();

    chatTriggerKeys.clear();
    chatTriggers.clear();
    for (int i = 0; i < lines.Len(); ++i)
    {
        if (lines[i].StartsWith("\"") && lines[i].EndsWith("\""))
        {
            lines[i] = lines[i].SubStr(1, lines[i].Len() - 2);
        }

        tArray<tString> parts = lines[i].Split(",");
        if (parts.Len() == 4)
        {
            // Split the triggers by semicolon
            tArray<tString> triggersArray = parts[0].Split(";");
            tArray<tString> responsesArray = parts[1].Split(";");

            for (auto &trigger : triggersArray)
            {
                trigger = trigger.ToLower();

                // Split the responses by semicolon
                std::vector<tString> responses(responsesArray.begin(), responsesArray.end());

                REAL extraDelay = atof(parts[2]);
                bool exact = atoi(parts[3]) == 1;

                // Store the trigger and its corresponding responses, delay and exact value
                chatTriggers[trigger] = std::make_tuple(responses, extraDelay, exact);

                // store order
                chatTriggerKeys.push_back(trigger);
            }
        }
        else
        {
            con << "LoadChatTriggers Error:\nMalformed line at index " << i + 1 << ": " << lines[i] << "\n";
            continue;
        }
    }
}

void eChatBot::InitiateAction(ePlayerNetID *triggeredByPlayer, tString message, bool eventTrigger, tString preAppend)
{
    eChatBot &bot = eChatBot::getInstance();

    if (!bot.ShouldAnalyze())
        return;

    bot.Stats().messagesRead++;

    auto [response, delay, sendingPlayer] = bot.findTriggeredResponse(triggeredByPlayer, message, eventTrigger);

    if (!response.empty()){
        tString output;
        output << preAppend
               << response;
        bot.preparePlayerMessage(output, delay, sendingPlayer);
    }
    else if (eventTrigger)
        con << "No trigger set for '" << message << "'\nSet one with 'PLAYER_MESSAGE_TRIGGERS_ADD'\n";
}

bool eChatBot::ShouldAnalyze()
{
    tArray<tString> players = se_playerMessageTargetPlayer.Split(",");
    for (int i = 0; i < players.Len(); i++)
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[0]) - 1);
        if (!local_p)
            continue;

        ePlayerNetID *netPlayer = local_p->netPlayer;
        if (!netPlayer)
            continue;

        return true;
    }
    return false;
}

REAL eChatBot::determineReadingDelay(tString message)
{
    return se_playerMessageSmartDelayReadingWPM > 0 ? calculateResponseSmartDelay(message, se_playerMessageSmartDelayReadingWPM) : 0;
}

REAL eChatBot::calculateResponseSmartDelay(tString response, REAL wpm)
{
    REAL delay = 0;

    int chatLen = response.Len();
    REAL delayPerChar = 60.0 / (5 * wpm); // in seconds
    delay = chatLen * delayPerChar;
    return delay;
}

void eChatBot::scheduleMessageTask(ePlayerNetID *netPlayer, tString message, bool chatFlag, REAL totalDelay, REAL flagDelay)
{
    gTaskScheduler.enqueueChain([=]() { // group Tasks
        float messageDelay = totalDelay - flagDelay;

        gTaskScheduler.schedule(
            "playerMessageTask", flagDelay, [netPlayer, message, chatFlag, messageDelay] // When we start typing
            {
                if (chatFlag)
                {
                    netPlayer->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, true);
                    ePlayerNetID::Update();

                    gTaskScheduler.schedule("playerMessageSetChatFlagFalse", messageDelay, [netPlayer, message, chatFlag] // When we send the message and stop typing
                    {
                        netPlayer->Chat(message);
                        if (chatFlag)
                            netPlayer->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, false);
                        ePlayerNetID::Update();
                    });
                }
                else
                {
                    netPlayer->Chat(message);
                } });                                                                        // multiple messages will be added to pendingTasks
    });
}

/*
triggeredPlayer: who triggered the message
chatMessage: the message potentially containing a trigger
*/
std::tuple<tString, REAL, ePlayerNetID *> eChatBot::findTriggeredResponse(ePlayerNetID *triggeredByPlayer, tString chatMessage, bool eventTrigger)
{
    tString lowerMessage(chatMessage.TrimWhitespace());

    if (chatMessage.empty())
        return std::make_tuple(tString(""), 0.0, nullptr);

    tToLower(lowerMessage);
    tString triggeredByPlayerName = tString("");
    ePlayerNetID *sendingPlayer = nullptr; // who should send this message?

    Stats().lastTriggeredBy = triggeredByPlayer;

    for (const auto &triggerKey : chatTriggerKeys)
    {
        auto triggerPair = chatTriggers.find(triggerKey);

        if (triggerPair == chatTriggers.end())
            continue;

        bool exact = std::get<2>(triggerPair->second), match = false;

        if (eventTrigger)
            exact = true;

        tString trigger = triggerPair->first.ToLower();

        ePlayerNetID *potentialSender = nullptr;

        if (trigger.Contains("$p"))
        {
            for (int i = MAX_PLAYERS - 1; i >= 0; i--)
            {
                ePlayer *p = ePlayer::PlayerConfig(i);

                if (p)
                {
                    ePlayerNetID *netp = p->netPlayer;

                    if (netp)
                    {
                        tString ourName = netp->GetName().ToLower();
                        tArray<tString> nameParts = ourName.Split(" ");

                        tString triggerWithoutName = trigger.Replace("$p", "").TrimWhitespace();
                        if (!lowerMessage.Contains(triggerWithoutName))
                            continue;

                        bool nameFound = false;
                        if (exact)
                        {
                            if (lowerMessage == ourName)
                                nameFound = true;
                            else
                            {
                                for (const auto &namePart : nameParts)
                                {
                                    if (lowerMessage == namePart)
                                    {
                                        nameFound = true;
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            for (const auto &namePart : nameParts)
                            {
                                if (lowerMessage.Contains(namePart))
                                {
                                    nameFound = true;
                                    break;
                                }
                            }
                        }

                        if (nameFound)
                        {
                            match = true;
                            potentialSender = netp;
                            break;
                        }
                    }
                }
            }
        }
        else if (trigger.StartsWith("$"))
        {
            if (trigger == "$number")
                match = exact ? lowerMessage.isNumber() : lowerMessage.containsNumber();
            else if (trigger == "$math")
                match = containsMath(lowerMessage, exact);
            else
                match = (exact && lowerMessage == trigger) || (!exact && lowerMessage.Contains(trigger));
        }
        else
        {
            match = (exact && lowerMessage == trigger) || (!exact && lowerMessage.Contains(trigger));
        }

        if (match)
        {
            Stats().lastMatchedTrigger = trigger;
            // Determine the sending player based on the type of trigger
            if (triggeredByPlayer != nullptr)
            {
                if (potentialSender == nullptr)
                {
                    if (tIsInList(se_playerTriggerMessagesDiedByVerifiedTriggers, trigger))
                    {
                        if (triggeredByPlayer->lastKilledPlayer == nullptr || !triggeredByPlayer->lastKilledPlayer->isLocal())
                            continue;
                        else
                            potentialSender = triggeredByPlayer->lastKilledPlayer;
                    }
                    else if (tIsInList(se_playerTriggerMessagesKillVerifiedTriggers, trigger))
                    {
                        if (triggeredByPlayer->lastDiedByPlayer == nullptr || !triggeredByPlayer->lastDiedByPlayer->isLocal())
                            continue;
                        else
                            potentialSender = triggeredByPlayer->lastDiedByPlayer;
                    }

                    if (potentialSender != nullptr && potentialSender->isLocal())
                        sendingPlayer = potentialSender;
                }
                triggeredByPlayerName = triggeredByPlayer->GetName();
            }

            REAL extraDelay = std::get<1>(triggerPair->second);

            // Vector of possible responses
            std::vector<tString> possibleResponses = std::get<0>(triggerPair->second);

            // Random response from the vector
            tString chosenResponse = possibleResponses[rand() % possibleResponses.size()];

            tString responseStr(chosenResponse);

            responseStr = responseStr.Replace("$p",triggeredByPlayerName);

            int dollarPos = 0;
            while ((dollarPos = chosenResponse.StrPos(dollarPos, "$")) != -1)
            {
                int endPos = chosenResponse.StrPos(dollarPos + 1, " ");
                bool keepSpace = false;

                if (endPos != -1)
                    keepSpace = true;
                else
                    endPos = chosenResponse.Len();

                tString functionName = chosenResponse.SubStr(dollarPos, endPos - dollarPos);

                if (!functionName.StartsWith("$p"))
                {
                    tString functionInput(chatMessage);
                    tString toReplace = Stats().lastMatchedTrigger + " ";

                    functionInput = chatMessage.Replace(toReplace,"");

                    toReplace = Stats().lastMatchedTrigger;

                    functionInput = functionInput.Replace(toReplace,"");

                    tString result = ExecuteFunction(functionName, functionInput);

                    if (!result.empty())
                    {
                        responseStr = responseStr.Replace(functionName, result);
                    }
                    else
                        responseStr = "";
                }

                dollarPos = endPos;
            }

            chosenResponse = responseStr;

            return std::make_tuple(chosenResponse, extraDelay, sendingPlayer);
        }
    }

    // Return empty message and 0 delay if no triggers match
    return std::make_tuple(tString(""), 0.0, nullptr);
}

void eChatBot::preparePlayerMessage(tString messageToSend, REAL extraDelay, ePlayerNetID *player)
{
    if (!helperConfig::sghuk)
        return;

    REAL totalDelay;

    if (se_playerMessageSmartDelay)
        extraDelay += calculateResponseSmartDelay(messageToSend, se_playerMessageSmartDelayTypingWPM) + se_playerMessageSmartDelayReactionTime;

    if (se_playerMessageDelay > 0)
        extraDelay += se_playerMessageDelay;

    if (se_playerMessageDelayRandMult > 0)
        extraDelay += (REAL)rand() / RAND_MAX * se_playerMessageDelayRandMult;

    totalDelay += extraDelay;

    REAL flagDelay = totalDelay * se_playerMessageChatFlagStartMult;

    if (flagDelay > totalDelay)
        flagDelay = totalDelay * 0.9;

    bool scheduled = false;

    if (player != nullptr)
    {
        if (tIsInList(se_playerMessageTargetPlayer, player->pID + 1))
        {
            scheduled = true;
            scheduleMessageTask(player, messageToSend, se_playerMessageChatFlag, totalDelay, flagDelay);
        }
    }
    else
    {
        tArray<tString> players = se_playerMessageTargetPlayer.Split(",");
        for (int i = 0; i < players.Len(); i++)
        {
            ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[0]) - 1);
            if (!local_p)
                continue;

            ePlayerNetID *netPlayer = local_p->netPlayer;
            if (!netPlayer)
                continue;

            scheduled = true;
            scheduleMessageTask(netPlayer, messageToSend, se_playerMessageChatFlag, totalDelay, flagDelay);
        }
    }

    if (scheduled)
        Stats().messagesSent++;

    if (se_playerMessageDisplayScheduledMessages && scheduled)
        con << "Scheduled message '" << messageToSend << "' "
            << flagDelay             << " (wait time) + "
            << totalDelay-flagDelay  << " (typing time) -> "
            << totalDelay            << " (total delay)."
            << " (ExtraTime: "       << extraDelay
            << " seconds.)\n";
}

bool containsMath(const tString &input, bool exact)
{
    int digitsCount = 0;
    int operatorsCount = 0;
    bool negativeSign = false;

    for (int i = 0; i < input.Len(); ++i)
    {
        char c = input(i);

        if (c == '\0')
            continue;

        if (std::isdigit(static_cast<unsigned char>(c)))
        {
            digitsCount++;
            if (i > 0 && input(i - 1) == '-' && !negativeSign)
            {
                operatorsCount++;
            }
        }
        else if (c == '+' || c == '*' || c == '/' || c == '^')
        {
            operatorsCount++;
        }
        else if (c == '-')
        {
            negativeSign = true;
            if (i > 0 && std::isdigit(static_cast<unsigned char>(input(i - 1))))
            {
                operatorsCount++;
            }
        }
        else if (exact)
        {
            return false;
        }

        if (digitsCount >= 2 && operatorsCount >= 1)
            return true;
    }

    return false;
}

tString stripNonOperatorsOrNumbers(const tString &input)
{
    tString result;
    for (int i = 0; i < input.Len(); ++i)
    {
        char c = input(i);

        if (c == '\0')
            continue;

        if (std::isdigit(static_cast<unsigned char>(c)) || (c == '+' || c == '-' || c == '.' || c == '*' || c == '/' || c == '^'))
            result += c;
    }
    return result;
}

static void AddChatTrigger(std::istream &s)
{
    tString params;
    params.ReadLine(s, true);

    if (params.empty())
    {
        con << "Usage: PLAYER_MESSAGE_TRIGGERS_ADD <trigger>,<response>,<extraDelay>,<exact>\n";
        return;
    }

    tArray<tString> parts = params.Split(",");

    if (parts.Len() != 4)
    {
        con << "Invalid input. Usage: PLAYER_MESSAGE_TRIGGERS_ADD <trigger>,<response>,<extraDelay>,<exact>\n";
        return;
    }

    tArray<tString> triggersArray = parts[0].Split(";");

    eChatBot &bot = eChatBot::getInstance();
    FileManager fileManager(se_playerTriggerMessagesFile, tDirectories::Var());

    for (auto &trigger : triggersArray)
    {
        trigger = trigger.ToLower();

        tArray<tString> responsesArray = parts[1].Split(";");
        std::vector<tString> responses(responsesArray.begin(), responsesArray.end());

        REAL extraDelay = atof(parts[2].c_str());
        bool exact = atoi(parts[3].c_str()) == 1;

        if (trigger.empty() || responses.empty())
        {
            con << "Error: Trigger and responses cannot be empty.\n";
            return;
        }

        bot.chatTriggers[trigger] = std::make_tuple(responses, extraDelay, exact);
        fileManager.Write(params);

        con << "Trigger, Response, Extra Delay, Exact?\n";
        con << "Added: " << params << "\n";
    }
}

static void RemoveChatTrigger(std::istream &s)
{
    tString params;
    params.ReadLine(s, true);

    if (params.empty())
    {
        con << "Usage: PLAYER_MESSAGE_TRIGGERS_REMOVE <line_number>\n";
        return;
    }

    FileManager fileManager(se_playerTriggerMessagesFile, tDirectories::Var());

    int lineNumber = atoi(params.c_str()) - 1;

    if (fileManager.Clear(lineNumber))
        con << "Removed line " << lineNumber + 1 << "\n";
    eChatBot &bot = eChatBot::getInstance();
    bot.chatTriggers.clear();
    bot.LoadChatTriggers();
}

static void ListChatTriggers(std::istream &s)
{
    eChatBot &chatBot = eChatBot::getInstance();

    if (!chatBot.chatTriggers.empty())
    {
        con << "Listing all loaded chat triggers:\n";
        con << "Line) Trigger, Response, Extra Delay, Exact?\n";

        int i = 1; // Line number counter
        for (const auto &triggerPair : chatBot.chatTriggers)
        {
            const tString &trigger = triggerPair.first;
            const auto &details = triggerPair.second;
            const std::vector<tString> &responses = std::get<0>(details);
            REAL extraDelay = std::get<1>(details);
            bool exact = std::get<2>(details);

            // combine multiple responses into a single string separated by semicolons
            std::string combinedResponses;
            for (const auto &response : responses)
            {
                if (!combinedResponses.empty())
                {
                    combinedResponses += ";";
                }
                combinedResponses += response;
            }

            con << i << ") " << trigger << ", " << combinedResponses << ", " << extraDelay << ", " << (exact ? "Yes" : "No") << "\n";
            i++;
        }
    }
    else
    {
        FileManager fileManager(se_playerTriggerMessagesFile, tDirectories::Var());
        tArray<tString> lines = fileManager.Load();

        con << "Chat triggers not loaded. Reading from file:\n"
            << "Line) Trigger, Response, Extra Delay, Exact?\n";

        for (int i = 0; i < lines.Len(); ++i)
        {
            tArray<tString> parts = lines[i].Split(",");

            if (parts.Len() != 4)
            {
                con << "Malformed line at index " << i + 1 << ": " << lines[i] << "\n";
                continue;
            }

            con << i + 1 << ") "
                << parts[0] << ", " << parts[1] << ", " << parts[2] << ", "
                << (parts[3] == "1" ? "Yes" : "No") << "\n";
        }
    }
}

static void ClearChatTriggers(std::istream &s)
{
    FileManager fileManager(se_playerTriggerMessagesFile, tDirectories::Var());
    eChatBot &bot = eChatBot::getInstance();
    fileManager.Clear();
    bot.chatTriggers.clear();
    con << "All chat triggers have been cleared.\n";
}

static void ReloadChatTriggers(std::istream &s)
{
    eChatBot &bot = eChatBot::getInstance();
    bot.LoadChatTriggers();
}

static tConfItemFunc ClearChatTriggers_conf = HelperCommand::tConfItemFunc("PLAYER_MESSAGE_TRIGGERS_CLEAR", &ClearChatTriggers);
static tConfItemFunc ListChatTriggers_conf = HelperCommand::tConfItemFunc("PLAYER_MESSAGE_TRIGGERS_LIST", &ListChatTriggers);
static tConfItemFunc AddChatTrigger_conf = HelperCommand::tConfItemFunc("PLAYER_MESSAGE_TRIGGERS_ADD", &AddChatTrigger);
static tConfItemFunc RemoveChatTrigger_conf = HelperCommand::tConfItemFunc("PLAYER_MESSAGE_TRIGGERS_REMOVE", &RemoveChatTrigger);
static tConfItemFunc ReloadChatTriggers_conf = HelperCommand::tConfItemFunc("PLAYER_MESSAGE_TRIGGERS_RELOAD", &ReloadChatTriggers);
