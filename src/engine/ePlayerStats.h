#ifndef ArmageTron_ePlayerStats_H
#define ArmageTron_ePlayerStats_H

#include "tDirectories.h"
#include "sqlite3.h"
#include "ePlayer.h"
#include <unordered_map>

#include "eTimer.h"

extern bool se_playerStats;

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

    REAL getTotalPlayTime(bool add = true)
    {
        return total_play_time + (add ? se_GameTime() : 0);
    }

    REAL getTotalSpecTime(bool add = true)
    {
        return total_spec_time + (add ? se_GameTime() : 0);
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
        return playerStatsMap[player->GetRealName()];
    }

    static PlayerData& getStats(tString playerName)
    {
        return playerStatsMap[playerName];
    }

    static void addKill(ePlayerNetID *player)
    {
        playerStatsMap[player->GetRealName()].kills++;
    }

    static void addDeath(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].deaths++;
    }

    static void addMatchWin(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].match_wins++;
    }

    static void addMatchLoss(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].match_losses++;
    }

    static void addMatchPlayed(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].matches_played++;
    }

    static void addRoundWin(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].round_wins++;
    }

    static void addRoundLoss(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].round_losses++;
    }

    static void addRoundPlayed(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].rounds_played++;
    }

    static void addPlayTime(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].total_play_time += se_GameTime();
    }

    static void addSpecTime(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].total_spec_time += se_GameTime();
    }

    static void addJoined(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].times_joined++;
    }

    static void setColor(ePlayerNetID * player, int r, int g, int b)
    {
        tString name = player->GetRealName();

        PlayerData &data = playerStatsMap[name];
        data.r = r;
        data.g = g;
        data.b = b;
    }

    static void addTotalMessages(ePlayerNetID * player)
    {
        playerStatsMap[player->GetRealName()].total_messages++;
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


#endif
