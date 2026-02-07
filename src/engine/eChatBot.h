#ifndef ArmageTron_eChatBot_H
#define ArmageTron_eChatBot_H

#include "tString.h"
#include "tSysTime.h"

#include <unordered_map>
#include <functional>
#include "defs.h"
#include "ePlayer.h"
#include "sqlite3.h"
#include "tDatabase.h"

extern tString se_playerMessageTriggersFile,
               se_playerMessageNotificationTrigger,
               se_playerMessageNotificationsDelimiter;               

extern bool se_playerMessageTriggers,
            se_playerMessageTriggersReactToSelf,
            se_playerMessageTriggersReactToLocal,
            se_playerMessageTriggersResendSilencedMessages,
            se_playerMessageTriggersRageQuits,
            se_playerMessageTriggersAcheivements,
            se_playerMessageTriggersAcheivementsLocal,
            se_playerMessageTriggersClearOnSilence,
            se_playerMessageTriggersSpamProtectionCheck,
            se_playerMessageTriggersContextBuilder;

extern int se_playerMessageTriggersAcheivementsKillsChangeVal,
           se_playerMessageTriggersAcheivementsKillStreakChangeVal,
           se_playerMessageTriggersAcheivementsMaxKillStreak,
           se_playerMessageTriggersAcheivementsChatsChangeVal,
           se_playerMessageTriggersAcheivementsJoinsChangeVal,
           se_playerMessageTriggersAcheivementsBansChangeVal;


extern REAL SpamProtectionDelayForMsg(const tString &msg);

std::vector<ePlayerNetID *> se_GetPlayerMessageEnabledPlayers();

struct eChatBotStats;
class eChatBotData;

enum ResponseType
{
    NORMAL,
    FUNC,
    SYM_FUNC,
};

struct eChatBotDataBase
{
    struct MessagePart
    {
        MessagePart(tString message, REAL delay, bool useChatFlag = false, REAL chatFlagPercentage = 0)
            : message(message), delay(delay), useChatFlag(useChatFlag), chatFlagPercentage(chatFlagPercentage) {}

        REAL calculateDelay();
        tString message;
        REAL delay;
        REAL chatFlagPercentage;
        bool useChatFlag;
    };

    struct ActionData
    {
        ActionData() : sendingPlayer(nullptr), delay(0.0),
                       response(tString("")), preAppend(tString("")),
                       eventTrigger(false), currentPartIndex(0) {}

        // INPUT PARAMS
        tString      preAppend;
        tString      inputMessage;
        tString      triggeredByName;
        bool         eventTrigger;
        bool         isExpedientTrigger;
        ePlayerNetID *triggeredBy;

        //OUTPUT PARAMS
        REAL         delay;
        tString      response;
        tString      matchedTrigger;
        ResponseType triggerType;
        ePlayerNetID *sendingPlayer;

        bool useChatFlag    = true;

        bool validateOutput = true;

        // SCHEDULED
        size_t currentPartIndex = 0;
        std::vector<MessagePart> messageParts;

        bool matchFound                           = false;
        bool abortOutput                          = false;
        bool pentalized_for_last_message          = false;
        bool pentalized_for_last_message_prefix   = false;
    };


    // DATABASE STATS
    struct eChatBotStatsBase
    {
        tString name                = tString("hackermans"); // Primary key
        int total_messages_read     = 0;
        int total_messages_sent     = 0;
        int times_banned            = 0;
        int times_banned_today      = 0;
        time_t  last_banned         = 0;
        tString last_banned_reason  = tString("");
        REAL total_up_time          = 0;
    };

};

using ChatBotColumnMapping = ColumnMapping<eChatBotStats>;

class eChatBotStats : public eChatBotDataBase::eChatBotStatsBase
{
public:

    static const std::vector<ChatBotColumnMapping> eChatBotStatsMappings;
    REAL upTime(bool current = true)
    {
        return current ? tSysTimeFloat() : total_up_time + tSysTimeFloat();
    }

    REAL messagesSent(bool current = true)
    {
        return current ? total_messages_sent - data_from_db.total_messages_sent : total_messages_sent;
    }

    REAL messagesRead(bool current = true)
    {
        return current ? total_messages_read - data_from_db.total_messages_read : total_messages_read;
    }

    eChatBotStatsBase data_from_db;
};

typedef tString (*ChatFunction)(tString);

class eChatBotFunctions
{
public:
    eChatBotFunctions(eChatBotData *data): data(data) {}

    void RegisterFunction(const tString &name, ChatFunction func)
    {
        functionMap[name] = func;
    }

    void RegisterFunction(const std::string &name, ChatFunction func)
    {
        RegisterFunction(tString(name), func);
    }

    tString ExecuteFunction(const tString &name, const tString &message)
    {
        if (functionMap.find(name) != functionMap.end())
        {
            return functionMap[name](message);
        }
        return tString("");
    }

    bool IsValidFunction(const tString &name)
    {
        return functionMap.find(name) != functionMap.end();
    }

    eChatBotData &Data()
    {
        return *data;
    }

    eChatBotData *data;
    std::map<tString, ChatFunction> functionMap;
};

class eChatBotMessager
{
public:
    eChatBotMessager(eChatBotData *data): data(data) {}

    void SetInputParams(ePlayerNetID *triggeredBy, tString inputMessage, bool eventTrigger, tString preAppend = tString(""));
    void SetOutputParams(tString &response, REAL &delay, ePlayerNetID *sendingPlayer = nullptr);

