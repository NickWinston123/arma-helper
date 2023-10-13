#ifndef ArmageTron_ePlayerStats_H
#define ArmageTron_ePlayerStats_H

#include "tDirectories.h"
#include "sqlite3.h"
#include "ePlayer.h"
#include <unordered_map>

#include "../tron/gHelper/gHelperUtilities.h"

#include "eTimer.h"

extern bool se_playerStats, se_playerStatsLog;

class PlayerDataBase
{
public:
    // Player
    int r,g,b;
    tString name;
    int total_messages   = 0;
    REAL fastest_speed   = 0;
    std::vector<std::string> chat_messages;
    REAL total_play_time = 0;
    REAL total_spec_time = 0;
    int times_joined     = 0;
    time_t last_seen     = 0;

    bool human = true;

    tString rgbString()
    {
        tString output;
        output  << "("
                << r
                << ", "
                << g
                << ", "
                << b
                << ")";
        return output;
    }

    // Cycle
    int kills = 0;
    int deaths = 0;
    int match_wins = 0;
    int match_losses = 0;
    int round_wins = 0;
    int round_losses = 0;
    int rounds_played = 0;
    int matches_played = 0;

    std::string getLastSeenAgo(bool in_game)
    {
        if (last_seen == 0)
            return "Never seen";

        time_t now = time(NULL);
        double seconds = in_game ? now : difftime(now, last_seen);
        double minutes = seconds / 60;
        double hours = minutes / 60;
        double days = hours / 24;

        if (seconds < 60)
            return std::to_string((int)seconds) + " seconds ago";
        else if (minutes < 60)
            return std::to_string((int)minutes) + " minutes ago";
        else if (hours < 24)
            return std::to_string((int)hours) + " hours ago";
        else
            return std::to_string((int)days) + " days ago";
    }

    REAL getTotalPlayTime(bool add = true)
    {
        return total_play_time + (add ? se_GameTime() : 0);
    }

    REAL getTotalSpecTime(bool add = true)
    {
        return total_spec_time + (add ? se_GameTime() : 0);
    }

        std::string getChatMessages()
        {
            std::string messages;
            bool first = true;

            for (auto messageIt = chat_messages.rbegin(); messageIt != chat_messages.rend(); ++messageIt)
            {
                if (!first)
                    messages += ", ";
                else
                    first = false;

                messages += *messageIt;
            }

            return messages;
        }

    double getKDRatio() const
    {
        if (deaths == 0)
        {
            return kills;
        }
        else if (kills >= deaths)
        {
            return static_cast<double>(kills) / deaths;
        }
        else
        {
            return -static_cast<double>(deaths) / kills;
        }
    }
};


class PlayerData : public PlayerDataBase
{
    using StatFunction = std::function<tString(PlayerDataBase *)>;
public:
    static std::map<std::string, StatFunction> valueMap;
    static std::set<std::string> valueMapdisplayFields;

    static tString getAvailableStatsStr(std::string source = "")
    {
        std::set<std::string> displayFields = PlayerData::valueMapdisplayFields;

        if (source == "leaderboardFunc")
        {
            // remove: rgb, chat_messages
            displayFields.erase("rgb");
            displayFields.erase("chat_messages");
        }

        tString result;
        result << "Available stats are: ";
        int count = 0;
        for (const auto &name : displayFields)
        {
            result << name;
            if (++count < displayFields.size())
                result << ", ";
        }
        return result;
    }


    tString getAnyValue(tString variable)
    {
        auto stat = valueMap.find(variable.stdString());
        if (stat != valueMap.end())
            return stat->second(this);
        else
        {
            tString emptyVal;
            return emptyVal;
        }
    }

    PlayerDataBase data_from_db;
};
#include <algorithm>

class ePlayerStats
{
public:

    static PlayerData& getStats(ePlayerNetID * player)
    {
        return playerStatsMap[player->GetRealName().ToLower()];
    }

    static PlayerData& getStats(tString playerName)
    {
        return playerStatsMap[playerName.ToLower()];
    }

