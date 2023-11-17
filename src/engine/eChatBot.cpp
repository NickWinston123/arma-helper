#include "eChatBot.h"
#include "ePlayer.h"
#include "ePlayerStats.h"
#include "eTimer.h"
#include "../tron/gGame.h"
#include "../tron/gMenus.h"
#include "tDatabase.h"

#include "eChatCommands.h"

#include <stack>
#include <queue>

bool se_playerTriggerMessages = false;
static tConfItem<bool> se_playerTriggerMessagesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS", se_playerTriggerMessages);

bool se_playerTriggerMessagesResendSilencedMessages = false;
static tConfItem<bool> se_playerTriggerMessagesResendSilencedMessagesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_RESEND_SILENCED_MESSAGES", se_playerTriggerMessagesResendSilencedMessages);

int se_playerTriggerMessagesQueueMaxOverloadedSize = 10;
static tConfItem<int> se_playerTriggerMessagesQueueMaxOverloadedSizeConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_QUEUE_OVERLOADED_MAX_SIZE", se_playerTriggerMessagesQueueMaxOverloadedSize);

tString se_playerTriggerMessagesFile("chattriggers.txt");
static tConfItem<tString> se_playerTriggerMessagesFileConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FILE", se_playerTriggerMessagesFile);

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

static bool se_playerMessageTriggersChatFlag = false;
static tConfItem<bool> se_playerMessageTriggersChatFlagConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CHATFLAG", se_playerMessageTriggersChatFlag);

static REAL se_playerMessageTriggersChatFlagStartMult = 0.5;
static tConfItem<REAL> se_playerMessageTriggersChatFlagStartMultConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CHATFLAG_START_MULT", se_playerMessageTriggersChatFlagStartMult);

static bool se_playerMessageDisplayScheduledMessages = false;
static tConfItem<bool> se_playerMessageDisplayScheduledMessagesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SHOW_SCHEDULED_MESSAGES", se_playerMessageDisplayScheduledMessages);

static int se_playerMessageTriggersFuncAdderVal = 1;
static tConfItem<int> se_playerMessageTriggersFuncAdderValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_ADDER_ADD_VAL", se_playerMessageTriggersFuncAdderVal);

static bool se_playerMessageTriggersMasterFuncFeedback = false;
static tConfItem<bool> se_playerMessageTriggersMasterFuncFeedbackConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_MASTER_FEEDBACK", se_playerMessageTriggersMasterFuncFeedback);

static bool se_playerMessageTriggersLeaderboardLabels = false;
static tConfItem<bool> se_playerMessageTriggersLeaderboardLabelsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_LEADERBOARD_LABELS", se_playerMessageTriggersLeaderboardLabels);

static REAL se_playerMessageTriggersSearchMaxResults = 10;
static tConfItem<REAL> se_playerMessageTriggersSearchMaxResultsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_SEARCH_MAX_RESULTS", se_playerMessageTriggersSearchMaxResults);

static int se_playerMessageTriggersChatSearchMaxResults = 10;
static tConfItem<int> se_playerMessageTriggersChatSearchMaxResultsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CHAT_SEARCH_MAX_RESULTS", se_playerMessageTriggersChatSearchMaxResults);


REAL se_playerTriggerMessagesSpamMaxlen = 300;
static tConfItem<REAL> se_playerTriggerMessagesSpamMaxlenConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SPAM_MAXLEN", se_playerTriggerMessagesSpamMaxlen);

REAL se_playerTriggerMessagesSpamMaxlenPartAdd = 0.5;
static tConfItem<REAL> se_playerTriggerMessagesSpamMaxlenPartAddConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY_SPAM_MAXLEN_NEGATIVE_PART_ADD", se_playerTriggerMessagesSpamMaxlenPartAdd);

static bool se_playerMessageTriggersChatFunctionsOnly = false;
static tConfItem<bool> se_playerMessageTriggersChatFunctionsOnlyConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_CHAT_FUNCTIONS_ONLY", se_playerMessageTriggersChatFunctionsOnly);

bool se_playerTriggerMessagesRageQuits = false;
static tConfItem<bool> se_playerTriggerMessagesRageQuitsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_RAGE_QUITS", se_playerTriggerMessagesRageQuits);

bool se_playerTriggerMessagesAcheivements = false;
static tConfItem<bool> se_playerTriggerMessagesAcheivementsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS", se_playerTriggerMessagesAcheivements);

bool se_playerTriggerMessagesAcheivementsLocal = false;
static tConfItem<bool> se_playerTriggerMessagesAcheivementsLocalConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_LOCAL", se_playerTriggerMessagesAcheivementsLocal);

extern int se_playerTriggerMessagesAcheivementsKillsChangeVal = 50;
static tConfItem<int> se_playerTriggerMessagesAcheivementsKillsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsKillsChangeVal);

extern int se_playerTriggerMessagesAcheivementsKillStreakChangeVal = 5;
static tConfItem<int> se_playerTriggerMessagesAcheivementsKillStreakChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLSTREAK_CURRENT_CHANGE_VAL", se_playerTriggerMessagesAcheivementsKillStreakChangeVal);

extern int se_playerTriggerMessagesAcheivementsMaxKillStreak = 1;
static tConfItem<int> se_playerTriggerMessagesAcheivementsMaxKillStreakConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLSTREAK_MAX_CHANGE_VAL", se_playerTriggerMessagesAcheivementsMaxKillStreak);

extern int se_playerTriggerMessagesAcheivementsChatsChangeVal = 25;
static tConfItem<int> se_playerTriggerMessagesAcheivementsChatsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_CHATS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsChatsChangeVal);

extern int se_playerTriggerMessagesAcheivementsJoinsChangeVal = 10;
static tConfItem<int> se_playerTriggerMessagesAcheivementsJoinsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_JOINS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsJoinsChangeVal);

extern int se_playerTriggerMessagesAcheivementsBansChangeVal = 5;
static tConfItem<int> se_playerTriggerMessagesAcheivementsBansChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_BANS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsBansChangeVal);


void addPrefix(tString &input, tString &prefix)
{
    tString prefixedString(prefix);
    prefixedString << input;
    input = prefixedString;
}

void validateOutput(tString &input)
{
    if (!input.empty() && input[0] == '/')
    {
        tString paddedString(" ");
        addPrefix(input, paddedString);
    }
}

tString stripNonOperatorsOrNumbers(const tString &input);
bool containsMath(tString input, bool exact);
bool containsRepeatedCharacters(tString message, bool exact);

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

    bool originalIsNegativeZero = isNegative && intPart.empty();

    int carry;
    if (originalIsNegativeZero) {
        carry = se_playerMessageTriggersFuncAdderVal;
    } else {
        carry = isNegative ? -se_playerMessageTriggersFuncAdderVal : se_playerMessageTriggersFuncAdderVal;
    }

    for (int i = fracPart.size() - 1; i >= 0; i--) {
        int sum = (fracPart[i] - '0') + carry;
        carry = sum / 10;
        fracPart[i] = '0' + (sum % 10);
    }

    if (intPart.empty()) intPart = "0";

    for (int i = intPart.size() - 1; i >= 0 && carry != 0; i--) {
        int sum = (intPart[i] - '0') + carry;
        carry = sum / 10;
        intPart[i] = '0' + (sum % 10);
    }

    while (carry != 0) {
        intPart = char('0' + (carry % 10)) + intPart;
        carry /= 10;
    }

    if (originalIsNegativeZero && !fracPart.empty() && fracPart == "0") {
        isNegative = true;
    }

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

    return input;
}

tString reverseFunc(tString message)
{
    tString input = message;

    input = input.Reverse();

    return input;
}