    void ResetParams();

    void FindTriggeredResponse();

    bool Send();
    bool ResendMessage(tString blockedMessage = tString(""));

    bool ScheduleMessageParts();

    REAL determineReadingDelay(tString message);
    REAL calculateResponseSmartDelay(tString response, REAL wpm);

    eChatBotDataBase::ActionData &Params();

    eChatBotStats &Stats();

    eChatBotData &Data();

    eChatBotData *data;
};

class eChatBotData : public eChatBotDataBase
{
public:
    eChatBotData()
    {
        functions = new eChatBotFunctions(this);
        messager  = new eChatBotMessager(this);
    }
    void StoreContextItem(const tString &item);

    tString ExtractAdditionalContextItems(const char *newLabel = nullptr, const char *newKey = nullptr);

    tString ExtractContext(bool clearContext = true)
    {
        tString combinedContext;

        for (const auto &item : contextItems)
        {
            if (!combinedContext.empty())
                combinedContext << " | ";
            combinedContext << item;
        }

        if (clearContext)
            contextItems.clear();

        return combinedContext;
    }

    void ClearContext()
    {
        contextItems.clear();
    }

    struct ChatTrigger {
        tString trigger;
        std::vector<tString> responses;
        REAL extraDelay;
        bool exact;

        tString trimmedTrigger;
        bool isChatNameTrigger;
        tString triggerWithoutName;
        bool isSpecialTrigger;
        bool isExpedientTrigger;
    };

    std::vector<tString> contextItems;
    std::vector<ChatTrigger> chatTriggers;


    TempConfItemManager *chatBotCommandConfItems;

    eChatBotStats stats;
    ActionData    params;
    eChatBotFunctions *functions;
    eChatBotMessager *messager;
};


class eChatBot
{
private:
    eChatBot()
    {
        InitChatFunctions();
    }

    eChatBot(const eChatBot &) = delete;
    eChatBot &operator=(const eChatBot &) = delete;

public:

    eChatBotData data;

    // instance
    static eChatBot &getInstance()
    {
        static eChatBot instance;
        if (instance.data.functions->functionMap.empty())
            instance.InitChatFunctions();

        return instance;
    }

    static void SilencedAction();

    void InitChatFunctions();

    void LoadChatTriggers();
    void LoadChatCommandConfCommands();

    eChatBotStats &Stats()
    {
        return data.stats;
    }

    eChatBotFunctions *Functions()
    {
        return data.functions;
    }

    eChatBotMessager *Messager()
    {
        return data.messager;
    }

    void roundEndAnalyzeBanStatus()
    {
        int num_spectators = 0;

        for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
        {
            ePlayerNetID *p = se_PlayerNetIDs(i);
            if (p && p->IsHuman() && !p->isLocal() && p->IsSpectating())
            {
                num_spectators++;
            }
        }

        time_t now = time(nullptr);
        time_t difference = now - Stats().last_banned;

        bool bannedOverTodaysLimit = Stats().times_banned_today >= se_playerWatchAutoRandomNameBanLimit;
        bool timeToRevert = ((difference > se_playerWatchAutoRandomNameRevertTime && Stats().upTime() >= (se_playerWatchAutoRandomNameRevertTime/2)) || difference > se_playerWatchAutoRandomNameRevertTime*2 );
        bool noSpecators = num_spectators == 0;
        bool inGameForFullWaitTime = Stats().upTime() >= (se_playerWatchAutoRandomNameRevertTime);

        if(!bannedOverTodaysLimit && !avoidPlayerInGame() && timeToRevert && (noSpecators || inGameForFullWaitTime))
        {
            forceRandomRename = false;
        }
    }

    static bool InitiateAction(ePlayerNetID *triggeredBy, tString inputMessage, bool eventTrigger = false, tString preAppend = tString(""));

    static tString findResponse(eChatBot &bot, tString playerName, tString trigger, tString value, tString valDelim = tString("$val1"), bool send = false);
    static tString findResponsePlayer(eChatBot &bot, ePlayerNetID* triggeredBy, tString trigger, tString value, tString valDelim = tString("$val1"), bool send = false);

    bool ShouldAnalyze();
};

#include "tDatabase.h"
class eChatBotStatsDBAction : public tDatabase<eChatBotStats, TableDefinition<eChatBotStats>>
{
public:
    eChatBotStatsDBAction(sqlite3* db)
        : tDatabase<eChatBotStats, TableDefinition<eChatBotStats>>(
            db,
            "eChatBotStats",
            TableDefinition<eChatBotStats>(
                [] { // Primary key
                    TableDefinition<eChatBotStats>::PrimaryKey pk;
                    pk.columnName = "name";
                    pk.getValueFunc = [](const eChatBotStats &stats) -> std::string {
                        return stats.name.stdString();
                    };
                    return pk;
                }(),
                eChatBotStats::eChatBotStatsMappings
            )
        )
    {}

    eChatBotStats& getTargetObject(const tString &name) override
    {
        return eChatBot::getInstance().Stats();
    }

    std::vector<eChatBotStats> getAllObjects() override
    {
        return {eChatBot::getInstance().Stats()};
    }

    void postLoadActions(eChatBotStats& chatBotStats) override
    {
        chatBotStats.data_from_db = chatBotStats;
        time_t startOfDay = getStartTimeOfDay();

        if (chatBotStats.last_banned < startOfDay)
            chatBotStats.times_banned_today = 0;
    }
};




#endif
