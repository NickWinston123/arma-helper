#include "eChatBot.h"
#include "ePlayer.h"
#include "ePlayerStats.h"
#include "eTimer.h"
#include "../tron/gGame.h"
#include "../tron/gMenus.h"
#include "tDatabase.h"
#include "tRandom.h"

#include "eChatCommands.h"

#include <stack>
#include <queue>

bool se_playerMessageTriggers = false;
static tConfItem<bool> se_playerMessageTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS", se_playerMessageTriggers);
tString se_playerMessageEnabledPlayers = tString("");
static tConfItem<tString> se_playerMessageEnabledPlayersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_ENABLED_PLAYERS", se_playerMessageEnabledPlayers);

tString se_playerMessageTriggersFile("chattriggers.txt");
static tConfItem<tString> se_playerMessageTriggersFileConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FILE", se_playerMessageTriggersFile);

REAL se_playerMessageTriggersSpamMaxlen = 300;
static tConfItem<REAL> se_playerMessageTriggersSpamMaxlenConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SPAM_MAXLEN", se_playerMessageTriggersSpamMaxlen);
REAL se_playerMessageTriggersDelayForNegativeParts = 1;
static tConfItem<REAL> se_playerMessageTriggersDelayForNegativePartsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DELAY_FOR_NEGATIVE_DELAY_PARTS", se_playerMessageTriggersDelayForNegativeParts);

tString se_playerMessageTriggersIgnorePhrases = tString("");
static tConfItem<tString> se_playerMessageTriggersIgnorePhrasesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_IGNORE_PHRASES", se_playerMessageTriggersIgnorePhrases);
tString se_playerMessageTriggersIgnorePhrasesStartsWith = tString("");
static tConfItem<tString> se_playerMessageTriggersIgnorePhrasesStartsWithConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_IGNORE_PHRASES_STARTS_WITH", se_playerMessageTriggersIgnorePhrasesStartsWith);
tString se_playerMessageTriggersIgnorePhrasesEndsWith = tString("");
static tConfItem<tString> se_playerMessageTriggersIgnorePhrasesEndsWithConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_IGNORE_PHRASES_ENDS_WITH", se_playerMessageTriggersIgnorePhrasesEndsWith);

static bool se_playerMessageTriggersChatFunctionsOnly = false;
static tConfItem<bool> se_playerMessageTriggersChatFunctionsOnlyConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNCTIONS_ONLY", se_playerMessageTriggersChatFunctionsOnly);

bool se_playerMessageTriggersSpamProtectionCheck = false;
static tConfItem<bool> se_playerMessageTriggersSpamProtectionCheckConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SPAM_PROTECTION_CHECK", se_playerMessageTriggersSpamProtectionCheck);

bool se_playerMessageTriggersResendSilencedMessages = false;
static tConfItem<bool> se_playerMessageTriggersResendSilencedMessagesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SILENCED_RESEND_MESSAGES", se_playerMessageTriggersResendSilencedMessages);
bool se_playerMessageTriggersResendSilencedPrefixDuplicatedMessages = false;
static tConfItem<bool> se_playerMessageTriggersResendSilencedPrefixDuplicatedMessagesConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SILENCED_RESEND_PREFIX_DUPLICATED_MESSAGES", se_playerMessageTriggersResendSilencedPrefixDuplicatedMessages);
bool se_playerMessageTriggersResendSilencedMessagesPrefixAmount = 3;
static tConfItem<bool> se_playerMessageTriggersResendSilencedMessagesPrefixAmountConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SILENCED_RESEND_MESSAGES_PREFIX_AMOUNT", se_playerMessageTriggersResendSilencedMessagesPrefixAmount);
tString se_playerMessageTriggersResendSilencedMessagesPrefixList = tString("!@#$%^&*()_+=-|");
static tConfItem<tString> se_playerMessageTriggersResendSilencedMessagesPrefixListConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SILENCED_RESEND_MESSAGES_PREFIX_LIST", se_playerMessageTriggersResendSilencedMessagesPrefixList);
bool se_playerMessageTriggersClearOnSilence = true;
static tConfItem<bool> se_playerMessageTriggersClearOnSilenceConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SILENCED_CLEAR_ON_SILENCE", se_playerMessageTriggersClearOnSilence);
REAL se_playerMessageTriggersResendSilencedMessagesExtraDelay = 0;
static tConfItem<REAL> se_playerMessageTriggersResendSilencedMessagesExtraDelayConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SILENCED_RESEND_MESSAGES_EXTRA_DELAY", se_playerMessageTriggersResendSilencedMessagesExtraDelay);


bool se_playerMessageTriggersContextBuilder = false;
static tConfItem<bool> se_playerMessageTriggersContextBuilderConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CONTEXT_BUILDER", se_playerMessageTriggersContextBuilder);
bool se_playerMessageTriggersContextBuilderStore = true;
static tConfItem<bool> se_playerMessageTriggersContextBuilderStoreConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CONTEXT_BUILDER_STORE", se_playerMessageTriggersContextBuilderStore);
tString se_playerMessageTriggersContextBuilderStoreFileName("contextbuilder.txt");
static tConfItem<tString> se_playerMessageTriggersContextBuilderStoreFileNameConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CONTEXT_BUILDER_STORE_FILE_NAME", se_playerMessageTriggersContextBuilderStoreFileName);
bool se_playerMessageTriggersContextBuilderStoreTimestamp = true;
static tConfItem<bool> se_playerMessageTriggersContextBuilderStoreTimestampConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CONTEXT_BUILDER_STORE_TIMESTAMP", se_playerMessageTriggersContextBuilderStoreTimestamp);
bool se_playerMessageTriggersContextBuilderAdditionalContext = false;
static tConfItem<bool> se_playerMessageTriggersContextBuilderAdditionalContextConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CONTEXT_BUILDER_ADDITIONAL_CONTEXT_EVERY_MESSAGE", se_playerMessageTriggersContextBuilderAdditionalContext);


bool se_playerMessageTriggerScheduleMultiple = true;
static tConfItem<bool> se_playerMessageTriggerScheduleMultipleConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_SCHEDULE_MULTIPLE", se_playerMessageTriggerScheduleMultiple);


int se_playerMessageTriggersQueueMaxOverloadedSize = 10;
static tConfItem<int> se_playerMessageTriggersQueueMaxOverloadedSizeConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_QUEUE_OVERLOADED_MAX_SIZE", se_playerMessageTriggersQueueMaxOverloadedSize);

bool se_playerMessageTriggersReactToSelf = false;
static tConfItem<bool> se_playerMessageTriggersReactToSelfConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_REACT_TO_SELF", se_playerMessageTriggersReactToSelf);

bool se_playerMessageTriggersReactToLocal = false;
static tConfItem<bool> se_playerMessageTriggersReactToLocalConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_REACT_TO_LOCAL", se_playerMessageTriggersReactToLocal);

static tString se_playerMessageTriggersKillVerifiedTriggers = tString("wd,nice,wp,gj,$diedother,annoying,n1");
static tConfItem<tString> se_playerMessageTriggersKillVerifiedTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_KILLED_VERIFIED_TRIGGERS", se_playerMessageTriggersKillVerifiedTriggers);

static tString se_playerMessageTriggersDiedByVerifiedTriggers("$died");
static tConfItem<tString> se_playerMessageTriggersDiedByVerifiedTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_DIED_BY_VERIFIED_TRIGGERS", se_playerMessageTriggersDiedByVerifiedTriggers);

static tString se_playerMessageTriggersJoinedRecentlyVerifiedTriggers("wb,welcome");
static tConfItem<tString> se_playerMessageTriggersJoinedRecentlyVerifiedTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_JOINED_RECENTLY_VERIFIED_TRIGGERS", se_playerMessageTriggersJoinedRecentlyVerifiedTriggers);
static REAL se_playerMessageTriggersJoinedRecentlyWaitTime = 40;
static tConfItem<REAL> se_playerMessageTriggersJoinedRecentlyWaitTimeConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_JOINED_RECENTLY_WAIT_TIME", se_playerMessageTriggersJoinedRecentlyWaitTime);

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


static tString se_playerMessageReminderFuncCommand = tString("/speakbot 3");
static tConfItemLine se_playerMessageReminderFuncCommandConf = HelperCommand::tConfItemLine("PLAYER_MESSAGE_TRIGGERS_FUNC_REMINDER_FUNC_COMMAND", se_playerMessageReminderFuncCommand);

static REAL se_playerMessageNotificationsDelay = 5;
static tConfItem<REAL> se_playerMessageNotificationsDelayConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_NOTIFICATIONS_DELAY", se_playerMessageNotificationsDelay);
tString se_playerMessageNotificationTrigger("$notificationslistfunc");
static tConfItem<tString> se_playerMessageNotificationTriggerConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_NOTIFICATIONS_TRIGGER", se_playerMessageNotificationTrigger);
static bool se_playerMessageNotificationsRestrictView = true;
static tConfItem<bool> se_playerMessageNotificationsRestrictViewConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_NOTIFICATIONS_RESTRICT_VIEW", se_playerMessageNotificationsRestrictView);
static bool se_playerMessageNotificationsListShowTime = false;
static tConfItem<bool> se_playerMessageNotificationsListShowTimeConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_NOTIFICATIONS_SHOW_TIME", se_playerMessageNotificationsListShowTime);

tString se_playerMessageNotificationsDelimiter = tString("+:N0T3D3liM1:-");
static tConfItem<tString> se_playerMessageNotificationsDelimiterConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_NOTIFICATIONS_DELIMTER", se_playerMessageNotificationsDelimiter);

static tString se_playerMessageTriggersChatChanceTriggers = tString("");
static tConfItem<tString> se_playerMessageTriggersChatChanceTriggersConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CHANCE_COMMANDS", se_playerMessageTriggersChatChanceTriggers);
static REAL se_playerMessageTriggersChatChanceTriggersThreshhold = 0.5;
static tConfItem<REAL> se_playerMessageTriggersChatChanceTriggersThreshholdConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_CHANCE_COMMANDS_THRESHOLD", se_playerMessageTriggersChatChanceTriggersThreshhold);

static tString se_playerMessageWhatsTheFuncVersionStr("");
static tConfItemLine se_playerMessageWhatsTheFuncVersionStrConf = HelperCommand::tConfItemLine("PLAYER_MESSAGE_TRIGGERS_FUNC_WHATS_THE_FUNC_VERSION_STRING", se_playerMessageWhatsTheFuncVersionStr);

static bool se_playerMessageWhatsTheFuncVersionShowDBAge = true;
static tConfItem<bool> se_playerMessageWhatsTheFuncVersionShowDBAgeConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_WHATS_THE_FUNC_VERSION_SHOW_DB_AGE", se_playerMessageWhatsTheFuncVersionShowDBAge);


bool se_playerMessageTriggersRageQuits = false;
static tConfItem<bool> se_playerMessageTriggersRageQuitsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_RAGE_QUITS", se_playerMessageTriggersRageQuits);

bool se_playerMessageTriggersAcheivements = false;
static tConfItem<bool> se_playerMessageTriggersAcheivementsConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS", se_playerMessageTriggersAcheivements);

bool se_playerMessageTriggersAcheivementsLocal = false;
static tConfItem<bool> se_playerMessageTriggersAcheivementsLocalConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_LOCAL", se_playerMessageTriggersAcheivementsLocal);

int se_playerMessageTriggersAcheivementsKillsChangeVal = 50;
static tConfItem<int> se_playerMessageTriggersAcheivementsKillsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLS_CHANGE_VAL", se_playerMessageTriggersAcheivementsKillsChangeVal);

