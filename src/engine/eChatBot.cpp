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

bool se_playerTriggerMessagesReactToLocal = false;
static tConfItem<bool> se_playerTriggerMessagesReactToLocalConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_REACT_TO_LOCAL", se_playerTriggerMessagesReactToLocal);

static tString se_playerTriggerMessagesKillVerifiedTriggers = tString("wd,nice,wp,gj,$diedother,annoying,n1");
static tConfItem<tString> se_playerTriggerMessagesKillVerifiedTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_KILLED_VERIFIED_TRIGGERS", se_playerTriggerMessagesKillVerifiedTriggers);

static tString se_playerTriggerMessagesDiedByVerifiedTriggers("$died");
static tConfItem<tString> se_playerTriggerMessagesDiedByVerifiedTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DIED_BY_VERIFIED_TRIGGERS", se_playerTriggerMessagesDiedByVerifiedTriggers);

tString se_playerMessageEnabledPlayers = tString("");
static tConfItem<tString> se_playerMessageEnabledPlayersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_ENABLED_PLAYERS", se_playerMessageEnabledPlayers);

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

static REAL se_playerMessageBanFuncWaitTime = 4;
static tConfItem<REAL> se_playerMessageBanFuncWaitTimeConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_BAN_WAIT_TIME", se_playerMessageBanFuncWaitTime);

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

int se_playerTriggerMessagesAcheivementsKillsChangeVal = 50;
static tConfItem<int> se_playerTriggerMessagesAcheivementsKillsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsKillsChangeVal);

int se_playerTriggerMessagesAcheivementsKillStreakChangeVal = 5;
static tConfItem<int> se_playerTriggerMessagesAcheivementsKillStreakChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLSTREAK_CURRENT_CHANGE_VAL", se_playerTriggerMessagesAcheivementsKillStreakChangeVal);

int se_playerTriggerMessagesAcheivementsMaxKillStreak = 1;
static tConfItem<int> se_playerTriggerMessagesAcheivementsMaxKillStreakConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLSTREAK_MAX_CHANGE_VAL", se_playerTriggerMessagesAcheivementsMaxKillStreak);

int se_playerTriggerMessagesAcheivementsChatsChangeVal = 25;
static tConfItem<int> se_playerTriggerMessagesAcheivementsChatsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_CHATS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsChatsChangeVal);

int se_playerTriggerMessagesAcheivementsJoinsChangeVal = 10;
static tConfItem<int> se_playerTriggerMessagesAcheivementsJoinsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_JOINS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsJoinsChangeVal);

int se_playerTriggerMessagesAcheivementsBansChangeVal = 5;
static tConfItem<int> se_playerTriggerMessagesAcheivementsBansChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_BANS_CHANGE_VAL", se_playerTriggerMessagesAcheivementsBansChangeVal);

tString se_playerTriggerMessagesFuncLogFile("chatbotLogFunc.txt");
static tConfItem<tString> se_playerTriggerMessagesFuncLogFileConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_LOG_FILE", se_playerTriggerMessagesFuncLogFile);

int se_playerTriggerMessagesGuessGameGiveupAnnounceInterval = 10;
static tConfItem<int> se_playerTriggerMessagesGuessGameGiveupAnnounceIntervalConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_GUESS_GAME_GIVEUP_ANNOUNCE_INTERVAL", se_playerTriggerMessagesGuessGameGiveupAnnounceInterval);


std::vector<ePlayerNetID *> se_GetPlayerMessageEnabledPlayers()
{
    std::vector<ePlayerNetID *> enabledPlayers;
    tArray<tString> players = se_playerMessageEnabledPlayers.Split(",");

    for (int i = 0; i < players.Len(); i++)
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[0]) - 1);
        if (!local_p)
            continue;

        ePlayerNetID *netPlayer = local_p->netPlayer;
        if (!netPlayer)
            continue;

        enabledPlayers.push_back(netPlayer);
    }

    return enabledPlayers;
}

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


    input.erase(std::remove_if(input.begin(), input.end(), ::isspace), input.end());

    if (input.empty()) {
        return tString("Invalid input");
    }

    bool isNegative = false;
    size_t idx = 0;
    if (input[0] == '-') {
        isNegative = true;
        idx = 1;
    }

    std::string intPart, fracPart;
    while (idx < input.size() && input[idx] != '.') {
        if (!std::isdigit(input[idx])) {
            return tString("Invalid input");
        }
        intPart += input[idx++];
    }
    if (idx < input.size() && input[idx] == '.') {
        idx++;
        while (idx < input.size()) {
            if (!std::isdigit(input[idx])) {
                return tString("Invalid input");
            }
            fracPart += input[idx++];
        }
    }

    while (intPart.size() > 1 && intPart[0] == '0') {
        intPart.erase(0, 1);
    }
    if (intPart.empty()) {
        intPart = "0";
    }

    while (fracPart.size() > 1 && fracPart.back() == '0') {
        fracPart.pop_back();
    }

    bool originalIsNegativeZero = isNegative && intPart == "0" && (fracPart.empty() || fracPart == "0");

    int adderValue = se_playerMessageTriggersFuncAdderVal;

    int carry;
    if (originalIsNegativeZero) {
        carry = adderValue;
        isNegative = false;
    } else {
        carry = isNegative ? -adderValue : adderValue;
    }

    for (int i = fracPart.size() - 1; i >= 0; i--) {
        int digit = fracPart[i] - '0';
        int sum = digit + carry;

        carry = sum / 10;
        sum = sum % 10;
        if (sum < 0) {
            sum += 10;
            carry -= 1;
        }

        fracPart[i] = '0' + sum;
    }

    for (int i = intPart.size() - 1; i >= 0; i--) {
        int digit = intPart[i] - '0';
        int sum = digit + carry;

        carry = sum / 10;
        sum = sum % 10;
        if (sum < 0) {
            sum += 10;
            carry -= 1;
        }

        intPart[i] = '0' + sum;
    }


    while (carry != 0) {
        int sum = carry % 10;
        if (sum < 0) {
            sum += 10;
            carry = (carry / 10) - 1;
        } else {
            carry = carry / 10;
        }
        intPart = char('0' + sum) + intPart;
    }

    while (intPart.size() > 1 && intPart[0] == '0') {
        intPart.erase(0, 1);
    }

    while (fracPart.size() > 1 && fracPart.back() == '0') {
        fracPart.pop_back();
    }

    if (intPart == "0" && fracPart.empty()) {
        isNegative = false;
    }

    std::string result = isNegative ? "-" : "";
    result += intPart;
    if (!fracPart.empty()) {
        result += '.' + fracPart;
    }

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

    tString answer(str);
    if (answer.empty())
    {
        eChatBot &bot = eChatBot::getInstance();
        bot.Messager()->Params().abortOutput = true;
    }

    return answer;
}

