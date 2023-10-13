#ifndef ArmageTron_ePlayerStats_H
#define ArmageTron_ePlayerStats_H

#include "tDirectories.h"
#include "sqlite3.h"
#include "ePlayer.h"
#include <unordered_map>
#include <numeric>


#include "tDatabase.h"

#include "../tron/gHelper/gHelperUtilities.h"

#include "eTimer.h"

extern bool se_playerStats, se_playerStatsLog;

std::vector<std::string> deserializeVector(const std::string &str);
std::string serializeVector(const std::vector<std::string> &vec);
std::string join(const std::vector<std::string> &vec, const std::string &delimiter);

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
    int total_score = 0;

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

    REAL getKDRatio() const
    {
        REAL result = 0.0;
        
        if (deaths == 0)
        {
            result = kills;
        }
        else if (kills >= deaths)
        {
            result = static_cast<REAL>(kills) / deaths;
        }
        else
        {
            result = -static_cast<REAL>(deaths) / kills;
        }
        return customRound(result,2);
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

    static void addScore(ePlayerNetID * player, int score)
    {
        PlayerData &stats = getStats(player);
        stats.total_score += score;
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

const std::vector<ColumnMapping> ePlayerStatsMappings = {
    {"name", "TEXT PRIMARY KEY",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) {
            sqlite3_bind_text(stmt, col++, stats.name.c_str(), -1, SQLITE_STATIC);
        },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) {
            stats.name = tString((char *)sqlite3_column_text(stmt, col++));
        }
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

    {"total_score", "INTEGER",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_int(stmt, col++, stats.total_score); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) { stats.total_score = sqlite3_column_int(stmt, col++);  }
    },

};

class ePlayerStatsDBAction  : public tDatabase<PlayerData, ColumnMapping> {
public:
    ePlayerStatsDBAction (sqlite3* db)
        : tDatabase<PlayerData, ColumnMapping>(db, "ePlayerStats", ePlayerStatsMappings) {}

    PlayerData& getTargetObject(const tString &name) override {
        return ePlayerStats::getStats(name);
    }

std::vector<PlayerData> getAllObjects() override {
    std::vector<PlayerData> playerDataVec;
    for (auto& pair : ePlayerStats::playerStatsMap) 
    {
        pair.second.name = pair.first;  
        playerDataVec.push_back(pair.second); 
    }
    return playerDataVec;
}

    void postLoadActions(PlayerData& playerData) override {
        playerData.data_from_db = playerData;
    }
};



#endif