int se_playerMessageTriggersAcheivementsKillStreakChangeVal = 5;
static tConfItem<int> se_playerMessageTriggersAcheivementsKillStreakChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLSTREAK_CURRENT_CHANGE_VAL", se_playerMessageTriggersAcheivementsKillStreakChangeVal);

int se_playerMessageTriggersAcheivementsMaxKillStreak = 1;
static tConfItem<int> se_playerMessageTriggersAcheivementsMaxKillStreakConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_KILLSTREAK_MAX_CHANGE_VAL", se_playerMessageTriggersAcheivementsMaxKillStreak);

int se_playerMessageTriggersAcheivementsChatsChangeVal = 25;
static tConfItem<int> se_playerMessageTriggersAcheivementsChatsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_CHATS_CHANGE_VAL", se_playerMessageTriggersAcheivementsChatsChangeVal);

int se_playerMessageTriggersAcheivementsJoinsChangeVal = 10;
static tConfItem<int> se_playerMessageTriggersAcheivementsJoinsChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_JOINS_CHANGE_VAL", se_playerMessageTriggersAcheivementsJoinsChangeVal);

int se_playerMessageTriggersAcheivementsBansChangeVal = 5;
static tConfItem<int> se_playerMessageTriggersAcheivementsBansChangeValConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGER_ACHEIVEMENTS_BANS_CHANGE_VAL", se_playerMessageTriggersAcheivementsBansChangeVal);

tString se_playerMessageTriggersFuncLogFile("chatbotLogFunc.txt");
static tConfItem<tString> se_playerMessageTriggersFuncLogFileConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_LOG_FILE", se_playerMessageTriggersFuncLogFile);

int se_playerMessageTriggersGuessGameGiveupAnnounceInterval = 10;
static tConfItem<int> se_playerMessageTriggersGuessGameGiveupAnnounceIntervalConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_FUNC_GUESS_GAME_GIVEUP_ANNOUNCE_INTERVAL", se_playerMessageTriggersGuessGameGiveupAnnounceInterval);

tString se_playerMessageEnabledExpedientKeys = tString("$$$,!!!");
static tConfItem<tString> se_playerMessageEnabledExpedientKeysConf = HelperCommand::tConfItem("PLAYER_MESSAGE_TRIGGERS_EXPEDIENT_KEYS", se_playerMessageEnabledExpedientKeys);