tString sayFunc(tString message)
{
    return message;
}

tString reverseFunc(tString message)
{
    return message.Reverse();
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
            if (se_playerStats)
            {
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

    ePlayerNetID *lastTriggeredBy = eChatBot::getInstance().Messager()->Params().triggeredBy;
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
        statsTargetPlayer = bot.Messager()->Params().triggeredBy;
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

    bool showLabel             = !desiredStatChoosen || se_playerMessageTriggersLeaderboardLabels;
    bool lastSeenStat          = (statLabel == "Last Seen");
    bool playTimeStat          = (statLabel == "Play Time");
    bool specTimeStat          = (statLabel == "Spectate Time");
    bool currentKillStreakStat = (statLabel == "Current Kill Streak");
    bool nicknameStat          = (statLabel == "Nickname");

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

    bool symLinkFunc = bot.Messager()->Params().triggerType == ResponseType::SYM_FUNC;

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

    if (statLabel.empty() && stat == "last_active")
    {
        statLabel = "Last Active";
    }

    if (statLabel.empty())
    {
        output << "Stat not found! Usage: '"
               << bot.Messager()->Params().matchedTrigger
               << " "
               << "stat"
               << " player' "
               << PlayerData::getAvailableStatsStr();
        return output;
    }

    playerName = message.SubStr(pos+1).TrimWhitespace();

    if (playerName.empty())
    {
        statsTargetPlayer = bot.Messager()->Params().triggeredBy;
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

            if (!bot.Messager()->Params().pentalized_for_last_message)
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
        else if (statLabel == "Last Active")
        {
            if (statsTargetPlayer)
                statValue << st_GetFormatTime(statsTargetPlayer->LastActivity(), false);
            else
                statValue << "Not here";
        }
        else
            statValue << stats.getAnyValue(stat.TrimWhitespace());

        if (showCurrentSessionVal && stats.in_server && stat != "last_active")
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

    if (!bot.Messager()->Params().triggeredBy || !bot.Messager()->Params().triggeredBy->encryptVerified )
        return sayFunc(message);

    bot.Messager()->Params().abortOutput = true;

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


        if (bot.Messager()->Params().sendingPlayer)
        {
            se_NewChatMessage(bot.Messager()->Params().sendingPlayer, feedback)->BroadCast();
        }
        else
        {
            for (auto localNetPlayer : se_GetPlayerMessageEnabledPlayers())
            {
                se_NewChatMessage(localNetPlayer, feedback)->BroadCast();
                break;
            }
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
    bool symLinkFunc = bot.Messager()->Params().triggerType == ResponseType::SYM_FUNC;
    ePlayerNetID *triggeredBy = bot.Messager()->Params().triggeredBy;
    PlayerData &stats = ePlayerStats::getStats(triggeredBy);

    int pos = 0;
    tString desiredAction = message.ExtractNonBlankSubString(pos);

    if (desiredAction.empty())
    {
        output << "Desired action not found. "
               << "Usage: '"
               << bot.Messager()->Params().matchedTrigger
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
               << bot.Messager()->Params().matchedTrigger
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

tString whatsTheFunc(tString message)
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
        ePlayerNetID *targetPlayer = bot.Messager()->Params().triggeredBy;

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
        ePlayerNetID *targetPlayer = bot.Messager()->Params().triggeredBy;

        if (targetPlayer)
        {
            if (se_GetLocalPlayer()->createdTime() <= targetPlayer->createdTime())
            {
                output << "you joined before me";
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
            output << tColoredString::RemoveColors(connectedServer->GetName());
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
        ePlayerNetID *targetPlayer = bot.Messager()->Params().triggeredBy;

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
    else if (target.Contains("fps"))
    {
        int newfps = static_cast<int>(se_AverageFPS());

        output << "Current FPS: " << newfps;
    }

    return output;
}

tString consolidatePlayerStatsFunc(tString message)
{
    tString output;

    tString params = message.TrimWhitespace();

    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeredByPlayer = bot.Messager()->Params().triggeredBy;

    if (!triggeredByPlayer || !triggeredByPlayer->encryptVerified )
    {
        output << "You are not verified";
        return output;
    }

    return ePlayerStats::consolidatePlayerStats(params);
}

tString statsDeleteFunc(tString message)
{
    tString output;

    tString player = message.TrimWhitespace().ToLower();

    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeredByPlayer = bot.Messager()->Params().triggeredBy;

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
               << bot.Messager()->Params().matchedTrigger
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

            std::string trigger = toLower(bot.Messager()->Params().matchedTrigger.stdString());
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

    // Force public message
    bot.Messager()->Params().preAppend = "";

    output << "> " << message;

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

    std::string lastMatchedTrigger(bot.Messager()->Params().matchedTrigger.stdString());

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
            statsTargetPlayer = bot.Messager()->Params().triggeredBy;
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

tString flipRepeatedCharactersFunc(tString message)
{
    std::string input(message.stdString());
    std::unordered_map<char, std::vector<int>> charIndices;

    for (size_t i = 0; i < input.size(); ++i)
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
    ePlayerNetID *triggeredBy = bot.Messager()->Params().triggeredBy;

    if (!triggeredBy)
        return output;

    ePlayerNetID *statsTargetPlayer = triggeredBy->lastDiedByPlayerBanFunc;

    if (triggeredBy->lastBannedPlayerTime != 0 && tSysTimeFloat() - triggeredBy->lastBannedPlayerTime < 5) {
        return output;
    }

    if (!statsTargetPlayer || ((tSysTimeFloat() - triggeredBy->lastDiedByTime) > se_playerMessageBanFuncWaitTime )  )
        return output;

    PlayerData &triggeredByStats = ePlayerStats::getStats(triggeredBy);

    if ((tSysTimeFloat() - triggeredBy->lastDiedByTime) < se_playerMessageBanFuncWaitTime )
        triggeredByStats.banned_a_player_this_round = false;

    ePlayerNetID *sendingPlayer = bot.Messager()->Params().sendingPlayer;
    if (triggeredByStats.banned_a_player_this_round)
    {
        bot.Messager()->ResetParams();
        bot.Messager()->SetInputParams(statsTargetPlayer, tString("$alreadybanned"), true);
        bot.Messager()->FindTriggeredResponse();
        bot.Messager()->Params().sendingPlayer = sendingPlayer;

        return bot.Messager()->Params().response;
    }
    else
    {
        triggeredByStats.banned_a_player_this_round = true;
        triggeredByStats.bans_given++;
        triggeredByStats.thisSession().bans_given++;
        triggeredBy->lastBannedPlayerTime = tSysTimeFloat();
    }

    PlayerData &bannedPlayerStats = ePlayerStats::getStats(statsTargetPlayer);
    bannedPlayerStats.times_banned++;
    bannedPlayerStats.thisSession().times_banned++;


    ePlayerStatsAcheivements::performAction(bannedPlayerStats, ePlayerStatsAcheivements::AcheivementsTypes::BANS);

    bot.Messager()->ResetParams();
    bot.Messager()->SetInputParams(statsTargetPlayer, tString("$banned"), true);
    bot.Messager()->FindTriggeredResponse();
    bot.Messager()->Params().sendingPlayer = sendingPlayer;

    return bot.Messager()->Params().response;
}

tString unvalidatedSayFunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    bot.Messager()->Params().validateOutput = false;
    return message;
}

tString timerFunc(tString message)
{
    static std::chrono::steady_clock::time_point timer;
    static bool isInitialized = false;

    eChatBot &bot = eChatBot::getInstance();
    tString output;

    if (message.empty())
    {
        if (isInitialized)
        {
            auto now = std::chrono::steady_clock::now();
            auto duration = now - timer;
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            tString formattedDifference = st_GetFormatTime(milliseconds / 1000.0, false, false);
            output << formattedDifference
                   << " elapsed since timer started.";
        }
        else
        {
            output << "Timer not started! Usage: '"
                   << bot.Messager()->Params().matchedTrigger
                   << " start | stop | reset'";
        }
    }
    else if (message == "start")
    {
        if (!isInitialized)
        {
            timer = std::chrono::steady_clock::now();
            output << "Timer started.";
            isInitialized = true;
        }
        else
        {
            auto now = std::chrono::steady_clock::now();
            auto duration = now - timer;
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            tString formattedDifference = st_GetFormatTime(milliseconds / 1000.0, false, false);
            output << "Timer was already started. "
                   << formattedDifference
                   << " elapsed. Use stop or reset!";
        }
    }
    else if (message == "stop")
    {
        if (isInitialized)
        {
            auto now = std::chrono::steady_clock::now();
            auto duration = now - timer;
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            tString formattedDifference = st_GetFormatTime(milliseconds / 1000.0, false, false);
            output << "Timer stopped. "
                   << formattedDifference
                   << " elapsed.";
            isInitialized = false;
        }
        else
        {
            output << "Timer not started! Usage: '"
                   << bot.Messager()->Params().matchedTrigger
                   << " stop | start | reset'";
        }

    }
    else if (message == "reset" || message == "restart")
    {
        auto now = std::chrono::steady_clock::now();
        if (isInitialized)
        {
            auto duration = now - timer;
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            tString formattedDifference = st_GetFormatTime(milliseconds / 1000.0, false, false);
            output << "Timer stopped. "
                   << formattedDifference
                   << " elapsed. ";
        }

        timer = now;
        output << "Timer reset.";
        isInitialized = true;
    }
    else
    {
        output << "Invalid input! Usage: '"
               << bot.Messager()->Params().matchedTrigger
               << " stop | start | reset'";
    }

    return output;
}

tString logFunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeredBy = bot.Messager()->Params().triggeredBy;

    if (!triggeredBy)
        return tString("No player context available.");

    FileManager fileManager(se_playerTriggerMessagesFuncLogFile, tDirectories::Var());

    message = message.TrimWhitespace();

    if (message.empty())
        return tString("Usage: 'store <message>' to log a message or 'list [page]' to view logs.");

    int pos = 0;
    tString command = message.ExtractNonBlankSubString(pos);

    if (command.ToLower() == "list")
    {
        tString param = message.ExtractNonBlankSubString(pos);
        int requestedPage = 1;

        if (!param.empty())
        {
            if (param.isNumber())
                requestedPage = atoi(param.c_str());
            else
                return tString("Invalid page number.");
        }

        tArray<tString> lines = fileManager.Load();
        int totalLogs = lines.Len();

        if (totalLogs == 0)
            return tString("No logs available.");

        const size_t maxMessageLength = se_SpamMaxLen - 1;
        std::vector<tString> pages;
        tString currentPage;
        size_t currentPageLength = 0;

        for (int i = 0; i < totalLogs; ++i)
        {
            tString line = lines[i];
            tArray<tString> logParts = line.Split(",");

            tString formattedLogEntry;

            if (logParts.Len() >= 3)
            {
                tString timeStr = logParts[0];
                tString playerName = logParts[1];
                tString logMessage = logParts[2];

                for (int j = 3; j < logParts.Len(); ++j)
                {
                    logMessage << "," << logParts[j];
                }

                formattedLogEntry << timeStr << " " << playerName << ": " << logMessage.TrimWhitespace();
            }
            else
            {
                formattedLogEntry = line;
            }

            size_t formattedLogEntryLength = formattedLogEntry.Len();
            size_t separatorLength = (currentPageLength == 0) ? 0 : 2;

            if (currentPageLength + separatorLength + formattedLogEntryLength > maxMessageLength)
            {
                if (currentPageLength > 0)
                    pages.push_back(currentPage);

                currentPage = formattedLogEntry;
                currentPageLength = formattedLogEntryLength;
            }
            else
            {
                if (currentPageLength > 0)
                {
                    currentPage << ", ";
                    currentPageLength += 2;
                }
                currentPage << formattedLogEntry;
                currentPageLength += formattedLogEntryLength;
            }
        }

        if (currentPageLength > 0)
            pages.push_back(currentPage);

        int totalPages = pages.size();

        if (requestedPage > totalPages)
            requestedPage = totalPages;

        if (requestedPage <= 0)
            requestedPage = 1;

        tString output;
        output << "Logs (" << requestedPage << "/" << totalPages << "): " << pages[requestedPage - 1];

        return output;
    }
    else if (command.ToLower() == "store")
    {
        tString logMessage = message.SubStr(pos).TrimWhitespace();
        if (logMessage.empty())
            return tString("No message passed to log!");

        tString playerName = triggeredBy->GetName();

        tString timeStr = st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S]");

        tString logEntry;
        logEntry << timeStr << "," << playerName << "," << logMessage.TrimWhitespace();

        fileManager.Write(logEntry);
        return tString("Message logged.");
    }
    else
    {
        return tString("Unknown command. Usage: 'store <message>' to log a message or 'list [page]' to view logs.");
    }
}

int randomValue = 0;
bool gameStarted = false;
int minRange = 1;
int maxRange = 100;
int attempts = 0;

time_t startTime;
tString guessGameFunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *player = bot.Messager()->Params().triggeredBy;

    if (!player)
        return tString("No player context available.");

    tString command = message.TrimWhitespace();

    if (command.empty())
    {
        tString output;
        output << "Usage: '"
               << bot.Messager()->Params().matchedTrigger
               << " start min-max' to begin, '"
               << bot.Messager()->Params().matchedTrigger
               << " number' to make a guess, or '"
               << bot.Messager()->Params().matchedTrigger
               << " give up' to reveal the number.";
        return output;
    }
    else if (command.ToLower().StartsWith("start"))
    {
        if (!gameStarted)
        {
            int pos = 0;
            command.ExtractNonBlankSubString(pos);
            tString rangeStr = command.SubStr(pos).TrimWhitespace();

            minRange = 1;
            maxRange = 100;

            if (!rangeStr.empty())
            {
                tArray<tString> rangeParts = rangeStr.Split("-");
                if (rangeParts.Len() == 2 && rangeParts[0].isNumber() && rangeParts[1].isNumber())
                {
                    int tempMin = atoi(rangeParts[0].c_str());
                    int tempMax = atoi(rangeParts[1].c_str());
                    if (tempMin < tempMax)
                    {
                        minRange = tempMin;
                        maxRange = tempMax;
                    }
                    else
                    {
                        return tString("Invalid range. Please ensure that min < max.");
                    }
                }
                else
                {
                    return tString("Invalid range format. Use 'min-max', e.g., '1-500'.");
                }
            }

            randomValue = rand() % (maxRange - minRange + 1) + minRange;
            gameStarted = true;
            attempts = 0;
            startTime = time(NULL);

            tString output;
            output << "I'm thinking of a number between " << minRange << " and " << maxRange << ". Try to guess it!";
            return output;
        }
        else
        {
            tString output;
            output << "The game is already started! Try to guess the number or type '"
                   << bot.Messager()->Params().matchedTrigger
                   << " give up' to reveal it.";
            return output;
        }
    }
    else if (command.ToLower() == "give up" || command.ToLower() == "quit")
    {
        if (gameStarted)
        {
            time_t endTime = time(NULL);
            int elapsedTime = static_cast<int>(difftime(endTime, startTime));

            tString revealMessage;
            revealMessage << "The number was " << randomValue
                          << ". Total attempts: " << attempts
                          << ". Time taken: " << elapsedTime << " second";
            if (elapsedTime != 1)
                revealMessage << "s";
            revealMessage << ". Better luck next time!";
            gameStarted = false;
            return revealMessage;
        }
        else
        {
            tString output;
            output << "There is no active game to give up. You can start a new game using '"
                   << bot.Messager()->Params().matchedTrigger
                   << " start'.";
            return output;
        }
    }
    else
    {
        if (!gameStarted)
        {
            tString output;
            output << "You need to start the game first using '"
                   << bot.Messager()->Params().matchedTrigger
                   << " start'.";
            return output;
        }

        if (!command.isNumber())
        {
            tString output;
            output << "Invalid input. Please enter a number between " << minRange << " and " << maxRange << ".";
            return output;
        }

        int guess = atoi(command.c_str());

        if (guess < minRange || guess > maxRange)
        {
            tString output;
            output << "Your guess is out of range. Please enter a number between " << minRange << " and " << maxRange << ".";
            return output;
        }

        attempts++;

        tString output;

        if (guess < randomValue)
        {
            output << "Too low! Attempts: " << attempts << ".";
        }
        else if (guess > randomValue)
        {
            output << "Too high! Attempts: " << attempts << ".";
        }
        else
        {
            time_t endTime = time(NULL);
            int elapsedTime = static_cast<int>(difftime(endTime, startTime));

            tString successMessage;
            successMessage << "Congratulations, " << player->GetName()
                           << "! You guessed the number in " << attempts << " attempt";
            if (attempts != 1)
                successMessage << "s";
            successMessage << " and " << elapsedTime << " second";
            if (elapsedTime != 1)
                successMessage << "s";
            successMessage << "!";
            gameStarted = false;
            return successMessage;
        }

        if (se_playerTriggerMessagesGuessGameGiveupAnnounceInterval > 0 &&
            attempts % se_playerTriggerMessagesGuessGameGiveupAnnounceInterval == 0)
        {
            output << " Try again or type '"
                   << bot.Messager()->Params().matchedTrigger
                   << " give up' to reveal the number.";
        }
        else
        {
            output << " Try again.";
        }

        return output;
    }
}

tString rpsGameFunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *player = bot.Messager()->Params().triggeredBy;

    if (!player)
        return tString("No player context available.");

    tString playerChoice = message.TrimWhitespace().ToLower();

    if (playerChoice != "rock" && playerChoice != "paper" && playerChoice != "scissors")
    {
        return tString("Invalid choice. Please choose 'rock', 'paper', or 'scissors'.");
    }

    srand(static_cast<unsigned int>(time(0)));
    const char* choices[] = { "rock", "paper", "scissors" };
    tString botChoice;
    botChoice << choices[rand() % 3];

    if (playerChoice == botChoice)
    {
        return tString("It's a tie! We both chose ") + botChoice + ".";
    }
    else if ((playerChoice == "rock" && botChoice == "scissors") ||
             (playerChoice == "paper" && botChoice == "rock") ||
             (playerChoice == "scissors" && botChoice == "paper"))
    {
        return tString("You win! You chose ") + playerChoice + " and I chose " + botChoice + ".";
    }
    else
    {
        return tString("You lose! You chose ") + playerChoice + " and I chose " + botChoice + ".";
    }
}


void eChatBot::InitChatFunctions()
{
    Functions()->RegisterFunction("$numbadderfunc", numberAdderFunc);
    Functions()->RegisterFunction("$numbcalcfunc", numberCalcFunc);
    Functions()->RegisterFunction("$sayfunc", sayFunc);
    Functions()->RegisterFunction("$revfunc", reverseFunc);
    Functions()->RegisterFunction("$statsfunc", statsFunc);
    Functions()->RegisterFunction("$nicknamefunc", nicknameFunc);
    Functions()->RegisterFunction("$KDfunc", playerKDFunc);
    Functions()->RegisterFunction("$leaderboardfunc", leaderboardFunc);
    Functions()->RegisterFunction("$exactstatfunc", exactStatFunc);
    Functions()->RegisterFunction("$masterfunc", masterFunc);
    Functions()->RegisterFunction("$hidestatfunc", hideStatFunc);
    Functions()->RegisterFunction("$whatsthefunc", whatsTheFunc);
    Functions()->RegisterFunction("$consolidatestatsfunc", consolidatePlayerStatsFunc);
    Functions()->RegisterFunction("$statsdeletefunc", statsDeleteFunc);
    Functions()->RegisterFunction("$searchfunc", searchFunc);
    Functions()->RegisterFunction("$sayoutloudfunc", sayOutLoudFunc);
    Functions()->RegisterFunction("$chatscrollsearchfunc", chatScrollSearchFunc);
    Functions()->RegisterFunction("$fliprepeatedcharsfunc", flipRepeatedCharactersFunc);
    Functions()->RegisterFunction("$banfunc", banFunc);
    Functions()->RegisterFunction("$unvalidatedsayfunc", unvalidatedSayFunc);
    Functions()->RegisterFunction("$timerfunc", timerFunc);
    Functions()->RegisterFunction("$logfunc", logFunc);
    Functions()->RegisterFunction("$guessgamefunc", guessGameFunc);
    Functions()->RegisterFunction("$rpsgamefunc", rpsGameFunc);
}

void eChatBot::LoadChatTriggers()
{
    gHelperUtility::Debug("eChatBot","Loading chat triggers...");
    FileManager fileManager(se_playerTriggerMessagesFile, tDirectories::Var());
    tArray<tString> lines = fileManager.Load();

    data.chatTriggers.clear();

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

                eChatBotData::ChatTrigger chatTrigger;

                chatTrigger.trigger = trigger;
                chatTrigger.responses = std::vector<tString>(responsesArray.begin(), responsesArray.end());
                chatTrigger.extraDelay = atof(parts[2]);
                chatTrigger.exact = atoi(parts[3]) == 1;

                // Preprocess
                chatTrigger.trimmedTrigger = trigger.TrimWhitespace();

                chatTrigger.isChatNameTrigger = (trigger.Contains("$p0 ") || trigger.EndsWith("$p0") || trigger == "$p0");
                chatTrigger.triggerWithoutName = chatTrigger.isChatNameTrigger ? trigger.Replace("$p0", "").TrimWhitespace() : tString("");

                chatTrigger.isSpecialTrigger = trigger.StartsWith("$");

                data.chatTriggers.push_back(chatTrigger);
            }
        }
        else
        {
            con << tThemedTextBase.LabelText("LoadChatTriggers")
                << "Error:\n"
                << tThemedTextBase.ErrorColor()  << "Malformed line at index "
                << tThemedTextBase.ItemColor()   << i + 1 << ": "
                << lines[i] << "\n";
            continue;
        }
    }
}