    static PlayerData& getStatsForAnalysis(tString playerName)
    {

        std::string playerNameLower = playerName.stdString();
        std::transform(playerNameLower.begin(), playerNameLower.end(), playerNameLower.begin(), ::tolower);

        // exact case-insensitive match
        for (const auto &kv : playerStatsMap)
        {
            std::string currentNameLower = kv.first.stdString();
            std::transform(currentNameLower.begin(), currentNameLower.end(), currentNameLower.begin(), ::tolower);

            if (currentNameLower == playerNameLower)
            {
                return playerStatsMap[kv.first];
            }
        }

        // no exact match, look for the closest match (substring search)
        for (const auto &kv : playerStatsMap)
        {
            std::string currentNameLower = kv.first.stdString();
            std::transform(currentNameLower.begin(), currentNameLower.end(), currentNameLower.begin(), ::tolower);

            if (currentNameLower.find(playerNameLower) != std::string::npos)
            {
                return playerStatsMap[kv.first];
            }
        }

        // default, null vals
        return playerStatsMap[playerName.ToLower()];
    }

    static void addKill(ePlayerNetID *player)
    {
        playerStatsMap[player->GetRealName().ToLower()].kills++;
    }

    static void addDeath(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].deaths++;
    }

    static void addMatchWin(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].match_wins++;
    }

    static void addMatchLoss(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].match_losses++;
    }

    static void addMatchPlayed(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].matches_played++;
    }

    static void addRoundWin(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].round_wins++;
    }

    static void addRoundLoss(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].round_losses++;
    }

    static void addRoundPlayed(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].rounds_played++;
    }

    static void addPlayTime(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].total_play_time += se_GameTime();
    }

    static void addSpecTime(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].total_spec_time += se_GameTime();
    }

    static void addJoined(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName().ToLower()].times_joined++;
    }

    static void setColor(ePlayerNetID * player, int r, int g, int b)
    {
        tString name = player->GetRealName().ToLower();

        PlayerData &data = playerStatsMap[name];
        data.r = r;
        data.g = g;
        data.b = b;
    }

    static void playerJoined(ePlayerNetID * player)
    {
        PlayerData &stats = getStats(player);
        stats.times_joined++;
        stats.last_seen = time(NULL);
        stats.human = player->IsHuman();
    }

    static void playerLeft(ePlayerNetID * player)
    {
        PlayerData &stats = getStats(player);
        stats.last_seen = time(NULL);
    }

    static void addMessage(ePlayerNetID * player, tString message)
    {
        PlayerData &stats = getStats(player);

        stats.chat_messages.push_back(message.stdString());
        stats.total_messages++;
    }

    static void updateMatchWinsAndLoss(ePlayerNetID *matchWinner);
    static void updateRoundWinsAndLoss();

    static void loadStatsFromDB();
    static void saveStatsToDB();
    static void reloadStatsFromDB();


    static REAL getTotalPlayersLogged()
    {
        return static_cast<REAL>(playerStatsMap.size());
    }

    static std::unordered_map<tString, PlayerData> playerStatsMap;
};


struct ColumnMapping {
    std::string columnName;
    std::string columnType;
    std::function<void(sqlite3_stmt*, int&, const PlayerData&)> bindFunc;
    std::function<void(sqlite3_stmt*, int&, PlayerData&)> extractFunc;
};

template <typename T>
void ensureTableAndColumnsExist(sqlite3 *db, const std::string& tableName, const std::vector<T>& columnMappings) 
{
    char *errMsg = 0;
    int rc;

    std::stringstream createSql;
    createSql << "CREATE TABLE IF NOT EXISTS " << tableName << "(";
    bool first = true;
    for (const auto &mapping : columnMappings)
    {
        if (!first)
        {
            createSql << ", ";
        }
        createSql << mapping.columnName << " " << mapping.columnType;
        first = false;
    }
    createSql << ");";

    rc = sqlite3_exec(db, createSql.str().c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::string debug = "SQL error during table/column existence check: " + std::string(errMsg) + "\n";
        gHelperUtility::DebugLog(debug);
        sqlite3_free(errMsg);
    }
    else
    {
        gHelperUtility::DebugLog("Table and columns ensured to exist successfully.\n");
    }
}


#endif