tString statsFunc(tString message)
{
    tString output;
    std::vector<std::pair<tString, tString>> statLabels = {
        {tString("RGB"), tString("RGB")},
        {tString("Play Time"), tString("Play Time")},
        {tString("Spectate Time"), tString("Spec Time")},
        {tString("Chat Count"), tString("Chats")}
    };

    if (!message.empty())
    {
        tString playerName(message.SubStr(0, 16).TrimWhitespace());
        ePlayerNetID* statsTargetPlayer = ePlayerNetID::GetPlayerByName(playerName, false);
        if (statsTargetPlayer)
            playerName = statsTargetPlayer->GetName();

        PlayerData* stats;
        if (statsTargetPlayer)
            stats = &ePlayerStats::getStats(statsTargetPlayer);
        else
            stats = &ePlayerStats::getStatsForAnalysis(playerName);

        tString displayName(!statsTargetPlayer || ePlayerStats::shouldEnforceLocalName(statsTargetPlayer) ?
                            (!stats->name.empty() ? stats->name : playerName) : playerName);

        output << displayName << ": ";

        bool playingPlayer = statsTargetPlayer && statsTargetPlayer->CurrentTeam();

        for (const auto& pair : statLabels)
        {
            const tString& key = pair.first;
            const tString& label = pair.second;

            if (stats->isPrivate(key))
            {
                output << label << ": ?, ";
            }
            else
            {
                if (key == "Play Time" || key == "Spectate Time")
                {
                    tString statValue = (playingPlayer ? stats->getPlayTimeStr() : stats->getSpecTimeStr());
                    if (stats->in_server)
                        statValue << " (" << (playingPlayer ? stats->thisSession().getPlayTimeStr() : stats->thisSession().getSpecTimeStr()) << ")";
                    output << label << ": " << statValue << ", ";
                }
                else if (key == "RGB")
                {
                    output << label << ": " << (stats->rgbString()) << ", ";
                }
                else if (key == "Chat Count")
                {
                    output << label << ": " << stats->getMessageCount();
                    if (stats->in_server)
                        output << " (" << stats->thisSession().getMessageCount() << ")";
                }
            }
        }

        if (output.EndsWith(", "))
            output = output.SubStr(0, output.Len() - 2);

        if (statsTargetPlayer)
        {
            if (se_playerStats)
                output << ", ";

            if (se_GetLocalPlayer()->createdTime() <= statsTargetPlayer->createdTime())
            {
                struct tm created = statsTargetPlayer->createTime_;
                struct tm now = getCurrentLocalTime();
                time_t difference = getDifferenceInSeconds(now, created);
                tString formattedDifference = st_GetFormatTime(difference, false, false);
                output << "Joined: "
                    << getTimeStringBase(statsTargetPlayer->createTime_)
                    << " EST ("
                    << formattedDifference
                    << "), ";
            }
            output << "Last Active: "
                   << st_GetFormatTime(statsTargetPlayer->LastActivity(), false, false);
        }
        else
        {
        if (se_playerStats) {
            output << ", ";
            if (stats->isPrivate(tString("Last Seen")))
                output << "Last seen: ?";
            else
                output << "Last seen: " << stats->getLastSeenAgoStr();
        }

        }

        return tString(output);
    }

    eChatBotStats &stats = eChatBot::getInstance().Stats();

    output << "Uptime: "
           << st_GetFormatTime(stats.upTime(), false, false)
           << " ("
           << st_GetFormatTime(stats.upTime(false), false, false)
           << "), "
           << "Messages Sent: "
           << stats.messagesSent()
           << " ("
           << stats.messagesSent(false)
           << "), "
           << "Messages Read: "
           << stats.messagesRead()
           << " ("
           << stats.messagesRead(false)
           << "), "
           << "Players: "
           << ePlayerStats::getTotalPlayersLogged()
           << " ("
           << ePlayerStats::getTotalPlayersLogged(false)
           << ")";

    return tString(output);
}

bool nickNameAvailable(tString nickname)
{
    nickname = nickname.ToLower();
    std::string nicknameStr = nickname.stdString();

    time_t currentTime = time(NULL);
    const time_t seventyTwoHoursInSeconds = 72 * 60 * 60; 

    for (auto pair : ePlayerStats::GetPlayerStatsMap())
    {
        PlayerData &stats = pair.second;

        bool hasPlayedWithinThreeDays = currentTime - stats.last_seen <= seventyTwoHoursInSeconds;
        if (stats.nickname == nicknameStr || (hasPlayedWithinThreeDays && stats.name == nickname))
            return false;
    }

    for (int i = MAX_PLAYERS - 1; i >= 0; i--)
    {
        ePlayer *lp = ePlayer::PlayerConfig(i);
        if (lp && lp->netPlayer)
        {
            if (lp->name.ToLower() == nickname)
                return false;
        }
    }
    return true;
}

tString nicknameFunc(tString message)
{
    tString output;
    tString newNickname = message.TrimWhitespace();

    if (!se_playerStats)
    {
        output << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return output;
    }

    ePlayerNetID *lastTriggeredBy = eChatBot::getInstance().Params().triggeredBy;
    PlayerData &stats = ePlayerStats::getStats(lastTriggeredBy);

    newNickname = newNickname.SubStr(0, 16);
    newNickname.RecomputeLength();
    newNickname = newNickname.TrimWhitespace();

    if (lastTriggeredBy == nullptr)
        return output;

    if (newNickname.empty() && !stats.nickname.empty())
    {
            std::string lastNick = stats.nickname;
            stats.nickname.clear();

            output << lastTriggeredBy->GetName()
                   << "'s nickname has been cleared and is no longer '"
                   << lastNick
                   << "'.\n";
            return output;
    }
    else if (newNickname.empty())
    {
        output << "Your nickname can not be blank.";
        return output;
    }
    else if (ePlayerNetID::GetPlayerByName(newNickname) || !nickNameAvailable(newNickname))
    {
        output << "A player with the name '" << newNickname << "' already exist.\n";
        return output;
    }
    else
    {
        stats.nickname = newNickname.stdString();
        output << lastTriggeredBy->GetName()
                << " is now nicknamed '"
                << stats.nickname
                << "'\n";

        return output;
    }

    return tString(output);
}

tString playerKDFunc(tString message)
{
    tString output;
    message = message.TrimWhitespace();
    int pos = 0;
    tString mode(message.ExtractNonBlankSubString(pos));
    bool current = false;

    if (mode == "now")
        current = true;
    else if (mode == "total")
        current = false;
    else
    {
        output << "No mode set!";
        return output;
    }

    message = message.SubStr(pos).TrimWhitespace();

    ePlayerNetID *statsTargetPlayer = nullptr;
    tString playerName;

    if (!se_playerStats)
    {
        output << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return output;
    }

    if (!message.empty())
    {
        playerName = message.SubStr(0, 16).TrimWhitespace();
        statsTargetPlayer = ePlayerNetID::GetPlayerByName(playerName, false);
        if (statsTargetPlayer)
            playerName = statsTargetPlayer->GetName();
    }
    else
    {
        eChatBot &bot = eChatBot::getInstance();
        statsTargetPlayer = bot.Params().triggeredBy;
        if (statsTargetPlayer)
            playerName = statsTargetPlayer->GetName();
    }

    if (!playerName.empty())
    {
        PlayerData *stats;
        if (statsTargetPlayer)
            stats = &ePlayerStats::getStats(statsTargetPlayer);
        else
            stats = &ePlayerStats::getStatsForAnalysis(playerName);

        tString displayName(!statsTargetPlayer || ePlayerStats::shouldEnforceLocalName(statsTargetPlayer) ? (!stats->name.empty() ? stats->name : playerName) : playerName);
        output << displayName
               << (current ? " (NOW)" : "")
                << ": ";

        std::vector<tString> orderedKeys =
        {
            tString("kills"),
            tString("deaths"),
            tString("kd"),
            tString("rounds_played"),
            tString("round_wins"),
            tString("round_losses"),
            tString("matches_played"),
            tString("match_wins"),
            tString("match_losses")
        };

        std::vector<tString> separators =
        {
            tString(", "),
            tString(", "),
            tString(" | Rounds- "),
            tString(", "),
            tString(", "),
            tString(" | Matches- "),
            tString(", "),
            tString(", ")
        };

        for (size_t i = 0; i < orderedKeys.size(); ++i)
        {
            const tString &key = orderedKeys[i];
            tString realLabel = PlayerData::getAnyLabel(key);
            tString label = realLabel;

            if (key == "kd")
                label = "K/D";
            else if (key == "rounds_played")
                label = "Played";
            else if (key == "round_wins")
                label = "Wins";
            else if (key == "round_losses")
                label = "Losses";
            else if (key == "matches_played")
                label = "Played";
            else if (key == "match_wins")
                label = "Wins";
            else if (key == "match_losses")
                label = "Losses";

            if (i != 0)
                output << separators[i - 1];

            if (stats->isPrivate(realLabel))
            {
                if (!label.empty())
                    output << label << ": ?";
                else
                    output << "?";
            }
            else
            {
                if (!label.empty())
                    output << label << ": ";

                output << (current ? stats->thisSession().getAnyValue(key) : stats->getAnyValue(key));
            }
        }
    }
    else
        output << "Player not found!";

    return output;
}