bool eChatBot::ShouldAnalyze()
{
    tArray<tString> players = se_playerMessageEnabledPlayers.Split(",");

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

bool eChatBot::InitiateAction(ePlayerNetID *triggeredBy, tString inputMessage, bool eventTrigger, tString preAppend)
{
    bool initiated;
    eChatBot &bot = eChatBot::getInstance();

    if (!bot.ShouldAnalyze())
        return initiated;

    if (!eventTrigger)
        bot.Stats().total_messages_read++;

    gHelperUtility::Debug("eChatBot","Received Input:", inputMessage);

    std::vector<ePlayerNetID *> sentByPlayers;

    for (auto localNetPlayer : se_GetPlayerMessageEnabledPlayers())
    {
        // Don't use the same player twice
        if (std::find(sentByPlayers.begin(), sentByPlayers.end(), localNetPlayer) != sentByPlayers.end())
            continue;

        bot.Messager()->ResetParams();
        bot.Messager()->SetInputParams(triggeredBy, inputMessage, eventTrigger, preAppend);

        bot.Messager()->Params().sendingPlayer = localNetPlayer;
        sentByPlayers.push_back(localNetPlayer);

        bot.Messager()->FindTriggeredResponse();

        if (!bot.Messager()->Params().abortOutput)
        {
            gHelperUtility::Debug("eChatBot", "Sending message:", bot.Messager()->Params().response);
            initiated = true;
            bot.Messager()->Send();
        }
        else if (eventTrigger)
            gHelperUtility::Debug("eChatBot", "No trigger set for '" + inputMessage.stdString() + "' Set one with 'PLAYER_MESSAGE_TRIGGERS_ADD'\n");
        else if (bot.Messager()->Params().matchFound)
            gHelperUtility::Debug("eChatBot", "Aborting output..");
        else
            gHelperUtility::Debug("eChatBot", "No match found..");

        // If function, no need for multiple players
        if (bot.Messager()->Params().triggerType != ResponseType::NORMAL)
            break;
    }

    return initiated;
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

void eChatBotMessager::SetInputParams(ePlayerNetID *triggeredBy, tString inputMessage, bool eventTrigger, tString preAppend)
{
    Params().triggeredBy  = triggeredBy;
    Params().inputMessage = inputMessage;
    Params().eventTrigger = eventTrigger;
    Params().preAppend    = preAppend;
}

void eChatBotMessager::SetOutputParams(tString &response, REAL &delay, ePlayerNetID *sendingPlayer)
{
    Params().response       = response;
    Params().delay          = delay;
    Params().sendingPlayer  = sendingPlayer;
    Params().validateOutput = true;
}

void eChatBotMessager::ResetParams()
{
    Params().response.Clear();
    Params().preAppend.Clear();
    Params().triggeredByName.Clear();

    Params().delay            = 0;
    Params().currentPartIndex = 0;

    Params().validateOutput   = true;
    Params().abortOutput      = false;
    Params().eventTrigger     = false;
    Params().matchFound       = false;

    Params().sendingPlayer    = nullptr;
    Params().triggeredBy      = nullptr;

    Params().messageParts.clear();

    Params().triggerType      = ResponseType::NORMAL;
}

void eChatBotMessager::FindTriggeredResponse()
{
    Params().inputMessage = tColoredString::RemoveColors(Params().inputMessage).TrimWhitespace();
    tString lowerMessage(Params().inputMessage);

    if (lowerMessage.empty())
        return;

    tToLower(lowerMessage);

    for (const auto &chatTrigger : data->chatTriggers)
    {
        bool match = false;
        bool exact = chatTrigger.exact;

        if (Params().eventTrigger)
            exact = true;

        const tString& trigger = chatTrigger.trigger;

        ePlayerNetID *potentialSender = nullptr;
        if (!Params().eventTrigger && chatTrigger.isChatNameTrigger) // Name trigger (contains $p0)
        {
            std::vector<ePlayerNetID*> enabledPlayers = Params().sendingPlayer
                ? std::vector<ePlayerNetID*>{Params().sendingPlayer}
                : se_GetPlayerMessageEnabledPlayers();

            tString triggerWithoutName = chatTrigger.triggerWithoutName;

            for (auto localNetPlayer : enabledPlayers)
            {
                tString ourName = localNetPlayer->GetName().ToLower();
                tArray<tString> nameParts = ourName.Split(" ");

                if (!triggerWithoutName.empty() && !lowerMessage.Contains(triggerWithoutName))
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
        else if (!Params().eventTrigger && chatTrigger.isSpecialTrigger) // Starts with "$" and is not eventTrigger
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

        if (!match)
            continue;

        Params().matchedTrigger = chatTrigger.trimmedTrigger;

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

        Params().delay = chatTrigger.extraDelay;

        // vector of possible responses
        std::vector<tString> possibleResponses = chatTrigger.responses;

        tString sanitizedTriggeredByName = Params().triggeredByName.Replace("$", "\\$");

        // random response from the vector
        tString chosenResponse = possibleResponses[rand() % possibleResponses.size()];
        chosenResponse         = chosenResponse.Replace("$p1", sanitizedTriggeredByName);

        if (Params().sendingPlayer) {
            chosenResponse.RecomputeLength();
            tString ourName = Params().sendingPlayer->GetName();
            chosenResponse = chosenResponse.Replace("$p0", ourName);
        }

        int dollarPos = 0;
        while ((dollarPos = chosenResponse.StrPos(dollarPos, "$")) != -1)
        {
            if (dollarPos > 0 && chosenResponse[dollarPos - 1] == '\\')
            {
                chosenResponse.RemoveSubStr(dollarPos - 1, 1);
                dollarPos--;
                dollarPos++;
                continue;
            }

            int openParenPos = chosenResponse.StrPos(dollarPos + 1, "(");
            int closeParenPos = -1;

            tString functionName;
            tString functionInput;

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
                if (endPos == -1)
                    endPos = chosenResponse.Len() - 1;

                functionName = chosenResponse.SubStr(dollarPos, endPos - dollarPos);
            }

            if (functionName == "$p1" || functionName == "$val1")
            {
                dollarPos += functionName.Len();
                continue;
            }

            if (!data->functions->IsValidFunction(functionName))
            {
                dollarPos += functionName.Len();
                continue;
            }

            Params().triggerType = ResponseType::FUNC;

            tString finalFunctionInput(functionInput);

            if (finalFunctionInput.empty())
            {
                tString toReplace = Params().matchedTrigger + " ";
                finalFunctionInput = Params().inputMessage.Replace(toReplace, "");

                toReplace = Params().matchedTrigger;
                finalFunctionInput = finalFunctionInput.Replace(toReplace, "");
            }
            else
            {
                Params().triggerType = ResponseType::SYM_FUNC;

                tString extraParams(Params().inputMessage);
                if (!extraParams.empty())
                    finalFunctionInput << tString(" ") << extraParams;

                tString toReplace = Params().matchedTrigger + " ";
                finalFunctionInput = finalFunctionInput.Replace(toReplace, "");

                toReplace = Params().matchedTrigger;
                finalFunctionInput = finalFunctionInput.Replace(toReplace, "");
            }

            finalFunctionInput = tColoredString::RemoveColors(finalFunctionInput);

            tString result = data->functions->ExecuteFunction(functionName, finalFunctionInput);

            tString functionPattern = functionName;
            if (!functionInput.empty())
                functionPattern += tString("(") + functionInput + tString(")");

            if (!result.empty())
            {
                chosenResponse = chosenResponse.Replace(functionPattern, result);
            }
            else
            {
                chosenResponse = "";
                break;
            }

            dollarPos = dollarPos + result.Len();
        }


        if (!chosenResponse.empty())
            Params().matchFound = true;
        else
            Params().abortOutput = true;

        if (Params().matchFound)
            gHelperUtility::Debug("eChatBot","Responding to trigger: " + Params().matchedTrigger.stdString());

        if (se_playerMessageTriggersChatFunctionsOnly && Params().matchFound && Params().triggerType == ResponseType::NORMAL )
        {
            gHelperUtility::Debug("eChatBot","PLAYER_MESSAGE_TRIGGER_CHAT_FUNCTIONS_ONLY set to TRUE, aborting output..");
            Params().abortOutput = true;
            return;
        }

        if (Params().validateOutput)
            validateOutput(chosenResponse);

        Params().response = chosenResponse;

        return;

    }

    Params().abortOutput = true;
}

bool eChatBotMessager::ResendMessage(tString blockedMessage)
{

    if (Params().pentalized_for_last_message)
        Params().delay += 3;

    if (Params().pentalized_for_last_message_prefix)
    {
        tString prefix;
        prefix << randomStr("!@#$%^&*()_+=-",3)
                << "| ";
        addPrefix(Params().response, prefix);
    }

    return Send();
}
bool eChatBotMessager::Send()
{
    if (!helperConfig::sghuk)
        return false;

    if (!Params().sendingPlayer)
    {
        gHelperUtility::Debug("eChatBot","No sending player set. Attemping to set one");

        tArray<tString> players = se_playerMessageEnabledPlayers.Split(",");

        for (int i = 0; i < players.Len(); i++)
        {
            ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[0]) - 1);
            if (!local_p)
                continue;

            ePlayerNetID *netPlayer = local_p->netPlayer;
            if (!netPlayer)
                continue;

            Params().sendingPlayer = netPlayer;
            break;
        }

    }

    if ((!Params().sendingPlayer) || (!tIsEnabledForPlayer(se_playerMessageEnabledPlayers, Params().sendingPlayer->pID + 1))) {
        gHelperUtility::Debug("eChatBot","No sending player set. Aborting");
        return false;
    }

    REAL extraDelay = 0;
    tString messageToSend = Params().response;

    bool forceSpecialDelay = Params().delay < -5;

    REAL totalDelay = 0.0;
    int messageLength = messageToSend.Len();

    if (messageLength > se_playerTriggerMessagesSpamMaxlen)
    {
        gHelperUtility::Debug("eChatBot","messageToSend cut because length > " +
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

    const size_t maxMessageLength = se_SpamMaxLen - 1;
    tArray<tString> messageParts = messageToSend.SplitBySize(maxMessageLength, true);

    int startPartIndex = static_cast<int>(std::max(static_cast<size_t>(0), Params().currentPartIndex));
    int numberOfParts = messageParts.Len() - startPartIndex;


    bool scheduled = false;
    for (int i = 0; i < messageParts.Len(); i++)
    {
        tString partToSend = messageParts[i];
        partToSend.RecomputeLength();

        if (messageParts.Len() > 1)
            gHelperUtility::Debug("eChatBot","partToSend", partToSend);

        if (!Params().preAppend.empty())
            partToSend = Params().preAppend + partToSend;

        REAL delayForPart = totalDelay/numberOfParts;

        eChatBotData::MessagePart part(partToSend, delayForPart,
                                       !forceSpecialDelay && se_playerMessageTriggersChatFlag,
                                       se_playerMessageTriggersChatFlagStartMult);

        Params().messageParts.push_back(part);
        scheduled = true;
    }

    if (scheduled)
        Stats().total_messages_sent++;

    ScheduleMessageParts();

    return scheduled;
}

bool eChatBotMessager::ScheduleMessageParts()
{
    gTaskScheduler.ClearQueueIfOverloaded(se_playerTriggerMessagesQueueMaxOverloadedSize);

    ePlayerNetID *player = Params().sendingPlayer;
    bool scheduled = false;

    size_t currentPartIndex = Params().currentPartIndex;
    size_t totalParts = Params().messageParts.size();

    for (size_t i = currentPartIndex; i < totalParts; ++i)
    {
        const auto& part = Params().messageParts[i];
        const tString& message = part.message;

        gTaskScheduler.enqueueChain([player, &part, &message]() {
            REAL flagDelay = part.delay * part.chatFlagPercentage;
            REAL messageDelay = part.delay - flagDelay;

            if (part.useChatFlag)
            {
                gTaskScheduler.schedule(
                    "playerMessageTask", flagDelay,
                    [player, messageDelay, &message, &part]
                    {
                        player->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, true);
                        ePlayerNetID::Update();

                        gTaskScheduler.schedule("playerMessageSetChatFlagFalse", messageDelay,
                        [player, &message, &part]
                        {
                            player->Chat(message, true);
                            player->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, false);
                            ePlayerNetID::Update();
                        }, 0, true);
                    }, 0, true);
            }
            else
            {
                gTaskScheduler.schedule(
                    "playerMessageTask", part.delay,
                    [player, &message] { player->Chat(message, true); }, 0, true);
            }
        });

        scheduled = true;
    }

    return scheduled;
}


REAL eChatBotMessager::determineReadingDelay(tString message)
{
    return se_playerMessageSmartDelayReadingWPM > 0 ? calculateResponseSmartDelay(message, se_playerMessageSmartDelayReadingWPM) : 0;
}

REAL eChatBotMessager::calculateResponseSmartDelay(tString response, REAL wpm)
{
    REAL delay = 0;

    int chatLen = response.Len();
    REAL delayPerChar = 60.0 / (5 * wpm); // in seconds
    delay = chatLen * delayPerChar;
    return delay;
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
    tArray<tString> responsesArray = parts[1].Split(";");

    REAL extraDelay = atof(parts[2].c_str());
    bool exact = atoi(parts[3].c_str()) == 1;

    if (triggersArray.Len() == 0 || responsesArray.Len() == 0)
    {
        con << "Error: Trigger and responses cannot be empty.\n";
        return;
    }

    std::vector<tString> responses(responsesArray.begin(), responsesArray.end());

    eChatBot &bot = eChatBot::getInstance();
    FileManager fileManager(se_playerTriggerMessagesFile, tDirectories::Var());

    tString lineToWrite = parts[0] + "," + parts[1] + "," + parts[2] + "," + parts[3];
    fileManager.Write(lineToWrite);

    for (auto &trigger : triggersArray)
    {
        trigger = trigger.ToLower();

        if (trigger.empty() || responses.empty())
        {
            con << "Error: Trigger and responses cannot be empty.\n";
            return;
        }

        eChatBotData::ChatTrigger chatTrigger;
        chatTrigger.trigger = trigger;
        chatTrigger.responses = responses;
        chatTrigger.extraDelay = extraDelay;
        chatTrigger.exact = exact;

        bot.data.chatTriggers.push_back(chatTrigger);
    }

    con << "Trigger, Response, Extra Delay, Exact?\n";
    con << "Added: " << params << "\n";
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
    bot.data.chatTriggers.clear();
    bot.LoadChatTriggers();
}

static void ListChatTriggers(std::istream &s)
{
    eChatBot &chatBot = eChatBot::getInstance();

    if (se_playerTriggerMessages && !chatBot.data.chatTriggers.empty())
    {
        con << "Listing all loaded chat triggers:\n";
        con << "Line) Trigger, Response, Extra Delay, Exact?\n";

        int i = 1;
        for (const auto &chatTrigger : chatBot.data.chatTriggers)
        {
            const tString &trigger = chatTrigger.trigger;
            const std::vector<tString> &responses = chatTrigger.responses;
            REAL extraDelay = chatTrigger.extraDelay;
            bool exact = chatTrigger.exact;


            std::string combinedResponses;
            for (const auto &response : responses)
            {
                if (!combinedResponses.empty())
                    combinedResponses += ";";

                combinedResponses += response;
            }

            con << i << ") "
                << trigger           << ", "
                << combinedResponses << ", "
                << extraDelay        << ", "
                << (exact ? "Yes" : "No")
                << "\n";
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
    bot.data.chatTriggers.clear();
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

    auto it = bot.Functions()->functionMap.find(functionName.ToLower());
    if (it != bot.Functions()->functionMap.end())
    {
        ChatFunction &func = it->second;

        bot.Messager()->Params().triggeredBy = player->netPlayer;
        bot.Messager()->Params().triggerType = ResponseType::FUNC;
        bot.Messager()->Params().matchedTrigger = functionName;

        tString output;
        output << func(inputStr);

        con << output << "\n";
    }
    else
    {
        con << "No function found for '" << functionName << "'\n";
    }
}

// Create executable console commands from chat functions
void eChatBot::LoadChatCommandConfCommands()
{
    if (data.chatBotCommandConfItems == nullptr)
        data.chatBotCommandConfItems = new TempConfItemManager();

    if (!data.chatBotCommandConfItems)
        return;

    data.chatBotCommandConfItems->DeleteConfitems();

    for (auto funcionPair : data.functions->functionMap)
    {
        tString command;
        command << funcionPair.first;
        tToUpper(command);
        data.chatBotCommandConfItems->StoreConfitem(new tConfItemFunc(command, &TempChatBotCommandRunner));
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

eChatBotDataBase::ActionData  &eChatBotMessager::Params()
{
    return Data().params;
}

eChatBotStats &eChatBotMessager::Stats()
{
    return Data().stats;
}

eChatBotData & eChatBotMessager::Data()
{
    return *data;
}
