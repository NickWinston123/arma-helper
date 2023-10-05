#ifndef ArmageTron_ePlayerStats_H
#define ArmageTron_ePlayerStats_H

#include "tDirectories.h"
#include "sqlite3.h"
#include "ePlayer.h"
#include <unordered_map>

#include "eTimer.h"

extern bool se_playerStats;

struct PlayerData {

    // Player
    tColor rgb; //int r_, int g_, int b_
    int total_messages = 0;

    tString rgbString() 
    {
        tString output;
                output  << "("
                        << rgb.r_ << ", " 
                        << rgb.g_ << ", " 
                        << rgb.b_ << ")";
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
    
    REAL getTotalPlayTime()
    {
        return total_play_time + se_GameTime();
    }

    double getKDRatio() 
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

class ePlayerStats 
{
public:

    static PlayerData& getStats(ePlayerNetID * player)
    {
        return playerStatsMap[player->GetName()];
    }

    static PlayerData& getStats(tString playerName)
    {
        return playerStatsMap[playerName];
    }

    static void addKill(ePlayerNetID *player)
    {
        playerStatsMap[player->GetName()].kills++;
    }

    static void addDeath(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].deaths++;
    }

    static void addMatchWin(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].match_wins++;
    }

    static void addMatchLoss(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].match_losses++;
    }

    static void addMatchPlayed(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].matches_played++;
    }

    static void addRoundWin(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].round_wins++;
    }

    static void addRoundLoss(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].round_losses++;
    }

    static void addRoundPlayed(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].rounds_played++;
    }

    static void addPlayTime(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].total_play_time += se_GameTime();
    }

    static void setColor(ePlayerNetID * player, tColor rgb)
    {
        playerStatsMap[player->GetName()].rgb = rgb;
    }

    static void addTotalMessages(ePlayerNetID * player)
    {
        playerStatsMap[player->GetName()].total_messages++;
    }

    static void updateMatchWinsAndLoss(ePlayerNetID *matchWinner);
    static void updateRoundWinsAndLoss();

    static void loadStatsFromDB();
    static void saveStatsToDB();
    static void reloadStatsFromDB();

    static std::unordered_map<tString, PlayerData> playerStatsMap;
};


#endif