tString leaderboardFunc(tString message)
{
    tString result;
    tString statName("kills");
    tString statLabel;
    int page = 1;
    const int playersPerPage = 5;

    bool desiredStatChoosen = false;

    if (!se_playerStats)
    {
        result << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return result;
    }

    int pos = 0;
    message = message.TrimWhitespace();

    tString desiredStat = message.ExtractNonBlankSubString(pos);

    if (!desiredStat.empty())
    {
        desiredStatChoosen = true;
        statName = desiredStat;
    }

    statLabel = PlayerData::getAnyLabel(statName);

    tString pageStr = message.ExtractNonBlankSubString(pos);

    if (!pageStr.empty())
        page = atoi(pageStr);

    if (page <= 0)
        page = 1;

    bool lastSeenStat = (statLabel == "Last Seen");
    bool showLabel = !desiredStatChoosen || se_playerMessageTriggersLeaderboardLabels;
    bool playTimeStat = (statLabel == "Play Time");
    bool specTimeStat = (statLabel == "Spectate Time");
    bool currentKillStreakStat = (statLabel == "Current Kill Streak");
    bool nicknameStat = (statLabel == "Nickname");

    bool penalizePlayersNotInGame = false;
    bool ignorePlayersNotInGame = false;

    if (lastSeenStat || nicknameStat)
        penalizePlayersNotInGame = true;

    if (currentKillStreakStat)
        ignorePlayersNotInGame = true;

    std::vector<std::pair<tString, PlayerData>> sortedPlayers(ePlayerStats::GetPlayerStatsMap().begin(), ePlayerStats::GetPlayerStatsMap().end());
    auto it = std::remove_if(sortedPlayers.begin(), sortedPlayers.end(),
                             [ignorePlayersNotInGame, nicknameStat](const std::pair<tString, PlayerData>& player)
                             {
                                 return !player.second.human ||
                                        (ignorePlayersNotInGame && !player.second.in_server) ||
                                        player.second.deleted ||
                                        (nicknameStat && player.second.nickname.empty());
                             });

    sortedPlayers.erase(it, sortedPlayers.end());


    if (showLabel)
    {
        bool fastestStat = !lastSeenStat && (statLabel == "Fastest Speed");
        if (lastSeenStat || fastestStat)
            showLabel = false;
    }

    auto sortUsingGetAnyValue = [&statName, &statLabel, &penalizePlayersNotInGame, &playTimeStat, &specTimeStat](auto &a, auto &b)
    {
        bool aPrivate = a.second.isPrivate(statLabel);
        bool bPrivate = b.second.isPrivate(statLabel);

        if (aPrivate && !bPrivate) return false;
        if (!aPrivate && bPrivate) return true;

        if (penalizePlayersNotInGame)
        {
            if (a.second.in_server && b.second.in_server)
                return a.second.last_seen < b.second.last_seen;
            if (a.second.in_server) return true;
            if (b.second.in_server) return false;

            return a.second.last_seen > b.second.last_seen;
        }
        else if (playTimeStat)
        {
            return a.second.total_play_time > b.second.total_play_time;
        }
        else if (specTimeStat)
        {
            return a.second.total_spec_time > b.second.total_spec_time;
        }
        else
        {
            REAL aValue = atof(a.second.getAnyValue(statName).c_str());
            REAL bValue = atof(b.second.getAnyValue(statName).c_str());
            return aValue > bValue;
        }
    };


    auto stat = PlayerData::valueMap.find(statName.stdString());
    if (stat != PlayerData::valueMap.end())
        std::sort(sortedPlayers.begin(), sortedPlayers.end(), sortUsingGetAnyValue);
    else
        return PlayerData::getAvailableStatsStr("leaderboardFunc");

    int startIdx = (page - 1) * playersPerPage;
    int endIdx = startIdx + playersPerPage;
    if (endIdx > sortedPlayers.size())
        endIdx = sortedPlayers.size();

    if (startIdx >= sortedPlayers.size())
    {
        result << "No players on page " << page << "\n";
        return result;
    }

    int totalPages = (sortedPlayers.size() + playersPerPage - 1) / playersPerPage;

    if (showLabel)
    {
        tString labelToDisplay(statLabel);
        if (nicknameStat)
            labelToDisplay = "Nickames";
        result << labelToDisplay << ": ";
    }

    for (int i = startIdx; i < endIdx; ++i)
    {
        PlayerData &stats = sortedPlayers[i].second;
        bool showValue = !stats.isPrivate(statLabel);
        if (i > startIdx)
        {
            result << ", ";
        }

        result << (i + 1) << ") " << sortedPlayers[i].first;
        tString value;
        if (showValue)
        {
            if (playTimeStat)
                value << stats.getPlayTimeStr();
            else if (specTimeStat)
                value << stats.getSpecTimeStr();
            else
                value << stats.getAnyValue(statName);
        }
        else
        {
            value << "?";
        }
        result << " (" << value << ")";
    }
    result << " (" << page << "/" << totalPages << ")\n";

    return result;
}

tString exactStatFunc(tString message)
{
    tString output;
    tString playerName, stat;
    ePlayerNetID *statsTargetPlayer = nullptr;
    eChatBot &bot = eChatBot::getInstance();

    bool symLinkFunc = bot.Params().triggerType == ResponseType::SYM_FUNC;

    if (!se_playerStats)
    {
        output << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return output;
    }

    message = message.TrimWhitespace();
    int pos = 0;
    stat = message.ExtractNonBlankSubString(pos);

    tString statLabel;
    statLabel = PlayerData::getAnyLabel(stat);

    if (statLabel.empty())
    {
        output << "Stat not found! Usage: '"
               << bot.Params().matchedTrigger
               << " "
               << "stat"
               << " player' "
               << PlayerData::getAvailableStatsStr();
        return output;
    }

    playerName = message.SubStr(pos+1).TrimWhitespace();

    if (playerName.empty())
    {
        statsTargetPlayer = bot.Params().triggeredBy;
    }

    if (!statsTargetPlayer)
        statsTargetPlayer = ePlayerNetID::GetPlayerByName(playerName, false);

    if (statsTargetPlayer)
        playerName = statsTargetPlayer->GetName();

    PlayerData *statsPtr;
    if (statsTargetPlayer)
        statsPtr = &ePlayerStats::getStats(statsTargetPlayer);
    else
        statsPtr = &ePlayerStats::getStatsForAnalysis(playerName);
    PlayerData &stats = *statsPtr;

    bool showValue = statLabel == "Hidden Stats" || !stats.isPrivate(statLabel);

    tString statValue;
    tString statAfterValue;
    if (showValue)
    {
        bool showCurrentSessionVal = true;

        tString currentSessionStatValue;
        if (statLabel == "Kill Death Ratio")
        {
            statValue << stats.getKDRatio(false);
            if (stats.in_server)
                currentSessionStatValue << stats.thisSession().getKDRatio(false);
        }
        else if (statLabel == "Fastest Speed")
        {
            statValue << stats.getSpeed(false);
            if (stats.in_server)
                currentSessionStatValue << stats.thisSession().getSpeed(false);
        }
        else if (statLabel == "Last Seen")
        {
            statValue << stats.getLastSeenAgoStr(true);
            if (stats.in_server)
                currentSessionStatValue << stats.thisSession().getLastSeenAgoStr(true);
        }
        else if (statLabel == "Times Banned")
        {
            statValue << stats.times_banned;

            statAfterValue << " - Bans Given: "
                           << stats.bans_given;

            if (stats.in_server)
                statAfterValue << " | This session: ("
                            << stats.thisSession().bans_given
                            << ")";
        }
        else if (statLabel == "Bans Given")
        {
            statValue << stats.bans_given;

            statAfterValue << " - Times Banned: "
                           << stats.times_banned;

            if (stats.in_server)
                statAfterValue << " | This session: ("
                            << stats.thisSession().times_banned
                            << ")";
        }
        else if (statLabel == "RGB")
        {
            statValue << stats.rgbString();

            if (!bot.Params().pentalized_for_last_message)
            {
                int r,g,b;
                if (statsTargetPlayer)
                {
                    r = statsTargetPlayer->r;
                    g = statsTargetPlayer->g;
                    b = statsTargetPlayer->g;
                }
                else
                {
                    r = stats.r;
                    g = stats.g;
                    b = stats.b;
                }

                statValue << " "
                          << ColorsCommand::cycleColorPreview(r, g, b, false);
            }
            showCurrentSessionVal = false;
        }
        else
            statValue << stats.getAnyValue(stat.TrimWhitespace());

        if (showCurrentSessionVal && stats.in_server)
        {
            if (currentSessionStatValue.empty())
                currentSessionStatValue << stats.thisSession().getAnyValue(stat.TrimWhitespace());

            statValue << " | This session: ("
                      << currentSessionStatValue
                      << ")";
        }
    }
    else
    {
        statValue << "?";
    }

    statValue << statAfterValue;

    tString displayName(!statsTargetPlayer || ePlayerStats::shouldEnforceLocalName(statsTargetPlayer) ? (!stats.name.empty() ? stats.name : playerName) : playerName);
    output << displayName
            << ": "
            << statLabel
            << " - ";

    if (!statValue.empty() || statLabel == "Chats" || statLabel == "Hidden Stats")
    {
        output << statValue;
    }
    else
        output << "Nothing found";

    return output;
}

