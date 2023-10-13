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

const std::vector<std::pair<std::string, std::string>> fields = {
    {"matches_played", "INTEGER"},
    {"rounds_played", "INTEGER"},
    {"total_messages", "INTEGER"},
    {"b", "INTEGER"},
    {"g", "INTEGER"},
    {"r", "INTEGER"},
    {"round_losses", "INTEGER"},
    {"times_joined", "INTEGER"},
    {"round_wins", "INTEGER"},
    {"total_spec_time", "REAL"},
    {"total_play_time", "REAL"},
    {"match_losses", "INTEGER"},
    {"deaths", "INTEGER"},
    {"match_wins", "INTEGER"},
    {"kills", "INTEGER"},
    {"chat_messages", "TEXT"},
    {"fastest_speed", "REAL"},
};

const std::string DELIMITER = "-+HACKERMANS+-";

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

void ePlayerStats::ensureTableAndColumnsExist(sqlite3 *db)
{
    char *errMsg = 0;
    int rc;

    char *tableCheckQuery = "SELECT name FROM sqlite_master WHERE type='table' AND name='ePlayerStats';";
    rc = sqlite3_exec(db, tableCheckQuery, 0, 0, &errMsg);

    if (rc == SQLITE_OK)
    {
        for (const auto &field : fields)
        {
            std::stringstream columnCheckSql;
            columnCheckSql << "PRAGMA table_info(ePlayerStats);";

            sqlite3_stmt *stmt;
            rc = sqlite3_prepare_v2(db, columnCheckSql.str().c_str(), -1, &stmt, 0);

            bool columnExists = false;
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                if (columnName == field.first)
                {
                    columnExists = true;
                    break;
                }
            }
            sqlite3_finalize(stmt);

            if (!columnExists)
            {
                std::stringstream addColumnSql;
                addColumnSql << "ALTER TABLE ePlayerStats ADD COLUMN " << field.first << " " << field.second << ";";
                rc = sqlite3_exec(db, addColumnSql.str().c_str(), 0, 0, &errMsg);
                if (rc != SQLITE_OK)
                {
                    std::string debug = std::string("SQL error: ") + (errMsg) + "\n";
                    gHelperUtility::DebugLog(debug);
                    sqlite3_free(errMsg);
                }
            }
        }
    }
    else
    {
        std::stringstream createSql;
        createSql << "CREATE TABLE ePlayerStats(name TEXT PRIMARY KEY";
        for (const auto &field : fields)
        {
            createSql << ", " << field.first << " " << field.second;
        }
        createSql << ");";
        rc = sqlite3_exec(db, createSql.str().c_str(), 0, 0, &errMsg);

        if (rc != SQLITE_OK)
        {
            std::string debug = std::string("SQL error: ") + (errMsg) + "\n";
            if (se_playerStatsLog)
                gHelperUtility::DebugLog(debug);
            sqlite3_free(errMsg);
        }
    }
}

