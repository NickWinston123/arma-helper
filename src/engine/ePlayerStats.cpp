#include "ePlayerStats.h"
#include "ePlayer.h"

#include "sqlite3.h"
#include "sqlite3.h"
#include <sstream>
#include <iostream>
#include "../tron/gCycle.h"
#include "nNetwork.h"

bool se_playerStats = false;
static tConfItem<bool> se_playerStatsConf("PLAYER_STATS", se_playerStats);


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
    {"total_play_time", "REAL"}
};

void ePlayerStats::loadStatsFromDB()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_open("stats.db", &db);
    if (rc != SQLITE_OK)
    {
        con << "Cannot open database: " << tString(sqlite3_errmsg(db)) << "\n";
        return;
    }

    std::stringstream selectSql;
    selectSql << "SELECT name";
    for (const auto &field : fields) {
        selectSql << ", " << field.first;
    }
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
        PlayerData pd;
        int column = 1;
        pd.kills = sqlite3_column_int(stmt, column++);
        pd.deaths = sqlite3_column_int(stmt, column++);
        pd.match_wins = sqlite3_column_int(stmt, column++);
        pd.match_losses = sqlite3_column_int(stmt, column++);
        pd.round_wins = sqlite3_column_int(stmt, column++);
        pd.round_losses = sqlite3_column_int(stmt, column++);
        pd.rgb.r_ = sqlite3_column_int(stmt, column++);
        pd.rgb.g_ = sqlite3_column_int(stmt, column++);
        pd.rgb.b_ = sqlite3_column_int(stmt, column++);
        pd.total_messages = sqlite3_column_int(stmt, column++);
        pd.rounds_played = sqlite3_column_int(stmt, column++);
        pd.matches_played = sqlite3_column_int(stmt, column++);
        pd.total_play_time = sqlite3_column_double(stmt, column++);
        playerStatsMap[name] = pd;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void ePlayerStats::saveStatsToDB()
{
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    rc = sqlite3_open("stats.db", &db);
    if (rc != SQLITE_OK)
    {
        con << "Cannot open database: " << tString(sqlite3_errmsg(db)) << "\n";
        return;
    }

    std::stringstream createSql;
    createSql << "CREATE TABLE IF NOT EXISTS ePlayerStats(name TEXT PRIMARY KEY";
    for (const auto &field : fields) {
        createSql << ", " << field.first << " " << field.second;
    }
    createSql << ");";
    rc = sqlite3_exec(db, createSql.str().c_str(), 0, 0, &errMsg);

    if (rc != SQLITE_OK)
    {
        con << "SQL error: " << tString(errMsg) << "\n";
        sqlite3_free(errMsg);
    }
    else
    {
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
               << kv.second.rgb.r_ << ","
               << kv.second.rgb.g_ << ","
               << kv.second.rgb.b_ << ","
               << kv.second.total_messages << ","
               << kv.second.rounds_played  << ","
               << kv.second.matches_played << ","
               << kv.second.total_play_time
               << ");";
            rc = sqlite3_exec(db, ss.str().c_str(), 0, 0, &errMsg);

            if (rc != SQLITE_OK)
            {
                con << "SQL error: " << tString(errMsg) << "\n";
                sqlite3_free(errMsg);
            }
        }
    }

    sqlite3_close(db);
}

void ePlayerStats::updateMatchWinsAndLoss(ePlayerNetID *matchWinner)
{
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
    {
        ePlayerNetID *currentPlayer = se_PlayerNetIDs[i];

        if (currentPlayer->CurrentTeam())
        {
            if (currentPlayer == matchWinner)
            {
                addMatchWin(currentPlayer);
            }
            else
            {
                addMatchLoss(currentPlayer);
            }

            addMatchPlayed(currentPlayer);
        }
    }
}

void ePlayerStats::updateRoundWinsAndLoss()
{
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
    {
        ePlayerNetID *currentPlayer = se_PlayerNetIDs[i];

        if (currentPlayer->CurrentTeam())
        {
            gCycle* cycle = ePlayerNetID::NetPlayerToCycle(currentPlayer);

            if (roundWinnerProcessed)
            {
                if (cycle && cycle->Alive())
                {
                    addRoundWin(currentPlayer);
                }
                else
                {
                    addRoundLoss(currentPlayer);
                }
            }
            
            addPlayTime(currentPlayer);

            addRoundPlayed(currentPlayer);
        }
    }
}

void ePlayerStats::reloadStatsFromDB()
{
    playerStatsMap.clear(); // clear the current stats
    loadStatsFromDB();      // reload stats from DB
}

std::unordered_map<tString, PlayerData> ePlayerStats::playerStatsMap;