tString masterFunc(tString message)
{
    tString output;
    eChatBot &bot = eChatBot::getInstance();

    if (!bot.Params().triggeredBy || !bot.Params().triggeredBy->encryptVerified )
        return sayFunc(message);

    bot.Params().abortOutput = true;

    sn_consoleUser(ePlayer::NetToLocalPlayer(se_GetLocalPlayer()));
    tCurrentAccessLevel level(tAccessLevel_Owner, true);
    std::stringstream s(static_cast<char const *>(message));
    tConfItemBase::LoadAll(s);

    if (se_playerMessageTriggersMasterFuncFeedback)
    {
        tString feedback;

        if (tConfItemBase::lastLoadOutput.empty())
            feedback << "Line loaded: '" << message << "'";
        else
            feedback << tConfItemBase::lastLoadOutput;

        for (auto localNetPlayer : se_GetLocalPlayers())
        {
            se_NewChatMessage(localNetPlayer, feedback)->BroadCast();
            break;
        }
    }

    return output;
}

tString hideStatFunc(tString message)
{
    tString output;
    message = message.TrimWhitespace();

    if (!se_playerStats)
    {
        output << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return output;
    }

    eChatBot &bot = eChatBot::getInstance();
    bool symLinkFunc = bot.Params().triggerType == ResponseType::SYM_FUNC;
    ePlayerNetID *triggeredBy = bot.Params().triggeredBy;
    PlayerData &stats = ePlayerStats::getStats(triggeredBy);
    int pos = 0;
    tString desiredAction = message.ExtractNonBlankSubString(pos);

    if (desiredAction.empty())
    {
        output << "Desired action not found. "
               << "Usage: '"
               << bot.Params().matchedTrigger
               << " ";
        if (!symLinkFunc)
            output << "unprivate or private"
                   << " ";
        output << "stat'";
        return output;
    }
    tArray<tString> statsArray = message.Split(" ");
    statsArray.RemoveAt(0);

    std::vector<tString> validStats;
    std::vector<tString> invalidStats;

    for (const auto &statName : statsArray)
    {
        tString statLabel = PlayerData::getAnyLabel(statName);

        if (statName.ToLower() == "all")
            statLabel = "ALL STATS";

        if (statLabel.empty())
            invalidStats.push_back(statName);
        else
            validStats.push_back(statLabel);
    }

    if (validStats.empty() && invalidStats.size() == 1)
    {
        output << "Stat not found! Usage: '"
               << bot.Params().matchedTrigger
               << " ";
        if (!symLinkFunc)
            output << desiredAction
                   << " ";


        output << "stat' "
                << PlayerData::getAvailableStatsStr("hidestatfunc");
        return output;
    }

    tString action;

    if (desiredAction == "private")
    {
        action << "hidden";
        for (const auto &statLabel : validStats)
            stats.addPrivateStat(statLabel);
    }
    else if (desiredAction == "unprivate")
    {
        action << "unhidden";
        for (const auto &statLabel : validStats)
            stats.removePrivateStat(statLabel);
    }

    output << triggeredBy->GetName();

    if (!validStats.empty())
    {
        output << ": ";
        for (size_t i = 0; i < validStats.size(); ++i)
        {
            if (i > 0)
                output << ", ";
            output << validStats[i];
        }
        output << " has been " << action;
    }

    if (!invalidStats.empty())
    {
        if (!validStats.empty())
            output << ". ";

        output << "Not found: ";
        for (size_t i = 0; i < invalidStats.size(); ++i)
        {
            if (i > 0)
                output << ", ";
            output << invalidStats[i];
        }
    }

    return output;
}

tString whatsThefunc(tString message)
{
    tString output;

    if (!se_playerStats)
    {
        output << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return output;
    }

    tString target = message.TrimWhitespace();

    if (target.Contains("score"))
    {
        ePlayerNetID::SortByScore();

        int max = se_PlayerNetIDs.Len();
        for (int i = 0; i < max; ++i)
        {
            ePlayerNetID *p = se_PlayerNetIDs(i);
            if (p && p->CurrentTeam())
            {
                output << p->GetName() << " (" << p->Score() << ")";
                if (i < max - 1 && se_PlayerNetIDs(i + 1) && se_PlayerNetIDs(i + 1)->CurrentTeam())
                    output << ", ";
            }
        }
    }
    else if (target.Contains("game_time"))
    {
        output << se_GameTime();
    }
    else if (target.Contains("joined_time"))
    {
        eChatBot &bot = eChatBot::getInstance();
        ePlayerNetID *targetPlayer = bot.Params().triggeredBy;

        if (targetPlayer)
        {
            struct tm created = targetPlayer->createTime_;
            output << getTimeStringBase(created)
                   << " EST";
        }
    }
    else if (target.Contains("been_here_for"))
    {
        eChatBot &bot = eChatBot::getInstance();
        ePlayerNetID *targetPlayer = bot.Params().triggeredBy;

        if (targetPlayer)
        {
            if (se_GetLocalPlayer()->createdTime() <= targetPlayer->createdTime())
            {
                output << "not sure, you joined before me";
            }
            else
            {
                struct tm now = getCurrentLocalTime();
                struct tm created = targetPlayer->createTime_;
                time_t difference = getDifferenceInSeconds(now, created);
                tString formattedDifference = st_GetFormatTime(difference, false, false);
                output << formattedDifference;
            }
        }
    }
    else if (target.Contains("current_server"))
    {
        nServerInfoBase *connectedServer = CurrentServer();
        if (connectedServer)
            output << tColoredString::RemoveColors(connectedServer->GetConnectionName());
        else
            output << "No Server?";
    }
    else if (target.Contains("players_today"))
    {
        time_t startOfDay = getStartTimeOfDay();

        int number_of_players = 0;
        for (auto stats : ePlayerStats::GetPlayerStatsMap())
        {
            if (stats.second.last_seen >= startOfDay)
                number_of_players++;
        }

        output << "Number of players since 12:00AM EST: " << number_of_players << "\n";
    }
    else if (target.Contains("players_this_hour"))
    {
        time_t startOfHour = getStartOfCurrentHour();

        int number_of_players = 0;
        for (auto statPair : ePlayerStats::GetPlayerStatsMap())
        {
            if (statPair.second.last_seen >= startOfHour || statPair.second.in_server)
                number_of_players++;
        }

        output << "Number of players since start of this hour: " << number_of_players << "\n";
    }
    else if (target.Contains("times_banned_today"))
    {
        eChatBotStats &stats = eChatBot::getInstance().Stats();
        output << stats.times_banned_today;

    }
    else if (target.Contains("times_banned"))
    {
        eChatBotStats &stats = eChatBot::getInstance().Stats();
        output << stats.times_banned;

    }
    else if (target.Contains("last_time_banned"))
    {
        eChatBotStats &stats = eChatBot::getInstance().Stats();
        struct tm lastTimeBanned;

        time_t now = time(NULL);
        double lastBanned = difftime(now, stats.last_banned);

        localtime_s(&lastTimeBanned, &stats.last_banned);
        output << getTimeStringBase(lastTimeBanned)
               << " EST"
               << " ("
               << st_GetFormatTime(lastBanned, false, false)
               << " ago)";
    }
    else if (target.Contains("time"))
    {
        struct tm now = getCurrentLocalTime();
        output << getTimeStringBase(now)
               << " EST";
    }
    else if (target.Contains("my_color"))
    {
        eChatBot &bot = eChatBot::getInstance();
        ePlayerNetID *targetPlayer = bot.Params().triggeredBy;

        if (targetPlayer)
        {
            PlayerData *stats = &ePlayerStats::getStats(targetPlayer);
            if (stats)
            {
                output << targetPlayer->GetName()
                       << ": "
                       << stats->rgbString()
                       << " "
                       << ColorsCommand::cycleColorPreview(targetPlayer->r,targetPlayer->g,targetPlayer->b, false);
            }
        }
    }

    return output;
}