void ePlayerStats::loadStatsFromDB()
{
    if (se_playerStatsLog)
        gHelperUtility::DebugLog("loadStatsFromDB");
    std::string debug = st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ").stdString() + "Starting loadStatsFromDB()\n";

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_open(tDirectories::Var().GetReadPath("stats.db"), &db);
    if (rc != SQLITE_OK)
    {
        debug += std::string("Cannot open database: ") + sqlite3_errmsg(db) + "\n";
        if (se_playerStatsLog)
            gHelperUtility::DebugLog(debug);
        return;
    }

    ensureTableAndColumnsExist(db);
    ;

    std::stringstream selectSql;
    selectSql << "SELECT name";
    for (const auto &field : fields)
        selectSql << ", " << field.first;

    selectSql << " FROM ePlayerStats;";
    debug += "USING QUERY: " + selectSql.str() + "\n";
    if (se_playerStatsLog)
        gHelperUtility::DebugLog(debug);
    rc = sqlite3_prepare_v2(db, selectSql.str().c_str(), -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        debug += std::string("Failed to fetch data: ") + sqlite3_errmsg(db) + "\n";
        if (se_playerStatsLog)
            gHelperUtility::DebugLog(debug);
        sqlite3_close(db);
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        tString name = tString((char *)sqlite3_column_text(stmt, 0));
        PlayerData &stats = getStats(name);
        int column = 1;

        stats.matches_played = sqlite3_column_int(stmt, column++);
        stats.rounds_played = sqlite3_column_int(stmt, column++);
        stats.total_messages = sqlite3_column_int(stmt, column++);
        stats.b = sqlite3_column_int(stmt, column++);
        stats.g = sqlite3_column_int(stmt, column++);
        stats.r = sqlite3_column_int(stmt, column++);
        stats.round_losses = sqlite3_column_int(stmt, column++);
        stats.times_joined = sqlite3_column_int(stmt, column++);
        stats.round_wins = sqlite3_column_int(stmt, column++);
        stats.total_spec_time = sqlite3_column_double(stmt, column++);
        stats.total_play_time = sqlite3_column_double(stmt, column++);
        stats.match_losses = sqlite3_column_int(stmt, column++);
        stats.deaths = sqlite3_column_int(stmt, column++);
        stats.match_wins = sqlite3_column_int(stmt, column++);
        stats.kills = sqlite3_column_int(stmt, column++);

        const char *chatSerialized = reinterpret_cast<const char *>(sqlite3_column_text(stmt, column++));
        if (chatSerialized)
            stats.chatMessages = deserializeVector(chatSerialized);

        stats.fastest_speed = sqlite3_column_double(stmt, column++);
        stats.data_from_db = stats;
    }

    eChatBotStats::ensureChatBotStatsTableAndColumnsExist(db);
    eChatBotStats::loadChatBotStatsFromDB(db);

    if (se_playerStatsLog)
        gHelperUtility::DebugLog(debug);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

std::string join(const std::vector<std::string> &vec, const std::string &delimiter)
{
    return std::accumulate(std::begin(vec), std::end(vec), std::string(),
                           [&](const std::string &ss, const std::string &s)
                           {
                               return ss.empty() ? s : ss + delimiter + s;
                           });
}

void ePlayerStats::saveStatsToDB()
{
    if (se_playerStatsLog)
        gHelperUtility::DebugLog("saveStatsToDB");
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    std::string debug = st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ").stdString() + "Starting saveStatsToDB()\n";

    rc = sqlite3_open(tDirectories::Var().GetReadPath("stats.db"), &db);
    if (rc != SQLITE_OK)
    {
        std::string debug = std::string("Cannot open database: ") + (sqlite3_errmsg(db)) + "\n";
        if (se_playerStatsLog)
            gHelperUtility::DebugLog(debug);
        return;
    }

    ensureTableAndColumnsExist(db);

    debug += "Saving player stats.\n";
    for (const auto &kv : playerStatsMap)
    {
        std::stringstream ss;
        std::vector<std::string> placeholders;
        for (size_t i = 0; i < fields.size() + 1; i++)
        {
            placeholders.push_back("?");
        }
        ss << "INSERT OR REPLACE INTO ePlayerStats(name";
        for (const auto &field : fields)
        {
            ss << ", " << field.first;
        }
        ss << ") VALUES(" << join(placeholders, ",") << ");";

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            std::string debug = std::string("Failed to prepare statement: ") + (sqlite3_errmsg(db)) + "\n";
            if (se_playerStatsLog)
                gHelperUtility::DebugLog(debug);
            continue;
        }

        sqlite3_bind_text(stmt, 1, kv.first.c_str(), -1, SQLITE_STATIC);
        int col = 2;

        sqlite3_bind_int(stmt, col++, kv.second.matches_played);
        sqlite3_bind_int(stmt, col++, kv.second.rounds_played);
        sqlite3_bind_int(stmt, col++, kv.second.total_messages);
        sqlite3_bind_int(stmt, col++, kv.second.b);
        sqlite3_bind_int(stmt, col++, kv.second.g);
        sqlite3_bind_int(stmt, col++, kv.second.r);
        sqlite3_bind_int(stmt, col++, kv.second.round_losses);
        sqlite3_bind_int(stmt, col++, kv.second.times_joined);
        sqlite3_bind_int(stmt, col++, kv.second.round_wins);
        sqlite3_bind_double(stmt, col++, kv.second.total_spec_time);
        sqlite3_bind_double(stmt, col++, kv.second.total_play_time);
        sqlite3_bind_int(stmt, col++, kv.second.match_losses);
        sqlite3_bind_int(stmt, col++, kv.second.deaths);
        sqlite3_bind_int(stmt, col++, kv.second.match_wins);
        sqlite3_bind_int(stmt, col++, kv.second.kills);
        sqlite3_bind_text(stmt, col++, serializeVector(kv.second.chatMessages).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, col++, kv.second.fastest_speed);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            std::string debug = std::string("SQL error while inserting: ") + (sqlite3_errmsg(db)) + "\n";
            if (se_playerStatsLog)
                gHelperUtility::DebugLog(debug);
        }

        sqlite3_finalize(stmt);
    }

    eChatBotStats::ensureChatBotStatsTableAndColumnsExist(db);
    eChatBotStats::saveChatBotStatsToDB(db);

    if (se_playerStatsLog)
        gHelperUtility::DebugLog(debug);

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

std::set<std::string> PlayerData::valueMapdisplayFields = {
    "rgb", "chats", "kills", "deaths", "match_wins",
    "match_losses", "round_wins", "round_losses", "rounds_played",
    "matches_played", "play_time", "spec_time", "times_joined",
    "kd", "chats", "fastest"};

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
    {"chats", [](PlayerDataBase *self)
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
     }}

};
