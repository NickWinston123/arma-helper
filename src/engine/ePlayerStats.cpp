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

std::vector<std::string> deserializeVector(const std::string &str);
std::string serializeVector(const std::vector<std::string> &vec);
std::string join(const std::vector<std::string> &vec, const std::string &delimiter);

const std::vector<ColumnMapping> ePlayerStatsMappings = {
    {"name", "TEXT PRIMARY KEY",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_text(stmt, col++, stats.name.c_str(), -1, SQLITE_STATIC); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { col++; }
    },

    {"matches_played", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.matches_played); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.matches_played = sqlite3_column_int(stmt, col++); }
    },

    {"rounds_played", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.rounds_played); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.rounds_played = sqlite3_column_int(stmt, col++); }
    },

    {"total_messages", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.total_messages); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.total_messages = sqlite3_column_int(stmt, col++); }
    },

    {"b", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.b); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.b = sqlite3_column_int(stmt, col++); }
    },

    {"g", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.g); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.g = sqlite3_column_int(stmt, col++); }
    },

    {"r", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.r); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.r = sqlite3_column_int(stmt, col++); }
    },

    {"round_losses", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.round_losses); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.round_losses = sqlite3_column_int(stmt, col++); }
    },

    {"round_wins", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.round_wins); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.round_wins = sqlite3_column_int(stmt, col++); }
    },

    {"total_play_time", "REAL",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_double(stmt, col++, stats.total_play_time); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.total_play_time = sqlite3_column_double(stmt, col++); }
    },

    {"match_losses", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.match_losses); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.match_losses = sqlite3_column_int(stmt, col++); }
    },

    {"deaths", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.deaths); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.deaths = sqlite3_column_int(stmt, col++); }
    },

    {"match_wins", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.match_wins); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.match_wins = sqlite3_column_int(stmt, col++); }
    },

    {"kills", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.kills); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.kills = sqlite3_column_int(stmt, col++);  }
    },

    {"times_joined", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.times_joined); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.times_joined = sqlite3_column_int(stmt, col++); }
    },

    {"total_spec_time", "REAL",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_double(stmt, col++, stats.total_spec_time); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.total_spec_time = sqlite3_column_double(stmt, col++); }
    },

    {"chat_messages", "TEXT",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_text(stmt, col++, serializeVector(stats.chat_messages).c_str(), -1, SQLITE_STATIC); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) {
            const char *chatSerialized = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col++));
            if (chatSerialized)
                stats.chat_messages = deserializeVector(chatSerialized);
        }
    },

    {"fastest_speed", "REAL",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_double(stmt, col++, stats.fastest_speed); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.fastest_speed = sqlite3_column_double(stmt, col++); }
    },

    {"last_seen", "BIGINT",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int64(stmt, col++, static_cast<sqlite3_int64>(stats.last_seen)); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.last_seen = static_cast<time_t>(sqlite3_column_int64(stmt, col++)); }
    },

    {"human", "BOOLEAN",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.human ? 1 : 0); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.human = sqlite3_column_int(stmt, col++) != 0; }
    },

};

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
    ensureTableAndColumnsExist(db, "ePlayerStats", ePlayerStatsMappings);

    std::stringstream selectSql;
    selectSql << "SELECT ";
    bool isFirst = true;
    for (const auto &mapping : ePlayerStatsMappings)
    {
        if (!isFirst)
            selectSql << ", ";
        else
            isFirst = false;

        selectSql << mapping.columnName;
    }
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
        int column = 0;
        for (const auto &mapping : ePlayerStatsMappings)
            mapping.extractFunc(stmt, column, stats);

        stats.name = name;
        stats.data_from_db = stats;
    }

    if (rc != SQLITE_DONE)
    {
        std::string errorMsg = "sqlite3_step failed: " + std::string(sqlite3_errmsg(db));
        gHelperUtility::DebugLog(errorMsg.c_str());
    }

    eChatBotStats::loadChatBotStatsFromDB(db);

    if (se_playerStatsLog)
        gHelperUtility::DebugLog(debug);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
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
        debug += "Cannot open database: " + std::string(sqlite3_errmsg(db)) + "\n";
        if (se_playerStatsLog)
            gHelperUtility::DebugLog(debug);
        return;
    }

    ensureTableAndColumnsExist(db, "ePlayerStats", ePlayerStatsMappings);

    if (playerStatsMap.empty()) 
        debug += "playerStatsMap is empty. No data to write to the database.\n";
    else
        debug += "Saving player stats. Total players: " + std::to_string(playerStatsMap.size()) + "\n";

    debug += "Saving player stats.\n";
    for (const auto &kv : playerStatsMap)
    {
        std::stringstream ss;
        std::vector<std::string> placeholders;

        for (size_t i = 0; i < ePlayerStatsMappings.size(); i++)
        {
            placeholders.push_back("?");
        }

        ss << "INSERT OR REPLACE INTO ePlayerStats(";
        bool isFirst = true;
        for (const auto &mapping : ePlayerStatsMappings)
        {
            if (!isFirst)
                ss << ", ";
            else
                isFirst = false;

            ss << mapping.columnName;
        }
        ss << ") VALUES(" << join(placeholders, ",") << ");";

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, ss.str().c_str(), -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            debug = std::string("Failed to prepare statement: ") + sqlite3_errmsg(db) + "\n";
            if (se_playerStatsLog)
                gHelperUtility::DebugLog(debug);
            return;
        }

        int column = 1;
        for (const auto& mapping : ePlayerStatsMappings)
            mapping.bindFunc(stmt, column, kv.second);


        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            debug = std::string("ePlayerStats SQL error while inserting: ") + sqlite3_errmsg(db) + "\n";
            if (se_playerStatsLog)
                gHelperUtility::DebugLog(debug);
        }

        sqlite3_finalize(stmt);
    }

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

std::string join(const std::vector<std::string> &vec, const std::string &delimiter)
{
    return std::accumulate(std::begin(vec), std::end(vec), std::string(),
                           [&](const std::string &ss, const std::string &s)
                           {
                               return ss.empty() ? s : ss + delimiter + s;
                           });
}