tString consolidatePlayerStatsFunc(tString message)
{
    tString output;

    tString params = message.TrimWhitespace();

    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeredByPlayer = bot.Params().triggeredBy;

    if (!triggeredByPlayer || !triggeredByPlayer->encryptVerified )
    {
        output << "You are not verified";
        return output;
    }

    return ePlayerStats::consolidatePlayerStats(params);
}

tString statsAltFunc(tString message)
{
    tString output;

    tString player = message.TrimWhitespace().ToLower();

    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeredByPlayer = bot.Params().triggeredBy;

    if (!triggeredByPlayer || !triggeredByPlayer->encryptVerified )
    {
        output << "You are not verified";
        return output;
    }

    return ePlayerStats::deletePlayerStats(player);
}

std::string toLower(const std::string& str)
{
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lowerStr;
}

tString searchFunc(tString message)
{
    tString result;
    int globalFoundCount = 0;
    int max_results = se_playerMessageTriggersSearchMaxResults;
    eChatBot &bot = eChatBot::getInstance();

    if (!se_playerStats)
    {
        result << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return result;
    }

    message = message.TrimWhitespace();
    int pos = 0;
    tString statName = message.ExtractNonBlankSubString(pos);
    tString statLabel = PlayerData::getAnyLabel(statName);
    bool chatSearch = statLabel == "Chats";

    if (statLabel.empty())
    {
        result << "Stat not found! Usage: '"
               << bot.Params().matchedTrigger
               << " "
               << "stat"
               << " search value' "
               << PlayerData::getAvailableStatsStr();
        return result;
    }

    tString searchPhrase = message.SubStr(pos).TrimWhitespace();
    std::string lowerSearchPhrase = toLower(searchPhrase.stdString());

    for (auto& statsPair : ePlayerStats::GetPlayerStatsMap())
    {
        auto& playerData = statsPair.second;
        int playerFoundCount = 0;

        if (chatSearch)
        {
            if (playerData.is_local)
                continue;

            std::string trigger = toLower(bot.Params().matchedTrigger.stdString());
            for (const auto& chatMessage : playerData.chat_messages)
            {

                std::string lowerChatMessage = toLower(chatMessage);

                if (lowerChatMessage.find(trigger) != std::string::npos)
                    continue;

                if (lowerChatMessage.find(lowerSearchPhrase) != std::string::npos)
                {
                    result << statsPair.first << ": " << chatMessage << "\n";
                    playerFoundCount++;
                    globalFoundCount++;

                    if (globalFoundCount >= max_results)
                        return result;
                }
            }
        }
        else
        {
            tString statValue = playerData.getAnyValue(statName);
            if (statValue.Contains(searchPhrase))
            {
                result << statsPair.first << " - " << statLabel << ": " << statValue << "\n";
                globalFoundCount++;
                if (globalFoundCount >= max_results)
                    return result;

            }
        }
    }

    if (result.empty() && globalFoundCount == 0)
        result << "No results found for '" << searchPhrase << "' in " << statName << ".\n";

    return result;
}

tString sayOutLoudFunc(tString message)
{
    tString output;
    eChatBot &bot = eChatBot::getInstance();

    bot.Params().abortOutput = true;

    output << "> " << message;

    for (auto localNetPlayer : se_GetLocalPlayers())
    {
        localNetPlayer->Chat(output, true);
    }

    return output;
}

std::vector<std::string> getAllPlayerChatsPaginated(size_t pageSize, int page,
                                                    size_t &totalNumOfChats, std::string &lastMatchedTrigger)
{
    std::vector<std::string> allChats;
    size_t start = (page - 1) * pageSize;

    // sort
    std::vector<std::pair<tString, PlayerData>> sortedPlayers;
    for (const auto &statsPair : ePlayerStats::GetPlayerStatsMap())
        sortedPlayers.push_back(statsPair);

    std::sort(sortedPlayers.begin(), sortedPlayers.end(),
              [](const auto &a, const auto &b) { return a.second.last_seen > b.second.last_seen; });

    totalNumOfChats = 0;
    for (auto &statsPair : sortedPlayers)
    {
        const auto &playerName = statsPair.first;
        auto &playerData = statsPair.second;

        if (playerData.is_local || playerData.isPrivate(tString("Chats")))
            continue;

        for (const auto &chatMessage : playerData.chat_messages)
        {
            if (chatMessage.find(lastMatchedTrigger) != std::string::npos)
                continue;

            totalNumOfChats++;

            if (totalNumOfChats > start && allChats.size() < pageSize)
            {
                tString formattedMessage = playerName;
                formattedMessage << ": " << chatMessage;
                allChats.push_back(formattedMessage.stdString());
            }
        }
    }

    return allChats;
}

std::vector<std::string> searchAllPlayerChats(const tString &searchPhrase, size_t pageSize, size_t &totalNumOfChats,
                                              std::string &lastMatchedTrigger)
{
    std::vector<std::string> foundChats;
    std::string lowerSearchPhrase = toLower(searchPhrase.stdString());
    totalNumOfChats = 0;

    for (auto &statsPair : ePlayerStats::GetPlayerStatsMap())
    {
        tString playerName = statsPair.first;
        auto &playerData = statsPair.second;

        if (playerData.is_local || playerData.isPrivate(tString("Chats")))
            continue;

        for (const auto &chatMessage : playerData.chat_messages)
        {
            if (chatMessage.find(lastMatchedTrigger) != std::string::npos)
                continue;

            std::string lowerChatMessage = toLower(chatMessage);

            if (lowerChatMessage.find(lowerSearchPhrase) != std::string::npos)
            {
                totalNumOfChats++;

                if (foundChats.size() < pageSize)
                {
                    tString formattedMessage = playerName;
                    formattedMessage << ": " << chatMessage;
                    foundChats.push_back(formattedMessage.stdString());
                }
            }
        }
    }

    return foundChats;
}

std::vector<std::string> searchChats(const std::vector<std::string> &chatMessages, const tString &searchPhrase,
                                     size_t pageSize, int page, size_t &totalNumOfChats, std::string &lastMatchedTrigger)
{
    std::vector<std::string> foundChats;
    std::vector<std::string> tempChats;

    for (auto rit = chatMessages.rbegin(); rit != chatMessages.rend(); ++rit)
    {
        if (rit->find(lastMatchedTrigger) != std::string::npos)
            continue;

        if (rit->find(searchPhrase.stdString()) != std::string::npos)
            tempChats.push_back(*rit);
    }

    totalNumOfChats = tempChats.size();

    size_t start = (page - 1) * pageSize;
    size_t end = start + pageSize;

    if (start >= totalNumOfChats)
        return foundChats;

    if (end > totalNumOfChats)
        end = totalNumOfChats;

    for (size_t i = start; i < end; ++i)
        foundChats.push_back(tempChats[i]);

    return foundChats;
}

