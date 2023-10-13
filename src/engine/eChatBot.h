#ifndef ArmageTron_eChatBot_H
#define ArmageTron_eChatBot_H

#include "tString.h"
#include "tSysTime.h"

#include <unordered_map>
#include <functional>
#include "defs.h"

#include "ePlayer.h"
#include "sqlite3.h"

extern bool se_playerTriggerMessagesReactToSelf;
extern tString se_playerTriggerMessagesFile;
extern tString se_playerTriggerMessagesIgnoreList;

extern bool se_playerTriggerMessages;

struct eChatBotStats
{
    int total_messages_read = 0;
    int total_messages_sent = 0;
    REAL total_up_time      = 0;
    tString lastMatchedTrigger;
    ePlayerNetID *lastTriggeredBy;

    static void loadChatBotStatsFromDB(sqlite3* db);
    static void saveChatBotStatsToDB(sqlite3* db);
    static void ensureChatBotStatsTableAndColumnsExist(sqlite3* db);

    REAL UpTime()
    {
        return total_up_time + tSysTimeFloat();
    }
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
    typedef tString (*ChatFunction)(tString);

    std::map<tString, std::tuple<std::vector<tString>, REAL, bool>> chatTriggers;
    std::vector<tString> chatTriggerKeys;

    eChatBotStats stats;

    // instance
    static eChatBot &getInstance()
    {
        static eChatBot instance;
        if (instance.functionMap.empty())
            instance.InitChatFunctions();

        return instance;
    }

    void InitChatFunctions();

    void LoadChatTriggers();

    std::map<tString, ChatFunction> functionMap;
    void RegisterFunction(const tString &name, ChatFunction func)
    {
        functionMap[name] = func;
    }

    tString ExecuteFunction(const tString &name, const tString &message)
    {
        if (functionMap.find(name) != functionMap.end())
        {
            return functionMap[name](message);
        }
        return tString("");
    }

    eChatBotStats& Stats()
    {
        return stats;
    }

    bool masterFuncResponse = false;
    std::tuple<tString, REAL, ePlayerNetID *> findTriggeredResponse(ePlayerNetID *triggeredByPlayer, tString chatMessage, bool eventTrigger);
    static void InitiateAction(ePlayerNetID *triggeredByPlayer, tString message, bool eventTrigger = false, tString preAppend = tString(""));
    void preparePlayerMessage(tString messageToSend, REAL extraDelay, ePlayerNetID *player);
    REAL determineReadingDelay(tString message);
    static void scheduleMessageTask(ePlayerNetID *netPlayer, tString message, bool chatFlag, REAL totalDelay, REAL flagDelay);
    REAL calculateResponseSmartDelay(tString response, REAL wpm);
    bool ShouldAnalyze();
};


#endif
