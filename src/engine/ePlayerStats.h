#ifndef ArmageTron_ePlayerStats_H
#define ArmageTron_ePlayerStats_H

#include "tDirectories.h"
#include "sqlite3.h"
#include "ePlayer.h"
#include <unordered_map>

#include "eTimer.h"

extern bool se_playerStats, se_playerStatsLog;

class PlayerDataBase
{
public:
    // Player
    int r;
    int g;
    int b;

    int total_messages = 0;

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
    REAL total_play_time = 0;
    REAL total_spec_time = 0;
    int times_joined     = 0;
    REAL fastest_speed   = 0;

    std::vector<std::string> chatMessages;

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

            for (auto messageIt = chatMessages.rbegin(); messageIt != chatMessages.rend(); ++messageIt)
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

    static tString getAvailableStatsStr()
    {
        tString result;
        result << "Available stats are: ";
        for (const auto &name : PlayerData::valueMapdisplayFields)
            result << name << ", ";

        result = result.SubStr(0, result.Len() - 2);
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

    static void addMessage(ePlayerNetID * player, tString message)
    {
        PlayerData &stats = getStats(player);

        stats.chatMessages.push_back(message.stdString());
        stats.total_messages++;
    }

    static void updateMatchWinsAndLoss(ePlayerNetID *matchWinner);
    static void updateRoundWinsAndLoss();

    static void ensureTableAndColumnsExist(sqlite3* db);
    static void loadStatsFromDB();
    static void saveStatsToDB();
    static void reloadStatsFromDB();


    static REAL getTotalPlayersLogged()
    {
        return static_cast<REAL>(playerStatsMap.size());
    }

    static std::unordered_map<tString, PlayerData> playerStatsMap;
};


#endif