std::vector<ePlayerNetID *> se_GetPlayerMessageEnabledPlayers()
{
    std::vector<ePlayerNetID *> enabledPlayers;
    tArray<tString> players = se_playerMessageEnabledPlayers.SplitForPlayers();

    for (int i = 0; i < players.Len(); i++)
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[i]) - 1);

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

    if (input.empty())
        return tString("Invalid input");

    bool isNegative = false;
    size_t idx = 0;
    if (input[0] == '-')
    {
        isNegative = true;
        idx = 1;
    }

    std::string intPart, fracPart;
    while (idx < input.size() && input[idx] != '.')
    {
        if (!std::isdigit(input[idx]))
            return tString("Invalid input");
        intPart += input[idx++];
    }
    if (idx < input.size() && input[idx] == '.')
    {
        idx++;
        while (idx < input.size())
        {
            if (!std::isdigit(input[idx]))
                return tString("Invalid input");
            fracPart += input[idx++];
        }
    }

    while (intPart.size() > 1 && intPart[0] == '0')
        intPart.erase(0, 1);

    if (intPart.empty())
        intPart = "0";


    while (fracPart.size() > 1 && fracPart.back() == '0')
        fracPart.pop_back();

    bool originalIsNegativeZero = isNegative && intPart == "0" && (fracPart.empty() || fracPart == "0");

    int adderValue = se_playerMessageTriggersFuncAdderVal;

    int carry;
    if (originalIsNegativeZero)
    {
        carry = adderValue;
        isNegative = false;
    }
    else
        carry = isNegative ? -adderValue : adderValue;

    for (int i = fracPart.size() - 1; i >= 0; i--)
    {
        int digit = fracPart[i] - '0';
        int sum = digit + carry;

        carry = sum / 10;
        sum = sum % 10;
        if (sum < 0)
        {
            sum += 10;
            carry -= 1;
        }

        fracPart[i] = '0' + sum;
    }

    for (int i = intPart.size() - 1; i >= 0; i--)
    {
        int digit = intPart[i] - '0';
        int sum = digit + carry;

        carry = sum / 10;
        sum = sum % 10;
        if (sum < 0)
        {
            sum += 10;
            carry -= 1;
        }

        intPart[i] = '0' + sum;
    }

    while (carry != 0)
    {
        int sum = carry % 10;
        if (sum < 0)
        {
            sum += 10;
            carry = (carry / 10) - 1;
        }
        else
        {
            carry = carry / 10;
        }
        intPart = char('0' + sum) + intPart;
    }

    while (intPart.size() > 1 && intPart[0] == '0')
        intPart.erase(0, 1);

    while (fracPart.size() > 1 && fracPart.back() == '0')
        fracPart.pop_back();

    if (intPart == "0" && fracPart.empty())
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
    return CalculateCommand::process(args, sg_helperDebug);
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
    static std::vector<std::pair<tString, tString>> statLabels =
        {
            {tString("RGB"), tString("RGB")},
            {tString("Play Time"), tString("Play Time")},
            {tString("Spectate Time"), tString("Spec Time")},
            {tString("Chat Count"), tString("Chats")},
            {tString("Notifications"), tString("Notifs")},
        };

    if (!message.empty())
    {
        tString playerName(message.SubStr(0, 16).TrimWhitespace());
        ePlayerNetID *statsTargetPlayer = ePlayerNetID::GetPlayerByName(playerName, false);
        if (statsTargetPlayer)
            playerName = statsTargetPlayer->GetName();

        PlayerData *stats;
        if (statsTargetPlayer)
            stats = &ePlayerStats::getStats(statsTargetPlayer);
        else
            stats = &ePlayerStats::getStatsForAnalysis(playerName);

        tString displayName(!statsTargetPlayer || ePlayerStats::shouldEnforceLocalName(statsTargetPlayer) ? (!stats->name.empty() ? stats->name : playerName) : playerName);

        output << displayName << ": ";

        std::vector<tString> segments;

        for (const auto &pair : statLabels)
        {
            const tString &key = pair.first;
            const tString &label = pair.second;

            tString entry;

            if (stats->isPrivate(key))
            {
                entry << label << ": ?";
                segments.push_back(entry);
                continue;
            }

            if (key == "Play Time")
            {
                tString statValue = stats->getPlayTimeStr();
                if (stats->in_server)
                    statValue << " (" << stats->thisSession().getPlayTimeStr() << ")";
                entry << label << ": " << statValue;
            }
            else if (key == "Spectate Time")
            {
                tString statValue = stats->getSpecTimeStr();
                if (stats->in_server)
                    statValue << " (" << stats->thisSession().getSpecTimeStr() << ")";
                entry << label << ": " << statValue;
            }
            else if (key == "RGB")
            {
                entry << label << ": " << stats->rgbString();
            }
            else if (key == "Chat Count")
            {
                entry << label << ": " << stats->getMessageCount();
                if (stats->in_server)
                    entry << " (" << stats->thisSession().getMessageCount() << ")";
            }
            else if (key == "Notifications")
            {
                int unseenCount = 0;
                int validCount = 0;

                for (const std::string &note : stats->notifications)
                {
                    tArray<tString> parts = tString(note).Split(se_playerMessageNotificationsDelimiter);

                    if (parts.Len() >= 3)
                    {
                        validCount++;
                        try
                        {
                            time_t ts = std::stoll(parts[2].stdString());
                            if (ts > stats->last_seen_notification_time)
                                unseenCount++;
                        }
                        catch (...) {}
                    }
                }

                entry << label << ": " << validCount;
                entry << " (" << unseenCount << " new)";
            }

            segments.push_back(entry);
        }

        for (size_t i = 0; i < segments.size(); ++i)
        {
            output << segments[i];
            if (i < segments.size() - 1)
                output << ", ";
        }

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
                       << " "
                       << getTimeZone(getCurrentLocalTime())
                       << " ("
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

        return output;
    }

    // no player passed - chatbot stats
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

    return output;
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

    bool now    = false;
    bool today  = false;
    bool total  = false;

    if (mode == "now")
        now = true;
    else if (mode == "today")
        today = true;
    else if (mode == "total")
        total = true;
    else
    {
        output << "No mode set! Use: 'now', 'today', or 'total'";
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

        PlayerDataBase &sourceStats = today ? stats->todayStats() :
                                        now   ? stats->thisSession() :
                                                *static_cast<PlayerDataBase*>(stats);
        tString displayName(
            (!statsTargetPlayer || ePlayerStats::shouldEnforceLocalName(statsTargetPlayer))
            ? (!stats->name.empty() ? stats->name : playerName)
            : playerName);

        output << displayName;
        if (now)    output << " (NOW)";
        if (today)  output << " (TODAY)";
        output << ": ";

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
            else if (key == "rounds_played" || key == "matches_played")
                label = "Played";
            else if (key == "round_wins" || key == "match_wins")
                label = "Wins";
            else if (key == "round_losses" || key == "match_losses")
                label = "Losses";

            if (i != 0)
                output << separators[i - 1];

            if (stats->isPrivate(realLabel))
            {
                output << label << ": ?";
            }
            else
            {
                output << label << ": " << sourceStats.getAnyValue(key);
            }
        }
    }
    else
    {
        output << "Player not found!";
    }

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

    tString mode = message.ExtractNonBlankSubString(pos);
    bool now = false;
    bool today = false;
    bool current = false;

    if (mode == "here")
        current = true;
    else if (mode == "now")
    {
        current = true;
        now = true;
    }
    else if (mode == "today")
        today = true;
    else if (mode == "all")
    {

    }
    else
    {
        result << "No mode set! Use: 'all', 'here', 'now', or 'today'\n";
        return result;
    }

    tString nextArg = message.ExtractNonBlankSubString(pos);
    if (!nextArg.empty())
    {
        if (isdigit(nextArg[0]))
        {
            page = atoi(nextArg);
        }
        else
        {
            desiredStatChoosen = true;
            statName = nextArg;

            tString possiblePage = message.ExtractNonBlankSubString(pos);
            if (!possiblePage.empty() && isdigit(possiblePage[0]))
                page = atoi(possiblePage);
        }
    }

    if (page <= 0)
        page = 1;

    statLabel = PlayerData::getAnyLabel(statName);

    bool showLabel             = !desiredStatChoosen || se_playerMessageTriggersLeaderboardLabels;
    bool lastSeenStat          = (statLabel == "Last Seen");
    bool playTimeStat          = (statLabel == "Play Time");
    bool specTimeStat          = (statLabel == "Spectate Time");
    bool currentKillStreakStat = (statLabel == "Current Kill Streak");
    bool nicknameStat          = (statLabel == "Nickname");
    bool slowestSpeedStat      = (statLabel == "Slowest Speed");
    bool pingStat              = (statLabel == "Ping");
    bool localStat             = (statLabel == "Local");
    bool aliveStat             = (statLabel == "Alive");

    bool penalizePlayersNotInGame = (lastSeenStat || nicknameStat);
    bool ignorePlayersNotInGame = currentKillStreakStat;

    time_t startOfDay = getStartTimeOfDay();

    std::vector<std::pair<tString, PlayerData>> sortedPlayers;
    for (auto &entry : ePlayerStats::GetPlayerStatsMap())
    {
        PlayerData &data = entry.second;

        if (!data.human || data.deleted)
            continue;
        if (current && !data.in_server)
            continue;
        if (ignorePlayersNotInGame && !data.in_server)
            continue;
        if (nicknameStat && data.nickname.empty())
            continue;
        if (today && data.last_seen < startOfDay)
            continue;

        PlayerDataBase &stats = today ? data.todayStats() :
                                 now   ? data.thisSession() :
                                         *static_cast<PlayerDataBase*>(&data);

        if (slowestSpeedStat && (stats.slowest_speed == 99999.0 || stats.slowest_speed == 0.0))
            continue;

        if (pingStat && stats.ping == 0)
            continue;

        sortedPlayers.push_back(entry);
    }

    if (showLabel && !se_playerMessageTriggersLeaderboardLabels)
    {
        bool fastestStat = !lastSeenStat && (statLabel == "Fastest Speed");
        if (lastSeenStat || fastestStat)
            showLabel = false;
    }

    auto sortUsingGetAnyValue = [&statName, &statLabel, &penalizePlayersNotInGame, &playTimeStat, &specTimeStat, &slowestSpeedStat, &today, &now, &localStat, &aliveStat](auto &a, auto &b)
    {
        bool aPrivate = a.second.isPrivate(statLabel);
        bool bPrivate = b.second.isPrivate(statLabel);
        if (aPrivate && !bPrivate) return false;
        if (!aPrivate && bPrivate) return true;

        PlayerDataBase &aStats = today ? a.second.todayStats() :
                                   now   ? a.second.thisSession() :
                                           *static_cast<PlayerDataBase*>(&a.second);
        PlayerDataBase &bStats = today ? b.second.todayStats() :
                                   now   ? b.second.thisSession() :
                                           *static_cast<PlayerDataBase*>(&b.second);

        if (penalizePlayersNotInGame)
        {
            if (a.second.in_server && b.second.in_server)
                return a.second.last_seen < b.second.last_seen;
            if (a.second.in_server) return true;
            if (b.second.in_server) return false;
            return a.second.last_seen > b.second.last_seen;
        }
        else if (playTimeStat)
            return aStats.total_play_time > bStats.total_play_time;
        else if (specTimeStat)
            return aStats.total_spec_time > bStats.total_spec_time;
        else if (slowestSpeedStat)
            return aStats.slowest_speed < bStats.slowest_speed;
        else if (localStat)
        return aStats.is_local > bStats.is_local;
        else if (aliveStat)
            return aStats.alive > bStats.alive;
        else
        {
            REAL aValue = atof(aStats.getAnyValue(statName).c_str());
            REAL bValue = atof(bStats.getAnyValue(statName).c_str());
            return aValue > bValue;
        }
    };

    auto stat = PlayerData::valueMap.find(statName.stdString());
    if (stat != PlayerData::valueMap.end())
        std::sort(sortedPlayers.begin(), sortedPlayers.end(), sortUsingGetAnyValue);
    else
        return PlayerData::getAvailableStatsStr("leaderboardFunc");

    int startIdx = (page - 1) * playersPerPage;
    int endIdx = std::min(startIdx + playersPerPage, (int)sortedPlayers.size());

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
            labelToDisplay = "Nicknames";
        result << labelToDisplay;
        if (now || current || today) result << " ";
    }

    if (now) result << "(NOW)";
    else if (current) result << "(HERE)";
    else if (today) result << "(TODAY)";
    result << ": ";

    for (int i = startIdx; i < endIdx; ++i)
    {
        PlayerData &data = sortedPlayers[i].second;
        PlayerDataBase &stats = today ? data.todayStats() :
                                     now   ? data.thisSession() :
                                             *static_cast<PlayerDataBase*>(&data);

        bool showValue = !data.isPrivate(statLabel);
        if (i > startIdx)
            result << ", ";

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
            value << "?";

        result << " (" << value << ")";
    }

    result << " (" << page << "/" << totalPages << ")";
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

    tString mode(message.ExtractNonBlankSubString(pos));
    bool now = false, today = false;
    if (mode == "now")
        now = true;
    else if (mode == "today")
        today = true;
    else if (mode != "total")
    {
        output << "No mode set! Use: 'now', 'today', or 'total'\n";
        return output;
    }

    stat = message.ExtractNonBlankSubString(pos);
    tString statLabel = PlayerData::getAnyLabel(stat);

    if (statLabel.empty() && stat == "last_active")
        statLabel = "Last Active";

    if (statLabel.empty())
    {
        output << "Stat not found! Usage: '"
               << bot.Messager()->Params().matchedTrigger
               << " stat player' "
               << PlayerData::getAvailableStatsStr();
        return output;
    }

    playerName = message.SubStr(pos + 1).TrimWhitespace();
    if (playerName.empty())
        statsTargetPlayer = bot.Messager()->Params().triggeredBy;

    if (!statsTargetPlayer)
        statsTargetPlayer = ePlayerNetID::GetPlayerByName(playerName, false);

    if (statsTargetPlayer)
        playerName = statsTargetPlayer->GetName();

    PlayerData *statsPtr = statsTargetPlayer ? &ePlayerStats::getStats(statsTargetPlayer)
                                              : &ePlayerStats::getStatsForAnalysis(playerName);
    PlayerData &stats = *statsPtr;

    PlayerDataBase &sourceStats = today ? stats.todayStats() :
                                now   ? stats.thisSession() :
                                        *static_cast<PlayerDataBase*>(&stats);

    bool showValue = statLabel == "Hidden Stats" || !stats.isPrivate(statLabel);

    tString statValue, statAfterValue, currentSessionStatValue;
    if (showValue)
    {
        bool showCurrentSessionVal = true;

        if (statLabel == "Kill Death Ratio")
        {
            statValue << sourceStats.getKDRatio(false);
        }
        else if (statLabel == "Fastest Speed")
        {
            statValue << sourceStats.getSpeed(false);
        }
        else if (statLabel == "Last Seen")
        {
            statValue << stats.getLastSeenAgoStr(false);

            if (stats.last_seen > 0)
            {
                struct tm *timeInfo = localtime(&stats.last_seen);
                char buffer[64];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);  // Format: 2024-06-14 13:45:00
                statValue << " (" << buffer << " " << getTimeZone(*timeInfo, true) << ")";
            }

            if (stats.in_server && statsTargetPlayer)
                statValue << " | Last Active: " << st_GetFormatTime(statsTargetPlayer->LastActivity(), false);

            showCurrentSessionVal = false;
        }
        else if (statLabel == "Times Banned")
        {
            statValue << sourceStats.times_banned;
            statAfterValue << " - Bans Given: " << sourceStats.bans_given;
        }
        else if (statLabel == "Bans Given")
        {
            statValue << sourceStats.bans_given;
            statAfterValue << " - Times Banned: " << sourceStats.times_banned;
        }
        else if (statLabel == "RGB")
        {
            statValue << stats.rgbString(!bot.Messager()->Params().pentalized_for_last_message);
            showCurrentSessionVal = false;
        }
        else if (statLabel == "Last Active")
        {
            statValue << (statsTargetPlayer ? st_GetFormatTime(statsTargetPlayer->LastActivity(), false) : "Not here");
            showCurrentSessionVal = false;
        }
        else
        {
            statValue << sourceStats.getAnyValue(stat.TrimWhitespace());
        }

        if (statLabel == "Slowest Speed" && sourceStats.slowest_speed == 99999.0)
            statValue = "N/A";
    }
    else
    {
        statValue << "?";
    }

    statValue << statAfterValue;

    tString displayName(!statsTargetPlayer || ePlayerStats::shouldEnforceLocalName(statsTargetPlayer)
                            ? (!stats.name.empty() ? stats.name : playerName)
                            : playerName);

    output << displayName;

    if (today)
        output << " (TODAY)";
    else if (now)
        output << " (NOW)";

    output << ": " << statLabel << " - ";


    if (!statValue.empty() || statLabel == "Chats" || statLabel == "Hidden Stats")
    {
        if (statLabel == "Chats")
            output << output.SubStr(0, se_SpamMaxLen - 1);
        else
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
            feedback << tColoredString::RemoveColors(tConfItemBase::lastLoadOutput);


        if (bot.Messager()->Params().sendingPlayer)
            se_NewChatMessage(bot.Messager()->Params().sendingPlayer, feedback)->BroadCast();
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

tString extractFuncName(const tString &response)
{
    int openParen = response.StrPos("(");
    if (response.StartsWith("$") && openParen != -1 && response.EndsWith(")"))
        return response.SubStr(0, openParen);
    return response;
}

tString whatsTheFunc(tString message)
{
    tString output;
    tString target = message.TrimWhitespace();

    if (target.Contains("player_score"))
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
                   << " "
                   << getTimeZone(created);
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
                output << "You joined before I did.";
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
        if (!se_playerStats)
        {
            output << "PlayerStats not initialized. PlayerStats are disabled!\n";
            return output;
        }

        time_t startOfDay = getStartTimeOfDay();

        int number_of_players = 0;
        for (auto stats : ePlayerStats::GetPlayerStatsMap())
        {
            if (stats.second.last_seen >= startOfDay)
                number_of_players++;
        }

        output << "Number of players since 12:00AM "
               << getTimeZone(getCurrentLocalTime())
               << ": "
               << number_of_players
               << "\n";
    }
    else if (target.Contains("players_this_hour"))
    {
        if (!se_playerStats)
        {
            output << "PlayerStats not initialized. PlayerStats are disabled!\n";
            return output;
        }

        time_t startOfHour = getStartOfCurrentHour();

        int number_of_players = 0;
        for (auto statPair : ePlayerStats::GetPlayerStatsMap())
        {
            if (statPair.second.last_seen >= startOfHour || statPair.second.in_server)
                number_of_players++;
        }

        output << "Number of players since start of this hour: "
               << number_of_players
               << "\n";
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
        struct tm* lastTimeBanned;

        time_t now = time(NULL);
        double lastBanned = difftime(now, stats.last_banned);

        lastTimeBanned = localtime(&stats.last_banned);
        output << getTimeStringBase(*lastTimeBanned)
               << " "
               << getTimeZone(*lastTimeBanned)
               << " ("
               << st_GetFormatTime(lastBanned, false, false)
               << " ago)";
        if (!stats.last_banned_reason.empty())
            output << ". Reason: "
                   << stats.last_banned_reason;
    }
    else if (target.Contains("current_time"))
    {
        struct tm now = getCurrentLocalTime();
        output << getTimeStringBase(now)
               << " "
               << getTimeZone(now);
    }
    else if (target.Contains("current_date"))
    {
        struct tm now = getCurrentLocalTime();
        output << st_GetCurrentTime("%Y-%m-%d", now);
    }
    else if (target.Contains("my_color"))
    {
        eChatBot &bot = eChatBot::getInstance();
        ePlayerNetID *targetPlayer = bot.Messager()->Params().triggeredBy;

        if (targetPlayer)
        {
            output << targetPlayer->GetName()
                   << ": ";

            if (se_playerStats)
            {
                PlayerData *stats = &ePlayerStats::getStats(targetPlayer);
                if (stats)
                    output << stats->rgbString(!bot.Messager()->Params().pentalized_for_last_message);
            }
            else
            {
                output << "("
                       << targetPlayer->r << ", "
                       << targetPlayer->g << ", "
                       << targetPlayer->b
                       << ")";
                if (!bot.Messager()->Params().pentalized_for_last_message)
                    output << " "
                           << ColorsCommand::cycleColorPreview(targetPlayer->r,targetPlayer->g,targetPlayer->b, false);
            }

        }
    }
    else if (target.Contains("fps"))
    {
        int newfps = static_cast<int>(se_AverageFPS());
        output << "Current FPS: "
               << newfps;
    }
    else if (target.Contains("commands"))
    {
        eChatBot &bot = eChatBot::getInstance();
        std::map<tString, tString> funcToTrigger;

        for (const auto &chatTrigger : bot.data.chatTriggers)
        {
            if (chatTrigger.trigger == "$number" ||
                chatTrigger.trigger == "$math" ||
                chatTrigger.trigger == "$repeatedchars")
                continue;

            for (const auto &response : chatTrigger.responses)
            {
                tString funcName = extractFuncName(response);
                if (bot.data.functions->functionMap.count(funcName))
                {
                    funcToTrigger.insert({funcName, chatTrigger.trigger});
                    break;
                }
            }
        }

        if (funcToTrigger.empty())
        {
            output << "No triggers found for registered commands.\n";
        }
        else
        {
            output << "Triggers: ";
            bool first = true;
            for (const auto &[func, trigger] : funcToTrigger)
            {
                if (!first)
                    output << ", ";
                output << trigger;
                first = false;
            }
        }
    }
    else if (target.Contains("smarter_bot_contribution"))
    {
        if (!sg_navigatorDebugTrackEvaluation)
        {
            output << "NAVIGATOR_DEBUG_TRACK_EVALUATION is not enabled!\n";
            return output;
        }

        tArray<tString> players = se_playerMessageEnabledPlayers.SplitForPlayers();
        for (int i = 0; i < players.Len(); i++)
        {
            ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[i]) - 1);
            if (!local_p)
                continue;

            ePlayerNetID *netPlayer = local_p->netPlayer;
            if (!netPlayer || !netPlayer->CurrentTeam())
                continue;

            output << local_p->sg_smarterBotContributionStr;
            break;
        }
    }
    else if (target.Contains("ip_address"))
    {
        eChatBot &bot = eChatBot::getInstance();
        ePlayerNetID *triggeredByPlayer = bot.Messager()->Params().triggeredBy;

        if (!triggeredByPlayer || !triggeredByPlayer->encryptVerified )
        {
            output << "You are not verified";
            return output;
        }

        if (sn_ourRealIPS.empty())
            output << "OUR_REAL_IPS is not set!\n";
        else
        {
            tString ipAddr = sn_GetMyAddress();
            tString pureIP = ipAddr;
            int pos = ipAddr.StrPos(":");

            if (pos != -1)
                pureIP = ipAddr.SubStr(0, pos);

            if (tIsInList(sn_ourRealIPS, pureIP))
                output << "IP address is listed in 'OUR_REAL_IPS'";
            else
                output << ipAddr;
        }
    }
    else if (target.Contains("player_count"))
    {
        int count = 0;
        tArray<tString> names;
        for (int i = 0; i < se_PlayerNetIDs.Len(); i++)
        {
            if (se_PlayerNetIDs[i] && (!se_PlayerNetIDs[i]->isLocal() || !tIsEnabledForPlayer(se_disableCreateSpecific, se_PlayerNetIDs[i]->pID + 1)))
            {
                count++;
                names.Add(se_PlayerNetIDs[i]->GetName());
            }
        }

        tString players;
        for (int i = 0; i < names.Len(); i++) {
            players << names[i];
            if (i < names.Len() - 1)
                players << ", ";
        }

        output << count << " (" << players << ")";
    }
    else if (target.Contains("spectator_count"))
    {
        int count = 0;
        tArray<tString> names;
        for (int i = 0; i < se_PlayerNetIDs.Len(); i++)
        {
            if (se_PlayerNetIDs[i] && !se_PlayerNetIDs[i]->CurrentTeam() && (!se_PlayerNetIDs[i]->isLocal() || !tIsEnabledForPlayer(se_disableCreateSpecific, se_PlayerNetIDs[i]->pID + 1)))
            {
                count++;
                names.Add(se_PlayerNetIDs[i]->GetName());
            }
        }

        tString spectators;
        for (int i = 0; i < names.Len(); i++) {
            spectators << names[i];
            if (i < names.Len() - 1)
                spectators << ", ";
        }

        output << count << " (" << spectators << ")";
    }
    else if (target.Contains("automator_score"))
    {
        tArray<tString> players = se_playerMessageEnabledPlayers.SplitForPlayers();

        for (int i = 0; i < players.Len(); i++)
        {
            ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[i]) - 1);
            if (!local_p)
                continue;

            ePlayerNetID *netPlayer = local_p->netPlayer;
            if (!netPlayer || !netPlayer->CurrentTeam())
                continue;

            if (!tIsEnabledForPlayer(sg_smarterBotEnabledPlayers, netPlayer->pID + 1))
                continue;

            output << "Eval score for "
                   << netPlayer->GetName()
                   << ": "
                   << gSmarterBotWeightAutomator::CalculateScoreForPlayer(netPlayer);

            int kills    = netPlayer->stats.kills;
            int deaths   = netPlayer->stats.deaths;
            int suicides = netPlayer->stats.suicides;

            output << " ("
                   << kills
                   << " kills, "
                   << deaths
                   << " deaths, "
                   << suicides
                   << " suicides)";
            break;
        }
    }
    else if (target.Contains("automator_weights"))
    {
        tArray<tString> players = se_playerMessageEnabledPlayers.SplitForPlayers();

        for (int i = 0; i < players.Len(); i++)
        {
            ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[i]) - 1);
            if (!local_p)
                continue;

            ePlayerNetID *netPlayer = local_p->netPlayer;
            if (!netPlayer || !netPlayer->CurrentTeam())
                continue;

            if (!tIsEnabledForPlayer(sg_smarterBotEnabledPlayers, netPlayer->pID + 1))
                continue;

            output << "Current weights for " << netPlayer->GetName() << ": " << gSmarterBotWeightAutomator::GetCurrentWeights(local_p, tString(": "), tString(", "));
            break;
        }
    }
    else if (target.Contains("chatbot_version"))
    {
        if (!se_playerStats)
        {
            output << "PlayerStats not initialized. PlayerStats are disabled!\n";
            return output;
        }

        if (!ePlayerStats::statsWereLoaded())
        {
            output << "PlayerStats not loaded. PlayerStats are disabled!\n";
            return output;
        }

        REAL ageSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - ePlayerStats::getStatsDBCreationDate()
        ).count();

        output << "Version: " << sn_programVersion;

        if (se_playerMessageWhatsTheFuncVersionShowDBAge)
            output << ", DB Age: " << st_GetFormatTime(ageSeconds, false, false);

        if (!se_playerMessageWhatsTheFuncVersionStr.empty())
            output << ", " << se_playerMessageWhatsTheFuncVersionStr;
    }
    else
    { // quick commands
        eChatBot &bot = eChatBot::getInstance();
        ePlayerNetID *targetPlayer = nullptr;

        int pos = 0;
        tString command = message.ExtractNonBlankSubString(pos);
        tString nameArg = message.SubStr(pos).TrimWhitespace();

        if (!nameArg.empty())
            targetPlayer = ePlayerNetID::GetPlayerByName(nameArg.SubStr(0, 16), false);
        else if (nameArg.empty() && bot.Messager()->Params().triggeredBy)
            targetPlayer = bot.Messager()->Params().triggeredBy;

        if (!targetPlayer)
        {
            output << "Player not found.";
            return output;
        }

        gCycle *pCycle = dynamic_cast<gCycle *>(targetPlayer->Object());

        output << targetPlayer->GetName() << ": ";

        if (target.StartsWith("player_ping"))
        {
            output << "Ping: "
                   << int(targetPlayer->ping * 1000) << "ms"
                   << " ("
                   << targetPlayer->ping
                   << ")";

            if (pCycle)
            {
                output << ", Lag: "
                       << int(pCycle->Lag()  * 1000) << "ms"
                       << " ("
                       << pCycle->Lag()
                       << ")";
            }

            output << ", Ping Changes: " << targetPlayer->lastPingIteration;
        }
        else if (target.StartsWith("player_info"))
        {
            if (!targetPlayer->IsHuman() )
                output << "Bot, ";

            output << (targetPlayer->CurrentTeam() ? "Playing" : "Spectating")
                   << ", Ping: " << int(targetPlayer->ping * 1000) << "ms";

            if (targetPlayer->lastTurnTime > 0)
                output << ", Last Turn: " << st_GetFormatTime(tSysTimeFloat() - targetPlayer->lastTurnTime, false, false);

            output << ", Aliases: " << targetPlayer->nameHistory.Len();
            if (targetPlayer->nameHistory.Len() > 0)
            {
                output << " (";
                for (int i = 0; i < targetPlayer->nameHistory.Len(); ++i)
                {
                    output << tColoredString::RemoveColors(targetPlayer->nameHistory[i]);
                    if (i + 1 < targetPlayer->nameHistory.Len())
                        output << ", ";
                }
                output << ")";
            }

            if (targetPlayer->IsChatting() && targetPlayer->ChattingTime() > 0)
                output << ", Chatting: " << st_GetFormatTime(targetPlayer->ChattingTime(), false, false);

            if (pCycle)
            {
                output << ", Lag: "    << (int(pCycle->Lag() * 1000)) << "ms"
                       << ", Pos: "    << roundeCoord(pCycle->Position())
                       << ", Dir: "    << roundeCoord(pCycle->Direction())
                       << ", Speed: "  << pCycle->verletSpeed_
                       << ", Rubber: " << pCycle->GetRubber() << "/" << sg_rubberCycle;

                if (!pCycle->Alive() && pCycle->lastDeathTime > 0)
                    output << ", Last Death: " << st_GetFormatTime(tSysTimeFloat() - pCycle->lastDeathTime, false, false);
                else
                    output << ", Alive Time: " << st_GetFormatTime(se_GameTime(), false, false);
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
    bool searchAllStats = (statName == "all");
    tString statLabel = PlayerData::getAnyLabel(statName);
    bool chatSearch = (statLabel == "Chats");

    if (!searchAllStats && statLabel.empty())
    {
        result << "Stat not found! Usage: '"
               << bot.Messager()->Params().matchedTrigger
               << " stat search value' "
               << PlayerData::getAvailableStatsStr();
        return result;
    }

    tString searchPhrase = message.SubStr(pos).TrimWhitespace();
    std::string lowerSearchPhrase = toLower(searchPhrase.stdString());

    for (auto& statsPair : ePlayerStats::GetPlayerStatsMap())
    {
        auto& playerData = statsPair.second;

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
                    globalFoundCount++;

                    if (globalFoundCount >= max_results)
                        return result;
                }
            }
        }
        else if (searchAllStats)
        {
            for (const auto& stat : PlayerDataBase::valueMap)
            {
                tString value = playerData.getAnyValue(tString(stat.first));

                if (value.Contains(searchPhrase))
                {
                    if (globalFoundCount == 0)
                        result << "Matches - ";
                    else
                        result << ", ";

                    result << statsPair.first << ": " << stat.second.first << "=" << value;
                    globalFoundCount++;
                    break; 

                    if (globalFoundCount >= max_results)
                    {
                        result << "\n";
                        return result;
                    }
                }
            }
        }
        else
        {
            tString statValue = playerData.getAnyValue(statName);
            if (statValue.Contains(searchPhrase))
            {
                if (globalFoundCount == 0)
                    result << statLabel << " - ";

                if (globalFoundCount > 0)
                    result << ", ";

                result << statsPair.first << ": " << statValue;
                globalFoundCount++;

                if (globalFoundCount >= max_results)
                {
                    result << "\n";
                    return result;
                }
            }
        }
    }

    if (result.empty() && globalFoundCount == 0)
    {
        if (searchAllStats)
            result << "No results found for '" << searchPhrase << "' across all stats.\n";
        else
            result << "No results found for '" << searchPhrase << "' in " << statName << ".\n";
    }

    return result;
}