tString chatScrollSearchFunc(tString message)
{
    int pos = 0;
    bool showUsage = false;
    tString output;
    eChatBot &bot = eChatBot::getInstance();

    std::string lastMatchedTrigger(bot.Params().matchedTrigger.stdString());

    if (!se_playerStats)
    {
        output << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return output;
    }

    message = message.TrimWhitespace();
    PlayerData *stats;
    tString playerName = message.ExtractNonBlankSubString(pos);

    bool all = playerName.ToLower() == "all";
    std::vector<std::string> chatsToDisplay;
    int page = 1;
    const size_t pageSize = se_playerMessageTriggersChatSearchMaxResults;
    tString param = message.ExtractNonBlankSubString(pos);
    size_t totalNumOfChats = 0;

    if (all)
    {
        bool params = !param.empty();
        if (param.isNumber() || !params)
        {
            if (params)
                page = atoi(param.c_str());
            chatsToDisplay = getAllPlayerChatsPaginated(pageSize, page, totalNumOfChats, lastMatchedTrigger);
        }
        else
        {
            tString searchPhrase = param + message.SubStr(pos);
            chatsToDisplay = searchAllPlayerChats(searchPhrase, pageSize, totalNumOfChats, lastMatchedTrigger);
        }
    }
    else // specific player
    {
        ePlayerNetID *statsTargetPlayer = nullptr;
        if (playerName.empty())
        {
            statsTargetPlayer = bot.Params().triggeredBy;
            showUsage = true;
        }
        else
        {
            statsTargetPlayer = ePlayerNetID::GetPlayerByName(playerName, false);
        }

        if (statsTargetPlayer)
        {
            stats = &ePlayerStats::getStats(statsTargetPlayer);
            playerName = statsTargetPlayer->GetName();
        }
        else
        {
            stats = &ePlayerStats::getStatsForAnalysis(playerName);
            playerName = stats->name;

        }

        output << playerName;

        if (param.isNumber())
        {
            page = atoi(param.c_str());
            chatsToDisplay = searchChats(stats->chat_messages, tString(""), pageSize, page, totalNumOfChats, lastMatchedTrigger);
        }
        else
        {
            tString searchPhrase = param + message.SubStr(pos);
            chatsToDisplay = searchChats(stats->chat_messages, searchPhrase, pageSize, 1, totalNumOfChats, lastMatchedTrigger);
        }
    }

    output << ": ";

    for (size_t i = 0; i < chatsToDisplay.size(); ++i)
    {
        output << chatsToDisplay[i];
        if (i < chatsToDisplay.size() - 1)
            output << ", ";
    }

    size_t totalNumOfPages = (totalNumOfChats + pageSize - 1) / pageSize;

    output << " (" << page << "/" << totalNumOfPages << ")";

    if (chatsToDisplay.empty())
        output = "No chat messages found.";

    if (showUsage)
        output << " | Usage: "
               << lastMatchedTrigger
               << " name 2";

    return output;
}

tString flipRepeatedCharacters(tString message)
{
    std::string input(message.stdString());
    std::unordered_map<char, std::vector<int>> charIndices;

    for (int i = 0; i < input.size(); ++i)
    {
        charIndices[input[i]].push_back(i);
    }

    std::vector<int> indicesToSwap;
    for (const auto &pair : charIndices)
    {
        if (pair.second.size() > 1)
        {
            indicesToSwap.insert(indicesToSwap.end(), pair.second.begin(), pair.second.end());
        }
    }

    std::sort(indicesToSwap.begin(), indicesToSwap.end());

    for (size_t i = 0; i < indicesToSwap.size(); i += 2)
    {
        if (i + 1 < indicesToSwap.size())
        {
            std::swap(input[indicesToSwap[i]], input[indicesToSwap[i + 1]]);
        }
    }

    return tString(input);
}

tString banFunc(tString message)
{
    tString output;
    message = message.TrimWhitespace();

    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeredBy = bot.Params().triggeredBy;

    if (!triggeredBy)
        return output;

    ePlayerNetID *statsTargetPlayer = triggeredBy->lastDiedByPlayer;

    if (!statsTargetPlayer)
        return output;

    PlayerData &triggeredByStats = ePlayerStats::getStats(triggeredBy);

    if (triggeredByStats.banned_a_player_this_round)
    {
        bot.ResetParams();
        bot.SetInputParams(statsTargetPlayer, tString("$alreadybanned"), true);
        bot.FindTriggeredResponse();

        return bot.Params().response;
    }
    else
    {
        triggeredByStats.banned_a_player_this_round = true;
        triggeredByStats.bans_given++;
        triggeredByStats.thisSession().bans_given++;
    }

    PlayerData &bannedPlayerStats = ePlayerStats::getStats(statsTargetPlayer);
    bannedPlayerStats.times_banned++;
    bannedPlayerStats.thisSession().times_banned++;


    ePlayerStatsAcheivements::performAction(bannedPlayerStats, ePlayerStatsAcheivements::AcheivementsTypes::BANS);

    bot.ResetParams();
    bot.SetInputParams(statsTargetPlayer, tString("$banned"), true);
    bot.FindTriggeredResponse();

    return bot.Params().response;
}

tString unvalidatedSayFunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    bot.Params().validateOutput = false;
    return message;
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
    RegisterFunction(tString("$leaderboardfunc"), leaderboardFunc);
    RegisterFunction(tString("$exactstatfunc"), exactStatFunc);
    RegisterFunction(tString("$masterfunc"), masterFunc);
    RegisterFunction(tString("$hidestatfunc"), hideStatFunc);
    RegisterFunction(tString("$whatsthefunc"), whatsThefunc);
    RegisterFunction(tString("$consolidatestatsfunc"), consolidatePlayerStatsFunc);
    RegisterFunction(tString("$statsaltfunc"), statsAltFunc);
    RegisterFunction(tString("$searchfunc"), searchFunc);
    RegisterFunction(tString("$sayoutloudfunc"), sayOutLoudFunc);
    RegisterFunction(tString("$chatscrollsearchfunc"), chatScrollSearchFunc);
    RegisterFunction(tString("$fliprepeatedcharsfunc"), flipRepeatedCharacters);
    RegisterFunction(tString("$banfunc"), banFunc);
    RegisterFunction(tString("$unvalidatedsayfunc"), unvalidatedSayFunc);
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

void eChatBot::SetInputParams(ePlayerNetID *triggeredBy, tString inputMessage, bool eventTrigger, tString preAppend)
{
    Params().triggeredBy = triggeredBy;
    Params().inputMessage = inputMessage;
    Params().eventTrigger = eventTrigger;
    Params().preAppend = preAppend;
}

void eChatBot::SetOutputParams(tString &response, REAL &delay, ePlayerNetID *sendingPlayer)
{
    Params().response = response;
    Params().delay = delay;
    Params().sendingPlayer = sendingPlayer;
    Params().validateOutput = true;
}

void eChatBot::ResetParams()
{
    Params().response.Clear();
    Params().preAppend.Clear();
    Params().triggeredByName.Clear();

    Params().delay          = 0;

    Params().validateOutput = true;
    Params().abortOutput    = false;
    Params().eventTrigger   = false;

    Params().sendingPlayer  = nullptr;
    Params().triggeredBy    = nullptr;

    Params().triggerType = ResponseType::NORMAL;
}

bool eChatBot::InitiateAction(ePlayerNetID *triggeredBy, tString inputMessage, bool eventTrigger, tString preAppend)
{
    bool initiated;
    eChatBot &bot = eChatBot::getInstance();

    if (!bot.ShouldAnalyze())
        return initiated;

    if (!eventTrigger)
        bot.Stats().total_messages_read++;

    bot.ResetParams();
    bot.SetInputParams(triggeredBy, inputMessage, eventTrigger, preAppend);
    bot.FindTriggeredResponse();

    if (!bot.Params().abortOutput)
    {
        initiated = true;
        bot.SendMessage();
    }
    else if (eventTrigger)
        con << "No trigger set for '" << inputMessage << "'\nSet one with 'PLAYER_MESSAGE_TRIGGERS_ADD'\n";

    return initiated;
}


