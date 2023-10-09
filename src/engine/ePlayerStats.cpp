#include "ePlayerStats.h"
#include "ePlayer.h"

#include "sqlite3.h"
#include "sqlite3.h"
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


const std::unordered_map<std::string, std::string> fields = {
    {"kills", "INTEGER"},
    {"deaths", "INTEGER"},
    {"match_wins", "INTEGER"},
    {"match_losses", "INTEGER"},
    {"round_wins", "INTEGER"},
    {"round_losses", "INTEGER"},
    {"r", "INTEGER"},
    {"g", "INTEGER"},
    {"b", "INTEGER"},
    {"total_messages", "INTEGER"},
    {"rounds_played", "INTEGER"},
    {"matches_played", "INTEGER"},
    {"total_play_time", "REAL"},
    {"total_spec_time", "REAL"},
    {"times_joined", "INTEGER"},
};

void ePlayerStats::loadStatsFromDB()
{

    std::string debug = st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ").stdString() + "Starting loadStatsFromDB()\n";

    if (se_playerStatsLog)
        gHelperUtility::DebugLog(debug);

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_open(tDirectories::Var().GetReadPath("stats.db"), &db);
    if (rc != SQLITE_OK)
    {
        con << "Cannot open database: " << tString(sqlite3_errmsg(db)) << "\n";
        return;
    }

    std::stringstream selectSql;
    selectSql << "SELECT name";
    for (const auto &field : fields)
        selectSql << ", " << field.first;

    selectSql << " FROM ePlayerStats;";
    rc = sqlite3_prepare_v2(db, selectSql.str().c_str(), -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        con << "Failed to fetch data: " << tString(sqlite3_errmsg(db)) << "\n";
        sqlite3_close(db);
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        tString name = tString((char *)sqlite3_column_text(stmt, 0));
        PlayerData & stats = getStats(name);
        int column = 1;
        stats.kills = sqlite3_column_int(stmt, column++);
        stats.deaths = sqlite3_column_int(stmt, column++);
        stats.match_wins = sqlite3_column_int(stmt, column++);
        stats.match_losses = sqlite3_column_int(stmt, column++);
        stats.round_wins = sqlite3_column_int(stmt, column++);
        stats.round_losses = sqlite3_column_int(stmt, column++);
        stats.r = sqlite3_column_int(stmt, column++);
        stats.g = sqlite3_column_int(stmt, column++);
        stats.b = sqlite3_column_int(stmt, column++);
        stats.total_messages = sqlite3_column_int(stmt, column++);
        stats.rounds_played = sqlite3_column_int(stmt, column++);
        stats.matches_played = sqlite3_column_int(stmt, column++);
        stats.total_play_time = sqlite3_column_double(stmt, column++);
        stats.total_spec_time = sqlite3_column_double(stmt, column++);
        stats.times_joined = sqlite3_column_int(stmt, column++);
        stats.data_from_db = stats;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void ePlayerStats::saveStatsToDB()
{
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    std::string debug = st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ").stdString() + "Starting saveStatsToDB()\n";

    rc = sqlite3_open(tDirectories::Var().GetReadPath("stats.db"), &db);
    if (rc != SQLITE_OK)
    {
        con << "Cannot open database: " << tString(sqlite3_errmsg(db)) << "\n";
        return;
    }


    char *tableCheckQuery = "SELECT name FROM sqlite_master WHERE type='table' AND name='ePlayerStats';";
    rc = sqlite3_exec(db, tableCheckQuery, 0, 0, &errMsg);

    if (rc == SQLITE_OK)
    {
        debug += "TABLE EXISTS.\n";

        for (const auto &field : fields)
        {
            debug += "Checking field: " + field.first + "\n";

            std::stringstream columnCheckSql;
            columnCheckSql << "PRAGMA table_info(ePlayerStats);";

            sqlite3_stmt *stmt;
            rc = sqlite3_prepare_v2(db, columnCheckSql.str().c_str(), -1, &stmt, 0);

            bool columnExists = false;
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                std::string columnName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                if (columnName == field.first)
                {
                    debug += "Column " + columnName + " exists.\n";
                    columnExists = true;
                    break;
                }
            }
            sqlite3_finalize(stmt);

            if (!columnExists)
            {
                debug += "Column " + field.first + " does not exist. Adding it.\n";

                std::stringstream addColumnSql;
                addColumnSql << "ALTER TABLE ePlayerStats ADD COLUMN " << field.first << " " << field.second << ";";
                rc = sqlite3_exec(db, addColumnSql.str().c_str(), 0, 0, &errMsg);
                if (rc != SQLITE_OK)
                {
                    con << "SQL error: " << tString(errMsg) << "\n";
                    sqlite3_free(errMsg);
                }
            }
        }
    }
    else
    {
        debug += "TABLE DOES NOT EXIST. Creating it.\n";

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
            con << "SQL error: " << tString(errMsg) << "\n";
            sqlite3_free(errMsg);
        }
    }

    if (rc != SQLITE_OK)
    {
        con << "SQL error: " << tString(errMsg) << "\n";
        sqlite3_free(errMsg);
    }
    else
    {
        debug += "Saving player stats.\n";
        for (const auto &kv : playerStatsMap)
        {
            std::stringstream ss;
            ss << "INSERT OR REPLACE INTO ePlayerStats(name";
            for (const auto &field : fields) {
                ss << ", " << field.first;
            }
            ss << ") VALUES('" << kv.first << "',"
               << kv.second.kills << ","
               << kv.second.deaths << ","
               << kv.second.match_wins << ","
               << kv.second.match_losses << ","
               << kv.second.round_wins << ","
               << kv.second.round_losses << ","
               << kv.second.r << ","
               << kv.second.g << ","
               << kv.second.b << ","
               << kv.second.total_messages << ","
               << kv.second.rounds_played  << ","
               << kv.second.matches_played << ","
               << kv.second.total_play_time << ","
               << kv.second.total_spec_time << ","
               << kv.second.times_joined
               << ");";
            rc = sqlite3_exec(db, ss.str().c_str(), 0, 0, &errMsg);

            if (rc != SQLITE_OK)
            {
                con << "SQL error: " << tString(errMsg) << "\n";
                sqlite3_free(errMsg);
            }
        }
    }

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
            gCycle* cycle = ePlayerNetID::NetPlayerToCycle(currentPlayer);

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
     }}};
