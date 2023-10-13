#include "tDatabase.h"
#include "ePlayerStats.h"
#include "ePlayer.h"
#include "eChatbot.h"

#include "sqlite3.h"

#include <numeric>
#include <sstream>
#include <iostream>

#include "../tron/gCycle.h"
#include "../tron/gHelper/gHelperUtilities.h"
#include "nNetwork.h"
#include "tDirectories.h"

bool se_playerStats = false;
static tConfItem<bool> se_playerStatsConf("PLAYER_STATS", se_playerStats);

bool se_playerStatsLog = false;
static tConfItem<bool> se_playerStatsLogConf("PLAYER_STATS_LOG", se_playerStatsLog);

tString se_playerStatsDataBaseFile("stats.db");
static tConfItem<tString> se_playerStatsDataBaseFileConf("PLAYER_STATS_DB_FILE", se_playerStatsDataBaseFile);

std::string join(const std::vector<std::string> &vec, const std::string &delimiter);

void ePlayerStats::loadStatsFromDB() 
{
    sqlite3 *db = tDBUtility::getDatabase(se_playerStatsDataBaseFile);
    if (!db) return;

    ePlayerStatsDBAction  playerDataAction(db);
    playerDataAction.loadStatsFromDB();

    eChatBotStatsDBAction chatbotStatsAction(db);
    chatbotStatsAction.loadStatsFromDB();

    sqlite3_close(db);
}

void ePlayerStats::saveStatsToDB() 
{
    sqlite3 *db = tDBUtility::getDatabase(se_playerStatsDataBaseFile);
    if (!db) return;

    ePlayerStatsDBAction  dbAction(db);
    dbAction.saveStatsToDB();

    eChatBotStatsDBAction chatbotStatsAction(db);
    chatbotStatsAction.saveStatsToDB();

    sqlite3_close(db);
}

void ePlayerStats::updateMatchWinsAndLoss(ePlayerNetID *matchWinner)
{
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
    {
        ePlayerNetID *currentPlayer = se_PlayerNetIDs[i];
        PlayerData &stats = getStats(currentPlayer);

        if (currentPlayer->CurrentTeam())
        {
            if (currentPlayer == matchWinner)
                stats.match_wins++;
            else
                stats.match_losses++;

            stats.matches_played++;
        }
    }
}

void ePlayerStats::updateRoundWinsAndLoss()
{
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
    {
        ePlayerNetID *currentPlayer = se_PlayerNetIDs[i];
        PlayerData &stats = getStats(currentPlayer);

        if (currentPlayer->CurrentTeam())
        {
            gCycle *cycle = ePlayerNetID::NetPlayerToCycle(currentPlayer);

            if (roundWinnerProcessed)
            {
                if (cycle && cycle->Alive())
                    stats.round_wins++;
                else
                    stats.round_losses++;
            }

            stats.total_play_time += se_GameTime();
            stats.rounds_played++;
        }
        else
        {
            stats.total_spec_time += se_GameTime();
        }
    }
}

void ePlayerStats::reloadStatsFromDB()
{
    playerStatsMap.clear(); // clear the current stats
    loadStatsFromDB();      // reload stats from DB
}

std::unordered_map<tString, PlayerData> ePlayerStats::playerStatsMap;

std::set<std::string> PlayerData::valueMapdisplayFields = 
{
    "rgb", "chats", "kills", "deaths", "match_wins",
    "match_losses", "round_wins", "round_losses", "rounds_played",
    "matches_played", "play_time", "spec_time", "times_joined",
    "kd", "chat_messages", "fastest"
};

std::map<std::string, PlayerData::StatFunction> PlayerData::valueMap = {
    {"rgb", [](PlayerDataBase *self)
     {
         return self->rgbString();
     }},
    {"r", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->r;
         return result;
     }},
    {"g", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->g;
         return result;
     }},
    {"b", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->b;
         return result;
     }},
    {"chats", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->total_messages;
         return result;
     }},
    {"kills", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->kills;
         return result;
     }},
    {"deaths", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->deaths;
         return result;
     }},
    {"match_wins", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->match_wins;
         return result;
     }},
    {"match_losses", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->match_losses;
         return result;
     }},
    {"round_wins", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->round_wins;
         return result;
     }},
    {"round_losses", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->round_losses;
         return result;
     }},
    {"rounds_played", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->rounds_played;
         return result;
     }},
    {"matches_played", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->matches_played;
         return result;
     }},
    {"play_time", [](PlayerDataBase *self)
     {
         tString result("");
         result << st_GetFormatTime(self->getTotalPlayTime(), false);
         return result;
     }},
    {"spec_time", [](PlayerDataBase *self)
     {
         tString result("");
         result << st_GetFormatTime(self->getTotalSpecTime(), false);
         return result;
     }},
    {"times_joined", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->times_joined;
         return result;
     }},
    {"kd", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->getKDRatio();
         return result;
     }},
    {"chat_messages", [](PlayerDataBase *self)
     {
         tString result("");
         result = tString(self->getChatMessages());
         return result;
     }},
    {"fastest", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->fastest_speed;
         return result;
     }},
    {"fastest_speed", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->fastest_speed;
         return result;
     }},
    {"speed", [](PlayerDataBase *self)
     {
         tString result("");
         result << self->fastest_speed;
         return result;
     }}

};



const std::string DELIMITER = "-+HACKERMANS+-";

std::string join(const std::vector<std::string> &vec, const std::string &delimiter)
{
    return std::accumulate(std::begin(vec), std::end(vec), std::string(),
                           [&](const std::string &ss, const std::string &s)
                           {
                               return ss.empty() ? s : ss + delimiter + s;
                           });
}


std::vector<std::string> deserializeVector(const std::string &str)
{
    std::vector<std::string> vec;

    if (str.find(DELIMITER) == std::string::npos)
    {
        vec.push_back(str);
        return vec;
    }

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(DELIMITER, prev)) != std::string::npos)
    {
        vec.push_back(str.substr(prev, pos - prev));
        prev = pos + DELIMITER.length();
    }
    vec.push_back(str.substr(prev));
    return vec;
}

std::string serializeVector(const std::vector<std::string> &vec)
{
    return std::accumulate(vec.begin(), vec.end(), std::string(),
                           [](const std::string &a, const std::string &b)
                           {
                               return a + (a.length() > 0 ? DELIMITER : "") + b;
                           });
}