void eChatBot::FindTriggeredResponse()
{
    Params().inputMessage = tColoredString::RemoveColors(Params().inputMessage).TrimWhitespace();
    tString lowerMessage(Params().inputMessage);

    if (lowerMessage.empty())
        return;

    tToLower(lowerMessage);

    for (const auto &triggerKey : chatTriggerKeys)
    {
        auto triggerPair = chatTriggers.find(triggerKey);

        if (triggerPair == chatTriggers.end())
            continue;

        bool exact = std::get<2>(triggerPair->second), match = false;

        if (Params().eventTrigger)
            exact = true;

        tString trigger = triggerPair->first.ToLower();

        ePlayerNetID *potentialSender = nullptr;
        bool chatNameTrigger = !Params().eventTrigger && (trigger.Contains("$p1 ") || trigger.EndsWith("$p1") || trigger == "$p1");
        if (chatNameTrigger)
        {
            for (auto localNetPlayer : se_GetLocalPlayers())
            {
                tString ourName = localNetPlayer->GetName().ToLower();
                tArray<tString> nameParts = ourName.Split(" ");

                tString triggerWithoutName = trigger.Replace("$p1", "").TrimWhitespace().ToLower();
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
                    potentialSender = localNetPlayer;
                    break;
                }
            }

        }
        else if (!Params().eventTrigger && trigger.StartsWith("$"))
        {
            if (trigger == "$number")
                match = exact ? lowerMessage.isNumber() : lowerMessage.containsNumber();
            else if (trigger == "$math")
                match = containsMath(lowerMessage, exact);
            else if (trigger == "$repeatedchars")
                match = containsRepeatedCharacters(lowerMessage, exact);
            else
                match = (exact && lowerMessage == trigger) || (!exact && lowerMessage.Contains(trigger));
        }
        else
        {
            match = (exact && lowerMessage == trigger) || (!exact && lowerMessage.Contains(trigger));
        }

        if (match)
        {
            Params().matchedTrigger = trigger.TrimWhitespace();

            // Determine the sending player based on the type of trigger
            if (Params().triggeredBy != nullptr)
            {
                if (potentialSender == nullptr)
                {
                    if (tIsInList(se_playerTriggerMessagesDiedByVerifiedTriggers, trigger))
                    {
                        if (Params().triggeredBy->lastKilledPlayer == nullptr || !Params().triggeredBy->lastKilledPlayer->isLocal())
                            continue;
                        else
                            potentialSender = Params().triggeredBy->lastKilledPlayer;
                    }
                    else if (tIsInList(se_playerTriggerMessagesKillVerifiedTriggers, trigger))
                    {
                        if (Params().triggeredBy->lastDiedByPlayer == nullptr || !Params().triggeredBy->lastDiedByPlayer->isLocal())
                            continue;
                        else
                            potentialSender = Params().triggeredBy->lastDiedByPlayer;
                    }

                    if (potentialSender != nullptr && potentialSender->isLocal())
                        Params().sendingPlayer = potentialSender;
                }
                Params().triggeredByName = Params().triggeredBy->GetName();
                if (se_playerStats)
                {
                    PlayerData &stats = ePlayerStats::getStats(Params().triggeredBy);
                    if (!stats.nickname.empty())
                        Params().triggeredByName = tString(stats.nickname);
                }
            }

            REAL extraDelay = std::get<1>(triggerPair->second);

            // vector of possible responses
            std::vector<tString> possibleResponses = std::get<0>(triggerPair->second);

            // random response from the vector
            tString chosenResponse = possibleResponses[rand() % possibleResponses.size()];

            tString responseStr = chosenResponse;

            responseStr = responseStr.Replace("$p1", Params().triggeredByName);

            int dollarPos = 0;

            while ((dollarPos = chosenResponse.StrPos(dollarPos, "$")) != -1)
            {
                int openParenPos = chosenResponse.StrPos(dollarPos + 1, "(");
                int closeParenPos = -1;

                tString functionName;
                tString functionInput;

                // SYM FUNC FUNCTION? - CHECK FOR ( )
                if (openParenPos != -1)
                {
                    closeParenPos = chosenResponse.StrPos(openParenPos + 1, ")");
                    if (closeParenPos != -1)
                    {
                        functionName = chosenResponse.SubStr(dollarPos, openParenPos - dollarPos).TrimWhitespace();
                        functionInput = chosenResponse.SubStr(openParenPos + 1, closeParenPos - openParenPos - 1);
                    }
                }

                if (closeParenPos == -1)
                {
                    int endPos = chosenResponse.StrPos(dollarPos + 1, " ");
                    bool keepSpace = false;

                    if (endPos != -1)
                        keepSpace = true;
                    else
                        endPos = chosenResponse.Len();

                    functionName = chosenResponse.SubStr(dollarPos, endPos - dollarPos);
                }

                bool functionResponse = !functionName.StartsWith("$p1") &&
                                        !functionName.StartsWith("$val1");
                if (functionResponse)
                {
                    Params().triggerType = ResponseType::FUNC;

                    tString finalFunctionInput(functionInput);

                    if (finalFunctionInput.empty())
                    {
                        tString toReplace = Params().matchedTrigger + " ";
                        finalFunctionInput = Params().inputMessage.Replace(toReplace,"");

                        toReplace = Params().matchedTrigger;
                        finalFunctionInput = finalFunctionInput.Replace(toReplace,"");
                    }
                    else
                    {
                        Params().triggerType = ResponseType::SYM_FUNC;

                        tString extraParams(Params().inputMessage);
                        if (!extraParams.empty())
                            finalFunctionInput << tString(" ") << extraParams;

                        tString toReplace = Params().matchedTrigger + " ";
                        finalFunctionInput = finalFunctionInput.Replace(toReplace,"");

                        toReplace = Params().matchedTrigger;
                        finalFunctionInput = finalFunctionInput.Replace(toReplace,"");
                    }

                    finalFunctionInput = tColoredString::RemoveColors(finalFunctionInput);
                    tString result = ExecuteFunction(functionName, finalFunctionInput);

                    tString functionPattern =  functionName;
                    if (!functionInput.empty())
                        functionPattern += tString("(") + functionInput + tString(")");

                    if (!result.empty()) {
                        responseStr = responseStr.Replace(functionPattern, result);
                    } else {
                        responseStr = "";
                    }
                }

                dollarPos = (closeParenPos == -1) ? dollarPos + 1 : closeParenPos + 1;
            }

            chosenResponse = responseStr;

            if (se_playerMessageTriggersChatFunctionsOnly && Params().triggerType == ResponseType::NORMAL )
            {
                Params().abortOutput = true;
                return;
            }

            if (Params().validateOutput)
                validateOutput(chosenResponse);

            Params().delay           = extraDelay;
            Params().response        = chosenResponse;

            return;
        }
    }

    Params().abortOutput = true;
}