tString sayOutLoudFunc(tString message)
{
    tString output;
    eChatBot &bot = eChatBot::getInstance();

    // Force public message
    bot.Messager()->Params().preAppend = "";

    output << "> "
           << message;

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

    int page = 1;
    int lastSpace = -1;
    for (int i = message.Len() - 1; i >= 0; --i)
    {
        if (isblank(message[i]))
        {
            lastSpace = i;
            break;
        }
    }

    if (lastSpace != -1)
    {
        tString possiblePage = message.SubStr(lastSpace + 1).TrimWhitespace();
        if (possiblePage.isNumber())
        {
            page = atoi(possiblePage.c_str());
            message = message.SubStr(0, lastSpace).TrimWhitespace();
        }
    }

    PlayerData *stats;
    tString playerName = message.ExtractNonBlankSubString(pos);

    bool all = playerName.ToLower() == "all";
    std::vector<std::string> chatsToDisplay;
    const size_t pageSize = se_playerMessageTriggersChatSearchMaxResults;
    tString param = message.ExtractNonBlankSubString(pos);
    size_t totalNumOfChats = 0;

    if (all)
    {
        bool params = !param.empty();
        if (param.isNumber() || !params)
        {
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
            indicesToSwap.insert(indicesToSwap.end(), pair.second.begin(), pair.second.end());
    }

    std::sort(indicesToSwap.begin(), indicesToSwap.end());

    for (size_t i = 0; i < indicesToSwap.size(); i += 2)
    {
        if (i + 1 < indicesToSwap.size())
            std::swap(input[indicesToSwap[i]], input[indicesToSwap[i + 1]]);
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

    if (triggeredBy->lastBannedPlayerTime != 0 && tSysTimeFloat() - triggeredBy->lastBannedPlayerTime < 5)
        return output;

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

        if (bot.Messager()->Params().response.empty())
            gHelperUtility::Debug("eChatBot", "No trigger set for '$alreadybanned' Set one with 'PLAYER_MESSAGE_TRIGGERS_ADD'\n");


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

tString unvalidatedsaynoprefixfunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    bot.Messager()->Params().validateOutput = false;
    bot.Messager()->Params().preAppend = tString("");

    return message;
}

tString encryptedUnvalidatedSayFunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeredByPlayer = bot.Messager()->Params().triggeredBy;

    if (!triggeredByPlayer || !triggeredByPlayer->encryptVerified )
        return tString("You are not verified");

    bot.Messager()->Params().validateOutput = false;
    bot.Messager()->Params().preAppend = tString("");

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

    FileManager fileManager(se_playerMessageTriggersFuncLogFile, tDirectories::Var());

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
        lines.Reverse();

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
                    logMessage << "," << logParts[j];

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
        output << "Logs ("
               << requestedPage
               << "/"
               << totalPages
               << "): "
               << pages[requestedPage - 1];

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
        logEntry << timeStr
                 << ","
                 << playerName
                 << ","
                 << logMessage.TrimWhitespace();

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
            output << "I'm thinking of a number between "
                   << minRange
                   << " and "
                   << maxRange
                   << ". Try to guess it!";
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
            output << "Invalid input. Please enter a number between "
                   << minRange
                   << " and "
                   << maxRange
                   << ".";
            return output;
        }

        int guess = atoi(command.c_str());

        if (guess < minRange || guess > maxRange)
        {
            tString output;
            output << "Your guess is out of range. Please enter a number between "
                   << minRange
                   << " and "
                   << maxRange
                   << ".";
            return output;
        }

        attempts++;

        tString output;

        if (guess < randomValue)
        {
            output << "Too low! Attempts: "
                   << attempts
                   << ".";
        }
        else if (guess > randomValue)
        {
            output << "Too high! Attempts: "
                   << attempts
                   << ".";
        }
        else
        {
            time_t endTime = time(NULL);
            int elapsedTime = static_cast<int>(difftime(endTime, startTime));

            tString successMessage;
            successMessage << "Congratulations, "
                           << player->GetName()
                           << "! You guessed the number in "
                           << attempts
                           << " attempt";
            if (attempts != 1)
                successMessage << "s";
            successMessage << " and "
                           << elapsedTime
                           << " second";
            if (elapsedTime != 1)
                successMessage << "s";
            successMessage << "!";
            gameStarted = false;
            return successMessage;
        }

        if (se_playerMessageTriggersGuessGameGiveupAnnounceInterval > 0 &&
            attempts % se_playerMessageTriggersGuessGameGiveupAnnounceInterval == 0)
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

tString nameHistoryTrackerFunc(tString message)
{
    tString result;
    tString input = message.TrimWhitespace();
    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *triggeringPlayer = bot.Messager()->Params().triggeredBy;

    if (!se_playerStats)
        return tString("PlayerStats not initialized. PlayerStats are disabled!\n");

    if (input.empty())
    {
        if (!triggeringPlayer)
            return tString("No player provided and no trigger context.");
        input = triggeringPlayer->GetName();
    }
    else
    {
        ePlayerNetID *player = ePlayerNetID::GetPlayerByName(input, false);
        if (player)
            input = player->GetName();
    }

    std::set<std::string> seen;
    std::vector<tString> history;

    std::function<void(tString)> trace;
    trace = [&](tString name)
    {
        std::string lowered = name.ToLower().stdString();
        if (seen.count(lowered))
            return;

        seen.insert(lowered);

        PlayerData &data = ePlayerStats::getStatsForAnalysis(name);
        tString canonical = !data.name.empty() ? data.name : name;
        history.push_back(canonical);

        for (auto it = data.name_history.rbegin(); it != data.name_history.rend(); ++it)
            trace(tString(*it));

        for (const auto &entry : ePlayerStats::GetPlayerStatsMap())
        {
            const PlayerData &other = entry.second;
            if (other.deleted || other.name_history.empty())
                continue;

            std::string prev = other.name_history.back();
            if (tString(prev).ToLower().stdString() == lowered)
            {
                tString next = !other.name.empty() ? other.name : entry.first;
                trace(next);
            }
        }
    };

    trace(input);
    std::reverse(history.begin(), history.end());

    std::set<std::string> finalSeen;
    std::vector<tString> orderedNames;
    for (const tString &name : history)
    {
        std::string lowered = name.ToLower().stdString();
        if (!finalSeen.count(lowered))
        {
            finalSeen.insert(lowered);
            orderedNames.push_back(name);
        }
    }

    tString chain;
    for (size_t i = 0; i < orderedNames.size(); ++i)
    {
        if (i > 0)
            chain << " --> ";
        chain << orderedNames[i];
    }

    result << orderedNames.back() << ": Name History - " << chain;
    return result;
}

tString reminderFunc(tString message)
{
    tString output;
    eChatBot &bot = eChatBot::getInstance();

    message = message.TrimWhitespace();

    if (message.empty())
    {
        output << "Usage: '" << bot.Messager()->Params().matchedTrigger
               << " <delay> <message>' (e.g. 10s, 5m, 2.5h, 1d, 2w, 1mo, 1y)";
        return output;
    }

    int pos = 0;
    tString secondsStr = message.ExtractNonBlankSubString(pos);
    tString reminderText = message.SubStr(pos).TrimWhitespace();

    if (secondsStr.empty() || reminderText.empty())
    {
        output << "Usage: '" << bot.Messager()->Params().matchedTrigger
               << " <delay> <message>' (e.g. 10s, 5m, 2.5h, 1d, 2w, 1mo, 1y)";
        return output;
    }

    bool valid = false;
    double delay = ParseTimeString(secondsStr.stdString(), valid);
    if (!valid || delay <= 0)
    {
        output << "Invalid delay. Try formats like 10s, 5m, 2h, 1d, 2w, 1mo, 1y.";
        return output;
    }

    static int reminderID = 0;
    std::string id = "chat_reminder_once_" + std::to_string(reminderID++);

    std::string prefix;
    if (bot.Messager()->Params().triggeredBy)
        prefix = bot.Messager()->Params().triggeredBy->GetName().stdString() + ": ";

    tString formatted;
    formatted = st_GetFormatTime(delay, false, false);
    tString secondsElapsed;
    secondsElapsed << " (" << formatted << " elapsed)";

    tString fullCmd;
    fullCmd << se_playerMessageReminderFuncCommand << " " << prefix << reminderText << secondsElapsed;
    std::string fullCommand = fullCmd.stdString();

    CommandScheduler::AddCommand(id, fullCommand, delay, false);

    output << prefix << "Reminder will trigger in " << formatted << ": " << reminderText;

    gHelperUtility::Debug("eChatBot", "CommandScheduler scheduled: " + fullCommand + " in " + std::to_string((int)delay) + "s");
    return output;
}

tString reminderListFunc(tString message)
{
    tString output;
    eChatBot &bot = eChatBot::getInstance();
    tString trigger = bot.Messager()->Params().matchedTrigger;
    ePlayerNetID *triggeredBy = bot.Messager()->Params().triggeredBy;

    message = message.TrimWhitespace();

    int page = 1;

    if (!message.empty())
    {
        if (message.isNumber())
            page = atoi(message.c_str());
        else
        {
            output << "Usage: '" << trigger << " <page>'";
            return output;
        }
    }

    std::vector<CommandScheduler::ScheduledCommand> sortedReminders;
    for (const auto &r : CommandScheduler::scheduledCommands)
    {
        if (r.id.rfind("chat_reminder_", 0) == 0)
            sortedReminders.push_back(r);
    }

    if (sortedReminders.empty())
        return tString("No upcoming reminders.");

    std::sort(sortedReminders.begin(), sortedReminders.end(),
              [](const CommandScheduler::ScheduledCommand &a, const CommandScheduler::ScheduledCommand &b)
              {
                  return a.triggerAt < b.triggerAt;
              });


    double nowWall = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

    const size_t maxLen = se_SpamMaxLen - 1;
    std::vector<tString> pages;
    tString currentPage;
    size_t currentLen = 0;

    for (const auto &r : sortedReminders)
    {
        double remaining = std::max(0.0, r.triggerAt - nowWall);
        tString whenStr = st_GetFormatTime(remaining, false, false);

        tString line;
        line << "[" << r.id << "] in " << whenStr << ": " << r.command;

        size_t len = line.Len();
        size_t sep = (currentLen == 0) ? 0 : 2;

        if (currentLen + sep + len > maxLen)
        {
            if (currentLen > 0)
                pages.push_back(currentPage);

            currentPage = line;
            currentLen = len;
        }
        else
        {
            if (currentLen > 0)
            {
                currentPage << ", ";
                currentLen += 2;
            }
            currentPage << line;
            currentLen += len;
        }
    }

    if (currentLen > 0)
        pages.push_back(currentPage);

    if (page <= 0)
        page = 1;
    if (page > (int)pages.size())
        page = (int)pages.size();

    output << "Reminders (" << page << "/" << pages.size() << "): " << pages[page - 1];
    return output;
}

tString notifyPlayerFunc(tString message)
{
    tString output;
    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *sender = bot.Messager()->Params().triggeredBy;

    message = message.TrimWhitespace();
    int pos = 0;
    tString targetName = message.ExtractNonBlankSubString(pos);
    tString noteText = message.SubStr(pos).TrimWhitespace();

    if (targetName.empty() || noteText.empty())
    {
        output << "Usage: '" << bot.Messager()->Params().matchedTrigger << " <player> <message>'";
        return output;
    }

    ePlayerNetID *targetPlayer = ePlayerNetID::GetPlayerByName(targetName, false);
    if (targetPlayer)
        targetName = targetPlayer->GetName();

    PlayerData *stats = targetPlayer
        ? &ePlayerStats::getStats(targetPlayer)
        : &ePlayerStats::getStatsForAnalysis(targetName);

    if (!stats || stats->name.empty())
    {
        output << "Player not found: " << targetName;
        return output;
    }

    std::string senderName = sender ? sender->GetName().stdString() : "Unknown";
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tString delim = se_playerMessageNotificationsDelimiter;

    tString formattedNote;
    formattedNote << senderName << delim << noteText << delim << std::to_string(now);
    stats->notifications.push_back(formattedNote.stdString());

    if (sender)
    {
        PlayerData &senderStats = ePlayerStats::getStats(sender);
        tString sentNote;
        sentNote << targetName << delim << noteText << delim << std::to_string(now);
        senderStats.sent_notifications.push_back(sentNote.stdString());
    }

    if (targetPlayer)
    {
        ePlayerNetID *capturedTarget = targetPlayer;
        time_t capturedNow = now;

        gTaskScheduler.schedule("notification", se_playerMessageNotificationsDelay, [capturedTarget, capturedNow]()
        {
            if (capturedTarget)
            {
                PlayerData& targetStats = ePlayerStats::getStats(capturedTarget);
                eChatBot::findResponsePlayer(
                    eChatBot::getInstance(),
                    capturedTarget,
                    se_playerMessageNotificationTrigger,
                    tString(""),
                    tString(""),
                    true
                );
                targetStats.last_seen_notification_time = capturedNow;
            }
        });
    }

    tString displayName(!targetPlayer || ePlayerStats::shouldEnforceLocalName(targetPlayer)
                        ? (!stats->name.empty() ? stats->name : targetName)
                        : targetName);

    output << "Notification sent to " << displayName << ".";
    return output;
}

tString buildPaginatedNotificationList(const std::vector<std::string> &notes, const tString &prefix, bool isSent = false, int page = 1)
{
    std::vector<tString> pages;
    const size_t maxLen = se_SpamMaxLen - 1;
    size_t currentLen = 0;
    tString currentPage;

    for (auto it = notes.rbegin(); it != notes.rend(); ++it)
    {
        tString raw(*it);
        tArray<tString> parts = raw.Split(se_playerMessageNotificationsDelimiter);

        tString line;
        if (parts.Len() >= 3)
        {
            tString name = parts[0], body = parts[1];
            if (se_playerMessageNotificationsListShowTime)
            {
                time_t timestamp = 0;
                try
                {
                    timestamp = std::stoll(parts[2].stdString());
                }
                catch (...)
                {
                }
                struct tm *timeinfo = localtime(&timestamp);
                std::string timeStr = getTimeStringBase(*timeinfo, false);
                line << (isSent ? "[To " : "[") << name << " @ " << timeStr << "] " << body;
            }
            else
            {
                line << (isSent ? "[To " : "[") << name << "] " << body;
            }
        }
        else if (parts.Len() >= 2)
        {
            tString name = parts[0], body = parts[1];
            line << (isSent ? "[To " : "[") << name << "] " << body;
        }
        else
        {
            line = raw;
        }

        size_t len = line.Len();
        size_t sep = (currentLen == 0) ? 0 : 2;

        if (currentLen + sep + len > maxLen)
        {
            if (currentLen > 0)
                pages.push_back(currentPage);
            currentPage = line;
            currentLen = len;
        }
        else
        {
            if (currentLen > 0)
            {
                currentPage << ", ";
                currentLen += 2;
            }
            currentPage << line;
            currentLen += len;
        }
    }

    if (currentLen > 0)
        pages.push_back(currentPage);

    if (page <= 0)
        page = 1;
    if (page > (int)pages.size())
        page = (int)pages.size();

    tString output;
    output << prefix << " (" << page << "/" << pages.size() << "): " << pages[page - 1];
    return output;
}

tString notificationsListFunc(tString message)
{
    ePlayerNetID *player = eChatBot::getInstance().Messager()->Params().triggeredBy;
    if (!player) return tString("No notifications found.");

    PlayerData *stats = &ePlayerStats::getStats(player);
    if (!stats) return tString("No notifications found.");

    int page = 1;
    message = message.TrimWhitespace();
    if (!message.empty())
    {
        if (message.isNumber()) page = atoi(message.c_str());
        else return tString("Usage: '<trigger> <page>'");
    }

    if (stats->notifications.empty())
        return tString("You have no notifications.");

    stats->last_seen_notification_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    return buildPaginatedNotificationList(stats->notifications, player->GetName(), false, page);
}

tString sentNotificationsListFunc(tString message)
{
    ePlayerNetID *sender = eChatBot::getInstance().Messager()->Params().triggeredBy;
    if (!sender) return tString("No sender context.");

    message = message.TrimWhitespace();
    tString playerName;
    int page = 1;

    tArray<tString> parts = message.Split(" ");
    if (parts.Len() == 1)
    {
        if (parts[0].isNumber())
            page = atoi(parts[0].c_str());
        else
            playerName = parts[0];
    }
    else if (parts.Len() >= 2)
    {
        playerName = parts[0];
        if (parts[1].isNumber()) page = atoi(parts[1].c_str());
    }

    PlayerData *stats = nullptr;
    if (!playerName.empty())
    {
        ePlayerNetID *targetPlayer = ePlayerNetID::GetPlayerByName(playerName, false);
        if (targetPlayer) playerName = targetPlayer->GetName();
        stats = targetPlayer
            ? &ePlayerStats::getStats(targetPlayer)
            : &ePlayerStats::getStatsForAnalysis(playerName);
    }
    else
    {
        stats = &ePlayerStats::getStats(sender);
        playerName = sender->GetName();
    }

    if (!stats) return tString("Could not locate stats.");
    if (stats->sent_notifications.empty()) return tString("No sent notifications found.");

    return buildPaginatedNotificationList(stats->sent_notifications, playerName, true, page);
}

tString viewNotificationsFunc(tString message)
{
    eChatBot &bot = eChatBot::getInstance();
    ePlayerNetID *player = bot.Messager()->Params().triggeredBy;
    if (!player) return tString("You must be a player to use this command.");

    message = message.TrimWhitespace();
    tString targetName;
    int page = 1;

    tArray<tString> parts = message.Split(" ");
    if (parts.Len() == 1)
    {
        if (parts[0].isNumber())
            page = atoi(parts[0].c_str());
        else
            targetName = parts[0];
    }
    else if (parts.Len() >= 2)
    {
        targetName = parts[0];
        if (parts[1].isNumber()) page = atoi(parts[1].c_str());
    }

    if (targetName.empty())
        targetName = player->GetName();

    if (se_playerMessageNotificationsRestrictView &&
        targetName != player->GetName() &&
        !player->encryptVerified)
    {
        return tString("You are not verified.");
    }

    ePlayerNetID *targetPlayer = ePlayerNetID::GetPlayerByName(targetName, false);
    if (targetPlayer) targetName = targetPlayer->GetName();

    PlayerData *stats = targetPlayer
        ? &ePlayerStats::getStats(targetPlayer)
        : &ePlayerStats::getStatsForAnalysis(targetName);

    if (!stats) return tString("Player not found: ") + targetName;
    if (stats->notifications.empty())
        return tString(targetPlayer ? targetName : stats->name) + " has no notifications.";

    return buildPaginatedNotificationList(stats->notifications, targetName, false, page);
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
    Functions()->RegisterFunction("$encryptedunvalidatedsayfunc", encryptedUnvalidatedSayFunc);
    Functions()->RegisterFunction("$unvalidatedsaynoprefixfunc", unvalidatedsaynoprefixfunc);
    Functions()->RegisterFunction("$namehistorytrackerfunc", nameHistoryTrackerFunc);
    Functions()->RegisterFunction("$reminderfunc", reminderFunc);
    Functions()->RegisterFunction("$reminderlistfunc", reminderListFunc);
    Functions()->RegisterFunction("$notifyfunc", notifyPlayerFunc);
    Functions()->RegisterFunction("$notificationslistfunc", notificationsListFunc);
    Functions()->RegisterFunction("$sentnotificationslistfunc", sentNotificationsListFunc);
    Functions()->RegisterFunction("$viewnotificationslistfunc", viewNotificationsFunc);
}

void eChatBot::LoadChatTriggers()
{
    gHelperUtility::Debug("eChatBot","Loading chat triggers...");
    FileManager fileManager(se_playerMessageTriggersFile, tDirectories::Var());
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

                chatTrigger.isExpedientTrigger = false;
                tArray<tString> keys = se_playerMessageEnabledExpedientKeys.Split(",");
                for (int i = 0; i < keys.Len(); ++i)
                {
                    if (trigger.StartsWith(keys[i]))
                    {
                        chatTrigger.isExpedientTrigger = true;
                        break;
                    }
                }

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
    tArray<tString> players = se_playerMessageEnabledPlayers.SplitForPlayers();

    for (int i = 0; i < players.Len(); i++)
    {
        ePlayer *local_p = ePlayer::PlayerConfig(atoi(players[i]) - 1);
        if (!local_p)
            continue;

        ePlayerNetID *netPlayer = local_p->netPlayer;
        if (!netPlayer)
            continue;

        return true;
    }
    return false;
}
tString eChatBot::findResponsePlayer(eChatBot &bot, ePlayerNetID* triggeredBy, tString trigger, tString value, tString valDelim, bool send)
{
    if (!se_playerMessageTriggers)
        return tString("");

    if (valDelim.empty())
        valDelim = tString("$val1");

    bot.Messager()->ResetParams();
    bot.Messager()->SetInputParams(nullptr, trigger, true);

    bot.Messager()->Params().triggeredBy = triggeredBy;
    bot.Messager()->Params().triggeredByName = triggeredBy->GetName();

    bot.Messager()->FindTriggeredResponse();

    if (!bot.Messager()->Params().response.empty())
        bot.Messager()->Params().response = bot.Messager()->Params().response.Replace(valDelim, value);
    else
        gHelperUtility::Debug("eChatBot", "No trigger set for '" + trigger.stdString() + "' Set one with 'PLAYER_MESSAGE_TRIGGERS_ADD'\n");

    if (send && !bot.Messager()->Params().response.empty())
        bot.Messager()->Send();

    return bot.Messager()->Params().response;
}


tString eChatBot::findResponse(eChatBot &bot, tString playerName, tString trigger, tString value, tString valDelim, bool send)
{
    if (!se_playerMessageTriggers)
        return tString("");

    if (valDelim.empty())
        valDelim = tString("$val1");

    bot.Messager()->ResetParams();
    bot.Messager()->Params().triggeredByName = playerName;
    bot.Messager()->SetInputParams(nullptr, trigger, true);
    bot.Messager()->FindTriggeredResponse();

    if (!bot.Messager()->Params().response.empty())
        bot.Messager()->Params().response = bot.Messager()->Params().response.Replace(valDelim, value);
    else
        gHelperUtility::Debug("eChatBot", "No trigger set for '" + trigger.stdString() + "' Set one with 'PLAYER_MESSAGE_TRIGGERS_ADD'\n");

    if (send && !bot.Messager()->Params().response.empty())
        bot.Messager()->Send();

    return bot.Messager()->Params().response;
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

bool containsMath(tString input, bool exact)
{
    input = stripNonOperatorsOrNumbers(input);

    int i = 0;
    int step = 0; // 0 = expecting first number, 1 = operator, 2 = second number

    while (i < input.Len())
    {
        while (i < input.Len() && isblank(input(i)))
            ++i;

        if (i < input.Len() && (input(i) == '-' || input(i) == '+'))
            ++i;

        int numberStart = i;
        bool hasDigits = false;
        bool hasDot = false;

        while (i < input.Len())
        {
            char c = input(i);
            if (std::isdigit(c))
            {
                hasDigits = true;
                ++i;
            }
            else if (c == '.' && !hasDot)
            {
                hasDot = true;
                ++i;
            }
            else break;
        }

        if (hasDigits)
        {
            tString token = input.SubStr(numberStart, i - numberStart);

            if (step == 0)
                step = 1;
            else if (step == 2)
                return true;
        }
        else if (exact)
            return false;

        while (i < input.Len() && isblank(input(i)))
            ++i;

        if (step == 1 && i < input.Len() &&
            (input(i) == '+' || input(i) == '-' || input(i) == '*' ||
             input(i) == '/' || input(i) == '^'))
        {
            ++i;
            step = 2;
        }
        else if (step == 2 && i < input.Len() && std::isdigit(input(i)))
            return true;
        else
            ++i;
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
        if (std::isdigit(static_cast<unsigned char>(c)) ||
            (c == '+' || c == '-' || c == '.' || c == '*' || c == '/' || c == '^'))
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

    if (!se_playerMessageTriggersIgnorePhrases.empty())
    {
        tArray<tString> phrases = se_playerMessageTriggersIgnorePhrases.Split(",");
        for (int i = 0; i < phrases.Len(); ++i)
        {
            phrases[i].ToLower();
            if (lowerMessage.Contains(phrases[i]))
            {
                gHelperUtility::Debug("eChatBot","Input message contained ignored phrase: " + phrases[i].stdString());
                Params().abortOutput = true;
                return;
            }
        }
    }

    if (!se_playerMessageTriggersIgnorePhrasesStartsWith.empty())
    {
        tArray<tString> phrases = se_playerMessageTriggersIgnorePhrasesStartsWith.Split(",");
        for (int i = 0; i < phrases.Len(); ++i)
        {
            phrases[i].ToLower();
            if (lowerMessage.StartsWith(phrases[i]))
            {
                gHelperUtility::Debug("eChatBot","Input message started with ignored phrase: " + phrases[i].stdString());
                Params().abortOutput = true;
                return;
            }
        }
    }

    if (!se_playerMessageTriggersIgnorePhrasesEndsWith.empty())
    {
        tArray<tString> phrases = se_playerMessageTriggersIgnorePhrasesEndsWith.Split(",");
        for (int i = 0; i < phrases.Len(); ++i)
        {
            phrases[i].ToLower();
            if (lowerMessage.EndsWith(phrases[i]))
            {
                gHelperUtility::Debug("eChatBot","Input message ended with ignored phrase: " + phrases[i].stdString());
                Params().abortOutput = true;
                return;
            }
        }
    }

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

        if (tIsInList(se_playerMessageTriggersJoinedRecentlyVerifiedTriggers, trigger))
        {
            if (Params().sendingPlayer == nullptr)
                continue;

            if (tSysTimeFloat() - Params().sendingPlayer->createdTime() > se_playerMessageTriggersJoinedRecentlyWaitTime &&
                tSysTimeFloat() - Params().sendingPlayer->lastTeamCreateTime > se_playerMessageTriggersJoinedRecentlyWaitTime)
                continue;
        }


        if (tIsInList(se_playerMessageTriggersChatChanceTriggers, trigger))
        {
            tRandomizer &randomizer = tRandomizer::GetInstance();
            REAL roll = randomizer.Get();
            gHelperUtility::Debug("ChanceTrigger", "Roll", roll);

            if (roll > se_playerMessageTriggersChatChanceTriggersThreshhold)
            {

                if (sg_helperDebug)
                    gHelperUtility::Debug("ChanceTrigger", "Skipped trigger due to roll",
                                          tString(trigger) << " (" << roll << " > " << se_playerMessageTriggersChatChanceTriggersThreshhold << ")");
                return;
            }
        }

        Params().matchedTrigger     = chatTrigger.trimmedTrigger;
        Params().isExpedientTrigger = chatTrigger.isExpedientTrigger;

        // Determine the sending player based on the type of trigger
        if (Params().triggeredBy != nullptr)
        {
            if (potentialSender == nullptr)
            {
                if (tIsInList(se_playerMessageTriggersDiedByVerifiedTriggers, trigger))
                {
                    if (Params().triggeredBy->lastKilledPlayer == nullptr || !Params().triggeredBy->lastKilledPlayer->isLocal())
                        continue;
                    else
                        potentialSender = Params().triggeredBy->lastKilledPlayer;
                }
                else if (tIsInList(se_playerMessageTriggersKillVerifiedTriggers, trigger))
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

        if (Params().triggeredBy)
            chosenResponse         = chosenResponse.Replace("$p2", Params().triggeredBy->GetName());

        if (Params().sendingPlayer)
        {
            chosenResponse.RecomputeLength();
            tString ourName = Params().sendingPlayer->GetName();
            chosenResponse  = chosenResponse.Replace("$p0", ourName);
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

            int openParenPos  = chosenResponse.StrPos(dollarPos + 1, "(");
            int closeParenPos = -1;

            tString functionName;
            tString functionInput;

            if (openParenPos != -1)
            {
                closeParenPos = chosenResponse.StrPos(openParenPos + 1, ")");
                if (closeParenPos != -1)
                {
                    functionName  = chosenResponse.SubStr(dollarPos, openParenPos - dollarPos).TrimWhitespace();
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

void eChatBot::SilencedAction()
{
    if (sg_helperDebug)
        gHelperUtility::Debug("eChatBot", "Last message was silenced.");

    eChatBot &bot = eChatBot::getInstance();
    auto &messenger = *bot.Messager();

    bool ableToChat = ePlayerNetID::canChat();
    bool ableToChatCommonPrefix = ePlayerNetID::canChatCommonPrefix(messenger.Params().response);
    bool canChat = (ableToChat && ableToChatCommonPrefix);

    messenger.Params().pentalized_for_last_message = !ableToChat;
    messenger.Params().pentalized_for_last_message_prefix = !ableToChatCommonPrefix;

    if (sg_helperDebug)
    {
        gHelperUtility::Debug("eChatBot", "Clearing all playerMessageTask from the TaskScheduler.");
        gHelperUtility::Debug("eChatBot", std::string("[SilencedAction] Penalized for message: ") + (messenger.Params().pentalized_for_last_message ? "true" : "false"));
        gHelperUtility::Debug("eChatBot", std::string("[SilencedAction] Penalized for prefix: ") + (messenger.Params().pentalized_for_last_message_prefix ? "true" : "false"));
    }

    tString silencedMessage = messenger.Params().currentPartIndex < messenger.Params().messageParts.size()
        ? messenger.Params().messageParts[messenger.Params().currentPartIndex].message
        : tString("");

    gTaskScheduler.removeTasksWithPrefix("playerMessageTask");

    std::vector<eChatBotData::MessagePart> partsToBeResent;
    partsToBeResent.reserve(messenger.Params().messageParts.size());

    size_t totalParts = messenger.Params().messageParts.size();

    for (size_t i = messenger.Params().currentPartIndex; i < totalParts; ++i)
    {
        auto &part = messenger.Params().messageParts[i];

        bool found = false;
        bool lastWasSilenced = ePlayerMessages.CheckIfSilenced();

        for (auto it = ePlayerMessages.incomingMessages.rbegin(); it != ePlayerMessages.incomingMessages.rend(); ++it)
        {
            if (it->StartsWith(part.message))
            {
                found = true;
                break;
            }
        }

        if (sg_helperDebug)
        {
            gHelperUtility::Debug("eChatBot", std::string("Checking message part[") + std::to_string(i) + "]: " + part.message.stdString());
            gHelperUtility::Debug("eChatBot", std::string("Matched incoming: ") + (found ? "true" : "false") + ", Last silenced: " + (lastWasSilenced ? "true" : "false") + ", Equals silencedMessage: " + ((part.message == silencedMessage) ? "true" : "false"));
        }

        if (found && ((lastWasSilenced && part.message == silencedMessage) || messenger.Params().pentalized_for_last_message_prefix))
        {
            if (sg_helperDebug)
                gHelperUtility::Debug("eChatBot", "Forcing resend due to silenced match or prefix penalty.");
            found = false;
        }

        if (!found)
            partsToBeResent.push_back(part);
        else
            messenger.Params().currentPartIndex = i + 1;
    }

    if (partsToBeResent.empty())
        return;

    REAL penaltyDelay = 0.0;
    if (messenger.Params().pentalized_for_last_message)
    {
        penaltyDelay = ((ePlayerNetID::nextSpeakTime - getSteadyTime()) * 1.1)
                       + se_playerMessageTriggersResendSilencedMessagesExtraDelay;
    }

    REAL perPartSpacing = 0.2f;

    for (size_t j = 0; j < partsToBeResent.size(); j++)
    {
        auto &p = partsToBeResent[j];

        bool retryingSilencedMessage = se_playerMessageTriggersResendSilencedPrefixDuplicatedMessages && (p.message == silencedMessage);

        if ((retryingSilencedMessage || messenger.Params().pentalized_for_last_message_prefix) && !p.message.StartsWith("/"))
        {
            tString randomPrefix;
            randomPrefix << randomStr(se_playerMessageTriggersResendSilencedMessagesPrefixList.stdString(),
                                      se_playerMessageTriggersResendSilencedMessagesPrefixAmount)
                         << " ";

            if (!messenger.Params().preAppend.empty() && p.message.StartsWith(messenger.Params().preAppend))
            {
                p.message = messenger.Params().preAppend + (randomPrefix + p.message.SubStr(messenger.Params().preAppend.Len()));
            }
            else
            {
                p.message = randomPrefix + p.message;
            }

            if (sg_helperDebug)
                gHelperUtility::Debug("eChatBot", std::string("Adding prefix for retry: ") + p.message.stdString());
        }

        if (p.delay <= 0)
            p.delay = penaltyDelay + (j * perPartSpacing);
        else
            p.delay += penaltyDelay + (j * perPartSpacing);
    }

    messenger.Params().messageParts = partsToBeResent;
    messenger.Params().currentPartIndex = 0;

    if (sg_helperDebug)
    {
        tString debugMsg("Parts to be sent: ");
        for (size_t j = 0; j < partsToBeResent.size(); ++j)
        {
            if (j > 0)
                debugMsg << ", ";
            debugMsg << partsToBeResent[j].message;
        }

        gHelperUtility::Debug("eChatBot", debugMsg.stdString());
    }

    messenger.ScheduleMessageParts();
}

REAL SpamProtectionDelayForMsg(const tString &msg)
{
    if (ePlayerNetID::canChatWithMsg(msg))
        return 0.0f;

    const REAL now  = getSteadyTime();
    REAL       wait = 0.0f;

    if (ePlayerNetID::nextSpeakTime > now)
        wait = std::max<REAL>(wait, ePlayerNetID::nextSpeakTime - now);

    if (msg.StartsWith(ePlayerNetID::nextSpeakTimePrefixCommonPrefix) &&
        ePlayerNetID::nextSpeakTimePrefix > now)
        wait = std::max<REAL>(wait,
                              ePlayerNetID::nextSpeakTimePrefix - now);

    return std::max<REAL>(wait, 0.0f);
}


bool eChatBotMessager::Send()
{
    if (!helperConfig::sghuk)
        return false;

    if (!Params().sendingPlayer)
    {
        gHelperUtility::Debug("eChatBot","No sending player set. Attemping to set one");
        for (auto *localNetPlayer : se_GetPlayerMessageEnabledPlayers())
        {
            Params().sendingPlayer = localNetPlayer;
            break;
        }
    }

    if (!Params().sendingPlayer ||
        !tIsEnabledForPlayer(se_playerMessageEnabledPlayers,
                             Params().sendingPlayer->pID + 1))
    {
        gHelperUtility::Debug("eChatBot", "No sending player set. Aborting");
        return false;
    }

    tString messageToSend = Params().response;

    if (Params().isExpedientTrigger)
    {
        if (messageToSend.Len() > se_playerMessageTriggersSpamMaxlen)
            messageToSend = messageToSend.SubStr(0, se_playerMessageTriggersSpamMaxlen);

        if (!Params().preAppend.empty())
            messageToSend = Params().preAppend + messageToSend;

        Params().sendingPlayer->Chat(messageToSend, true);

        Stats().total_messages_sent++;
        return true;
    }

    Params().pentalized_for_last_message_prefix = !ePlayerNetID::canChatCommonPrefix(messageToSend) ||
                                                ( !ePlayerNetID::nextSpeakTimePrefixCommonPrefix.empty() && messageToSend.StartsWith(ePlayerNetID::nextSpeakTimePrefixCommonPrefix));

    if (!messageToSend.StartsWith("/") && Params().pentalized_for_last_message_prefix)
    {
        if (sg_helperDebug)
        {
            tString debug;
            debug << "Message not able to be sent because " << messageToSend << " starts with " << ePlayerNetID::nextSpeakTimePrefixCommonPrefix << " adding prefix";
            gHelperUtility::Debug("eChatBot", debug.stdString());
        }

        tString randomPrefix;
        randomPrefix << randomStr(se_playerMessageTriggersResendSilencedMessagesPrefixList.stdString(),
                                    se_playerMessageTriggersResendSilencedMessagesPrefixAmount)
                     << " ";

        messageToSend = randomPrefix + messageToSend;
    }

    bool  forceSpecialDelay = (Params().delay < -5);

    if (messageToSend.Len() > se_playerMessageTriggersSpamMaxlen)
    {
        gHelperUtility::Debug("eChatBot",
                              "messageToSend cut because length > " +
                                  std::to_string(se_playerMessageTriggersSpamMaxlen) +
                                  " Length: " +
                                  std::to_string(messageToSend.Len()));
        messageToSend = messageToSend.SubStr(0, se_playerMessageTriggersSpamMaxlen);
    }

    REAL extraDelay = 0.0;

    if (se_playerMessageSmartDelay)
        extraDelay += calculateResponseSmartDelay(messageToSend,
                                                  se_playerMessageSmartDelayTypingWPM)
                    + se_playerMessageSmartDelayReactionTime;

    if (se_playerMessageDelay > 0)
        extraDelay += se_playerMessageDelay;

    if (se_playerMessageDelayRandMult > 0)
        extraDelay += (REAL)rand() / RAND_MAX * se_playerMessageDelayRandMult;

    const REAL totalDelay = Params().delay + extraDelay;

    const size_t maxMessageLength = se_SpamMaxLen - 1;
    tArray<tString> messageParts =
        messageToSend.SplitBySize(maxMessageLength, true);

    const int   startPartIndex = std::max(0, (int)Params().currentPartIndex);
    const int   numberOfParts  = messageParts.Len() - startPartIndex;
    const bool  applySpamCheck = se_playerMessageTriggersSpamProtectionCheck;

    REAL spamDelay = 0.0;
    if (applySpamCheck)
        spamDelay = std::max(SpamProtectionDelayForMsg(messageToSend), 0.0f);

    bool scheduled = false;

    for (int i = 0; i < messageParts.Len(); ++i)
    {
        tString partText = messageParts[i];
        partText.RecomputeLength();

        if (!Params().preAppend.empty())
            partText = Params().preAppend + partText;

        REAL delayForPart = 0.0;

        if (forceSpecialDelay)
        {
            delayForPart = (i == 0) ? 0.0
                                    : se_playerMessageTriggersDelayForNegativeParts;
        }
        else
        {
            delayForPart = (numberOfParts > 0)
                               ? (totalDelay / numberOfParts)
                               : totalDelay;
        }

        if (i == 0 && spamDelay > 0.0)
            delayForPart += spamDelay;

        eChatBotData::MessagePart part(
            partText,
            delayForPart,
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
    std::string baseId = "playerMessageTask";
    gTaskScheduler.ClearQueueIfOverloaded(se_playerMessageTriggersQueueMaxOverloadedSize, baseId);

    ePlayerNetID* player = Params().sendingPlayer;
    if (!player)
        return false;

    bool scheduled = false;

    size_t currentPartIndex = Params().currentPartIndex;
    size_t totalParts = Params().messageParts.size();
    if (totalParts == 0)
        return false;

    REAL cumulativeDelay = 0.0;

    for (size_t i = currentPartIndex; i < totalParts; ++i)
    {
        const auto& part = Params().messageParts[i];
        const tString& message = part.message;

        std::string taskId = baseId;
        if (se_playerMessageTriggerScheduleMultiple)
            taskId += "_" + std::to_string(player->ID()) + "_" + std::to_string(i)
                      + "_" + std::to_string(tSysTimeFloat());

        gTaskScheduler.schedule(
            taskId,
            cumulativeDelay,
            [player, part, message, taskId]()
            {
                REAL flagDelay    = part.delay * part.chatFlagPercentage;
                REAL messageDelay = part.delay - flagDelay;

                if (part.useChatFlag)
                {
                    gTaskScheduler.schedule(
                        taskId + "_flag",
                        flagDelay,
                        [player, messageDelay, message, taskId]()
                        {
                            player->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, true);
                            ePlayerNetID::Update();

                            std::string followUpId = taskId + "_end";
                            gTaskScheduler.schedule(
                                followUpId,
                                messageDelay,
                                [player, message]()
                                {
                                    player->Chat(message, true);
                                    player->SetChatting(ePlayerNetID::ChatFlags::ChatFlags_Chat, false);
                                    ePlayerNetID::Update();
                                },
                                0,
                                true // once
                            );
                        },
                        0,
                        true // once
                    );
                }
                else
                {
                    gTaskScheduler.schedule(
                        taskId + "_msg",
                        part.delay,
                        [player, message]()
                        {
                            player->Chat(message, true);
                        },
                        0,
                        true // once
                    );
                }
            },
            0,  // repeats
            true // run once
        );

        cumulativeDelay += part.delay;

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
    FileManager fileManager(se_playerMessageTriggersFile, tDirectories::Var());

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
        chatTrigger.trigger    = trigger;
        chatTrigger.responses  = responses;
        chatTrigger.extraDelay = extraDelay;
        chatTrigger.exact      = exact;

        bot.data.chatTriggers.push_back(chatTrigger);
    }

    con << "Trigger, Response, Extra Delay, Exact?\n"
        << "Added: "
        << params
        << "\n";
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

    FileManager fileManager(se_playerMessageTriggersFile, tDirectories::Var());

    int lineNumber = atoi(params.c_str()) - 1;

    if (fileManager.Clear(lineNumber))
        con << "Removed line "
            << lineNumber + 1
            << "\n";

    eChatBot &bot = eChatBot::getInstance();
    bot.data.chatTriggers.clear();
    bot.LoadChatTriggers();
}

static void ListChatTriggers(std::istream &s)
{
    eChatBot &chatBot = eChatBot::getInstance();

    if (se_playerMessageTriggers && !chatBot.data.chatTriggers.empty())
    {
        con << "Listing all loaded chat triggers:\n"
            << "Line) Trigger, Response, Extra Delay, Exact?\n";

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
        FileManager fileManager(se_playerMessageTriggersFile, tDirectories::Var());
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

            con << i + 1    << ") "
                << parts[0] << ", "
                << parts[1] << ", "
                << parts[2] << ", "
                << (parts[3] == "1" ? "Yes" : "No")
                << "\n";
        }
    }
}

static void ClearChatTriggers(std::istream &s)
{
    FileManager fileManager(se_playerMessageTriggersFile, tDirectories::Var());
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

void eChatBotData::StoreContextItem(const tString &item)
{
    if (item.empty())
    {
        gHelperUtility::Debug("eChatBotData", "Context item is empty. Not storing.");
        return;
    }

    contextItems.push_back(item);

    if (sg_helperDebug)
        gHelperUtility::Debug("eChatBotData", "Context item stored: ", item);

    if (se_playerMessageTriggersContextBuilderStore)
    {
        tString line;

        if (se_playerMessageTriggersContextBuilderAdditionalContext)
            line << ExtractAdditionalContextItems() << "\n";

        if (se_playerMessageTriggersContextBuilderStoreTimestamp)
        {
            struct tm now = getCurrentLocalTime();
            line << "["
                 << st_GetCurrentTime("%Y-%m-%d", now)
                 << " "
                 << getTimeStringBase(now)
                 << " "
                 << getTimeZone(now)
                 << "] ";
        }

        line << item;

        FileManager fileManager(se_playerMessageTriggersContextBuilderStoreFileName, tDirectories::Var());
        fileManager.Write(line);
    }
}

tString eChatBotData::ExtractAdditionalContextItems(const char *newLabel, const char *newKey)
{
    tString addlContext;

    auto store = [this, &addlContext](const char *label, const char *key) {
        if (!label || !*label || !key || !*key)
            return;

        tString context;
        context << label << ": " << whatsTheFunc(tString(key));

        if (!context.empty()) {
            if (!addlContext.empty()) {
                addlContext << " | ";
            }
            addlContext << context;
        }
    };


    if (newLabel && *newLabel && newKey && *newKey)
    {
        store(newLabel, newKey);
    }
    else
    {
        if (!se_playerMessageTriggersContextBuilderStoreTimestamp)
        {
            store("Date", "current_date");
            store("Time", "current_time");
        }

        store("Game Time", "game_time");
        store("Current Server", "current_server");
        store("Current Score", "player_score");
        store("Player Count", "player_count");
        store("Spectator Count", "spectator_count");
    }

    return addlContext;
}


static void ViewContextItems(std::istream &s)
{
    tString combinedContext;

    eChatBot &bot = eChatBot::getInstance();
    con << "Context: "
        << bot.data.ExtractContext()
        << "\n";
}
static tConfItemFunc ViewContextItemsConf = HelperCommand::tConfItemFunc("PLAYER_MESSAGE_TRIGGERS_CONTEXT_BUILDER_VIEW", &ViewContextItems);

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

    {"last_banned_reason", "TEXT",
    [](sqlite3_stmt *stmt, int &col, const eChatBotStats &stats)
    {
        const char* last_banned_reasonCStr = stats.last_banned_reason.c_str();
        sqlite3_bind_text(stmt, col++, last_banned_reasonCStr, -1, SQLITE_TRANSIENT);
    },
    [](sqlite3_stmt *stmt, int &col, eChatBotStats &stats)
    {
        const unsigned char* text = sqlite3_column_text(stmt, col++);
        if (text != nullptr) {
            stats.last_banned_reason = reinterpret_cast<const char*>(text);
        }
    }},

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


static void PlayerMessageTriggersInput(std::istream &s)
{
    bool eventTrigger = false;
    tString inputMessage;
    ePlayer *conUser = sn_consoleUser();

    if (!conUser)
    {
        gHelperUtility::DebugForce("eChatBot","No console user found..");
        return;
    }

    ePlayerNetID *triggeredBy = conUser->netPlayer;
    eChatBot &bot = eChatBot::getInstance();
    bot.LoadChatTriggers();

    inputMessage.ReadLine(s, true);
    gHelperUtility::DebugForce("eChatBot","Received Input: ", inputMessage);

    bot.Messager()->ResetParams();
    bot.Messager()->SetInputParams(triggeredBy, inputMessage, eventTrigger);

    bot.Messager()->FindTriggeredResponse();

    if (!bot.Messager()->Params().abortOutput)
        gHelperUtility::DebugForce("eChatBot","Received Output:", bot.Messager()->Params().response);
    else if (bot.Messager()->Params().matchFound)
        gHelperUtility::DebugForce("eChatBot", "Aborting output..");
    else
        gHelperUtility::DebugForce("eChatBot", "No match found..");
}
static tConfItemFunc PlayerMessageTriggersInput_conf = HelperCommand::tConfItemFunc("PLAYER_MESSAGE_TRIGGERS_INPUT", &PlayerMessageTriggersInput);