bool eChatBot::SendMessage()
{
    if (!helperConfig::sghuk)
        return false;

    REAL extraDelay;
    tString messageToSend = Params().response;
    tString preAppend = Params().preAppend;
    bool eventTrigger = Params().eventTrigger;
    ePlayerNetID *player = Params().sendingPlayer;

    bool forceSpecialDelay = (extraDelay < -5);

    REAL totalDelay = 0.0;
    bool scheduled = false;
    int messageLength = messageToSend.Len();

    if (messageLength > se_playerTriggerMessagesSpamMaxlen)
    {
        gHelperUtility::Debug("sendMessage","messageToSend cut because length > " +
                                                     std::to_string(se_playerTriggerMessagesSpamMaxlen) +
                                                     " Length: " +
                                                     std::to_string(messageLength));
        messageToSend = messageToSend.SubStr(0, se_playerTriggerMessagesSpamMaxlen);
    }
    if (se_playerMessageSmartDelay)
        extraDelay += calculateResponseSmartDelay(messageToSend, se_playerMessageSmartDelayTypingWPM) + se_playerMessageSmartDelayReactionTime;

    if (se_playerMessageDelay > 0)
        extraDelay += se_playerMessageDelay;

    if (se_playerMessageDelayRandMult > 0)
        extraDelay += (REAL)rand() / RAND_MAX * se_playerMessageDelayRandMult;

    totalDelay = Params().delay + extraDelay;

    REAL flagDelay = totalDelay * se_playerMessageTriggersChatFlagStartMult;

    if (flagDelay > totalDelay)
        flagDelay = totalDelay * 0.9;

    const size_t maxMessageLength = se_SpamMaxLen - 1;
    int numParts = (messageToSend.Len() + maxMessageLength - 1) / maxMessageLength;
    int previousLength = messageToSend.Len();
    tString sentMesage(messageToSend);

    while (messageToSend.Len() > 0) // break into parts determined by maxMessageLength
    {
        tString partToSend = messageToSend.SubStr(0, maxMessageLength);
        messageToSend = messageToSend.SubStr(maxMessageLength);
        partToSend.RecomputeLength();
        messageToSend.RecomputeLength();

        if (messageToSend.Len() >= previousLength)
            break;

        if (!preAppend.empty())
            partToSend = preAppend + partToSend;

        bool shouldSend = numParts == 1 || !partToSend.empty();

        if (shouldSend)
        {
            if (player != nullptr)
            {
                if (tIsInList(se_playerMessageTargetPlayer, player->pID + 1))
                {
                    scheduled = true;
                    scheduleMessageTask(player, partToSend, !forceSpecialDelay && se_playerMessageTriggersChatFlag, totalDelay, flagDelay);
                }
            }
            else
            {
                tArray<tString> players = se_playerMessageTargetPlayer.Split(",");
                int numPlayers = players.Len();

                for (int i = 0; i < numPlayers; i++)
                {
                    if (!eventTrigger && numPlayers > 1 && i > 0)
                        continue;

                    ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[i]) - 1);
                    if (!local_p)
                        continue;

                    ePlayerNetID *netPlayer = local_p->netPlayer;
                    if (!netPlayer)
                        continue;

                    scheduled = true;
                    scheduleMessageTask(netPlayer, partToSend, !forceSpecialDelay && se_playerMessageTriggersChatFlag, totalDelay, flagDelay);
                }
            }
        }

        if (forceSpecialDelay && numParts > 1)
        {
            totalDelay = se_playerTriggerMessagesSpamMaxlenPartAdd;
            flagDelay  = totalDelay * se_playerMessageTriggersChatFlagStartMult;
        }
        else
        {
            totalDelay += extraDelay + calculateResponseSmartDelay(partToSend, se_playerMessageSmartDelayTypingWPM) + se_playerMessageSmartDelayReactionTime;
            flagDelay = totalDelay * se_playerMessageTriggersChatFlagStartMult;
            if (flagDelay > totalDelay)
                flagDelay = totalDelay * 0.9;
        }
        previousLength = messageToSend.Len();
    }

    if (scheduled)
        Stats().total_messages_sent++;

    if (se_playerMessageDisplayScheduledMessages && scheduled)
        con << "Scheduled message '"   << sentMesage << "' "
            << flagDelay               << " (wait time) + "
            << totalDelay - flagDelay  << " (typing time) -> "
            << totalDelay              << " (total delay). "
            << " (ExtraTime: "         << extraDelay << " seconds.) "
            << "Sent in "              << numParts << " parts.\n";

    return scheduled;
}

bool eChatBot::ResendMessage()
{
    if (Params().pentalized_for_last_message && Params().delay <= 0)
        Params().delay = .5;

    if (Params().pentalized_for_last_message_prefix)
    {
        tString prefix;
        prefix << randomStr("!@#$%^&*()_+=-",3)
               << " ";
        addPrefix(Params().response, prefix);
    }

    return SendMessage();
}


void eChatBot::scheduleMessageTask(ePlayerNetID *netPlayer, tString message, bool chatFlag, REAL totalDelay, REAL flagDelay)
{
    gTaskScheduler.checkAndClearQueueIfOverloaded(se_playerTriggerMessagesQueueMaxOverloadedSize);

    gTaskScheduler.enqueueChain([=]() { // group Tasks
        float messageDelay = totalDelay - flagDelay;

        gTaskScheduler.schedule(
            "playerMessageTask", flagDelay, [netPlayer, message, chatFlag, messageDelay] // When we start typing
            {
                if (chatFlag)
                {
                    netPlayer->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, true);
                    ePlayerNetID::Update();

                    gTaskScheduler.schedule("playerMessageSetChatFlagFalse", messageDelay, [netPlayer, message, chatFlag]
                    {
                        netPlayer->Chat(message, true);
                        if (chatFlag)
                            netPlayer->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, false);
                        ePlayerNetID::Update();
                    }, 0, true);
                }
                else
                {
                    netPlayer->Chat(message, true);
                }
            }, 0, true);
    });
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

bool containsRepeatedCharacters(tString message, bool exact)
{
    std::string input(message.stdString());
    std::unordered_map<char, int> charCount;

    for (char c : input)
    {
        if (std::isdigit(c))
            return false;

        charCount[c]++;
    }

    if (!exact)
    {
        for (const auto &pair : charCount)
        {
            if (pair.second > 1)
                return true;
        }
        return false;
    }
    else
    {
        for (const auto &pair : charCount)
        {
            if (pair.second == 1)
                return false;
        }
        return true;
    }
}

bool containsMath( tString input, bool exact)
{
    int digitsCount = 0;
    int operatorsCount = 0;
    bool negativeSign = false;

    input = stripNonOperatorsOrNumbers(input);

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


static void TempChatBotCommandRunner(std::istream &input)
{
    ePlayer *player = sn_consoleUser();
    tString functionName(tConfItemBase::lastLoadCommandName);

    if (functionName.empty())
    {
        con << "Function name is empty.\n";
        return;
    }

    if (!player || !player->netPlayer)
    {
        con << "No player to run this command.\n";
        return;
    }

    tString inputStr;
    inputStr.ReadLine(input);
    inputStr = tColoredString::RemoveColors(inputStr).TrimWhitespace();

    eChatBot &bot = eChatBot::getInstance();

    auto it = bot.functionMap.find(functionName.ToLower());
    if (it != bot.functionMap.end())
    {
        eChatBot::ChatFunction &func = it->second;

        bot.Params().triggeredBy = player->netPlayer;
        bot.Params().triggerType = ResponseType::FUNC;
        bot.Params().matchedTrigger = functionName;

        tString output;
        output << func(inputStr);

        con << output << "\n";
    }
    else
    {
        con << "No function found for '" << functionName << "'\n";
    }
}

void eChatBot::LoadChatCommandConfCommands()
{
    if (chatBotCommandConfItems == nullptr)
        chatBotCommandConfItems = new TempConfItemManager();

    if (!chatBotCommandConfItems)
        return;

    chatBotCommandConfItems->DeleteConfitems();

    for (auto funcionPair : functionMap)
    {
        tString command;
        command << funcionPair.first;
        tToUpper(command);
        chatBotCommandConfItems->StoreConfitem(new tConfItemFunc(command, &TempChatBotCommandRunner));
    }
}

const std::vector<ChatBotColumnMapping> eChatBotStats::eChatBotStatsMappings =
{
    {"hackermans", "TEXT PRIMARY KEY",
        [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats){ sqlite3_bind_text(stmt, col++, "HACKERMANS", -1, SQLITE_STATIC); },
        [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats) { col++; }
    },

    {"total_uptime", "REAL",
        [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats) { sqlite3_bind_double(stmt, col++, stats.total_up_time + tSysTimeFloat()); },
        [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats) { stats.total_up_time = sqlite3_column_double(stmt, col++); }
    },

    {"total_messages_read", "INTEGER",
        [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats) { sqlite3_bind_int(stmt, col++, stats.total_messages_read); },
        [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats) { stats.total_messages_read = sqlite3_column_int(stmt, col++); }
    },

    {"total_messages_sent", "INTEGER",
        [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats) { sqlite3_bind_int(stmt, col++, stats.total_messages_sent); },
        [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats) { stats.total_messages_sent = sqlite3_column_int(stmt, col++); }
    },

    {"last_banned", "BIGINT",
        [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats) { sqlite3_bind_int(stmt, col++, static_cast<sqlite3_int64>(stats.last_banned)); },
        [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats) { stats.last_banned  = static_cast<time_t>(sqlite3_column_int64(stmt, col++)); }
    },

    {"times_banned", "INTEGER",
        [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats) { sqlite3_bind_int(stmt, col++, stats.times_banned); },
        [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats) { stats.times_banned = sqlite3_column_int(stmt, col++); }
    },

    {"times_banned_today", "INTEGER",
        [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats) { sqlite3_bind_int(stmt, col++, stats.times_banned_today); },
        [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats) { stats.times_banned_today = sqlite3_column_int(stmt, col++); }
    },
};

