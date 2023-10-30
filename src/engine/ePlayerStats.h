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
#include "eChatBot.h"

extern bool se_playerStats, se_playerStatsLog;

std::vector<std::string> deserializeVector(const std::string &str);
std::string serializeVector(const std::vector<std::string> &vec);
std::string join(const std::vector<std::string> &vec, const std::string &delimiter);

extern bool se_playerStatsLocalForcedName;

class PlayerDataBase
{
public:
    // Player
    int r,g,b;
    tString name;
    int total_messages              = 0;
    int messages_this_session       = 0;
    REAL total_play_time            = 0;
    REAL play_time_this_session     = 0;
    REAL total_spec_time            = 0;
    REAL spec_time_this_session     = 0;
    int times_joined                = 0;
    time_t last_seen                = 0;
    bool is_local                   = false;
    bool human                      = true;
    bool in_server                  = false;
    bool seen_this_session          = false;
    bool in_game                    = false; // current team?
    
    std::vector<std::string> chat_messages;
    std::vector<std::string> privated_stats;
    std::vector<std::string> name_history;

    // Cycle
    int kills                       = 0;
    int deaths                      = 0;
    int match_wins                  = 0;
    int match_losses                = 0;
    int round_wins                  = 0;
    int round_losses                = 0;
    int rounds_played               = 0;
    int matches_played              = 0;
    int total_score                 = 0;
    REAL fastest_speed              = 0;
    int current_kill_streak         = 0;
    int max_kill_streak             = 0;
    int kills_while_dead            = 0;
    bool alive                      = false;
    
    bool deleted                    = false;

    void consolidateStats(const std::vector<PlayerDataBase> &playersToConsolidate)
    {
        for(const auto &player : playersToConsolidate)
        {
            total_messages += player.total_messages;
            total_play_time += player.total_play_time;
            total_spec_time += player.total_spec_time;
            times_joined += player.times_joined;

            if(player.last_seen > last_seen)
                last_seen = player.last_seen;

            kills += player.kills;
            deaths += player.deaths;
            match_wins += player.match_wins;
            match_losses += player.match_losses;
            round_wins += player.round_wins;
            round_losses += player.round_losses;
            rounds_played += player.rounds_played;
            matches_played += player.matches_played;
            total_score += player.total_score;

            if(player.fastest_speed > fastest_speed)
                fastest_speed = player.fastest_speed;

            current_kill_streak += player.current_kill_streak;

            if(player.max_kill_streak > max_kill_streak)
                max_kill_streak = player.max_kill_streak;

            kills_while_dead += player.kills_while_dead;

            chat_messages.insert(chat_messages.end(), player.chat_messages.begin(), player.chat_messages.end());

            std::set<std::string> uniquePrivateStats(privated_stats.begin(), privated_stats.end());
            for(const auto &player : playersToConsolidate)
            {
                uniquePrivateStats.insert(player.privated_stats.begin(), player.privated_stats.end());
            }
            privated_stats.assign(uniquePrivateStats.begin(), uniquePrivateStats.end());
        }
    }

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

    REAL getLastSeenAgo(bool current = true)
    {
        if (current && (last_seen == 0 || in_server))
            return 0;

        time_t now = time(NULL);
        double lastSeen = difftime(now, last_seen);

        return lastSeen;
    }

    REAL calculatedScore()
    {
        return total_score;
    }

    tString getLastSeenAgoStr(bool current = true, bool exact = false)
    {
        REAL lastSeen = getLastSeenAgo(current);

        if (current && lastSeen == 0)
        {
            if (in_server)
                return tString("Now");
            else
                return tString("Never seen");
        }

        return exact ? st_GetFormatTime(lastSeen) : getTimeAgoString(lastSeen);
    }

    REAL getPlayTime(bool current = true)
    {
        if (current && !in_game)
            return 0; 

        if (in_server)
        {
            if (in_game)
                return current ? play_time_this_session + se_GameTime() : total_play_time + se_GameTime();

            return current ? play_time_this_session : total_play_time;
        }

        return total_play_time;
    }

    REAL getSpecTime(bool current = true)
    {
        if (in_server)
        {
            return current ? spec_time_this_session + se_GameTime() : total_spec_time + se_GameTime();
        }
        return total_spec_time;
    }

    tString getChatMessages()
    {
        tString messages;
        bool first = true;

        for (auto messageIt = chat_messages.rbegin(); messageIt != chat_messages.rend(); ++messageIt)
        {
            if (!first)
                messages << ", ";
            else
                first = false;

            messages << *messageIt;
        }

        return messages;
    }

    int getMessageCount(bool current = true)
    {
        if (in_server)
        {
            return current ? messages_this_session : total_messages;
        }
        return total_messages;
    }

    tString getNameHistory()
    {
        tString names;
        bool first = true;

        for (const auto& name : name_history)
        {
            if (!first)
                names << ", ";
            else
                first = false;

            names << name;
        }

        return names;
    }

    tString getHiddenStats()
    {
        tString messages;
        bool first = true;

        for (auto messageIt = privated_stats.rbegin(); messageIt != privated_stats.rend(); ++messageIt)
        {
            if (!first)
                messages << ", ";
            else
                first = false;

            messages << *messageIt;
        }

        return messages;
    }

    REAL getKDRatio(bool round = true) const
    {
        REAL result;

        if (deaths == 0)
            result = kills;
        else
            result = static_cast<REAL>(kills) / deaths;


        return round ? customRound(result,2) : result;
    }

    REAL getSpeed(bool round = true) const
    {
        return round ? customRound(fastest_speed,2) : fastest_speed;
    }
};


class PlayerData : public PlayerDataBase
{
public:
    using StatFunction = std::function<tString(PlayerDataBase *)>;
    using StatInfo = std::pair<std::string, PlayerData::StatFunction>;
    static std::map<std::string, StatInfo> valueMap;

    static const std::set<std::string> valueMapdisplayFields;

    static tString getAvailableStatsStr(std::string source = "")
    {
        std::set<std::string> displayFields = PlayerData::valueMapdisplayFields;

        if (source == "leaderboardFunc")
        {
            displayFields.erase("rgb");
            displayFields.erase("chat_messages");
            displayFields.erase("hidden");
            displayFields.erase("name_history");
        }

        if (source != "hidestatfunc")
        {
            displayFields.erase("all");
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
        {
            auto func = stat->second.second;
            return func(this);
        }
        else
        {
            tString emptyVal;
            return emptyVal;
        }
    }

    static tString getAnyLabel(tString variable)
    {
        if (variable.empty())
            return tString("");

        auto stat = valueMap.find(variable.stdString());
        if (stat != valueMap.end())
        {
            return tString(stat->second.first.c_str());
        }
        else
        {
            return tString("");
        }
    }

    void addPrivateStat(tString statLabel)
    {
        std::string label = statLabel.stdString();

        if (label == "ALL STATS")
        {
            privated_stats.clear();
            privated_stats.push_back(label);
            return;
        }

        auto allStatsIt = std::find(privated_stats.begin(), privated_stats.end(), "ALL STATS");
        if (allStatsIt != privated_stats.end())
            privated_stats.erase(allStatsIt);

        if (!privateStat(statLabel))
            privated_stats.push_back(label);
    }

    void removePrivateStat(tString statLabel)
    {
        std::string label = statLabel.stdString();

        if (label == "ALL STATS")
        {
            privated_stats.clear();
            return;
        }

        auto it = std::find(privated_stats.begin(), privated_stats.end(), label);
        if (it != privated_stats.end())
            privated_stats.erase(it);
    }

    bool privateStat(tString statLabel)
    {
        if (privated_stats.empty())
            return false;

        if (std::find(privated_stats.begin(), privated_stats.end(), "ALL STATS") != privated_stats.end())
            return true;

        return std::find(privated_stats.begin(), privated_stats.end(), statLabel.stdString()) != privated_stats.end();
    }

    PlayerDataBase data_from_db;
};

struct CommandState
{
    tString confirmationKey;
    tString targetPlayerName;
    tArray<tString> additionalPlayers;
};

class ePlayerStats
{
    static CommandState deleteState;
    static CommandState consolidateState;

public:
    static tString deletePlayerStats(tString input)
    {
        tString output;
        input = input.ToLower().TrimWhitespace();

        // confirmation 
        if (!deleteState.confirmationKey.empty())
        {
            if (input != deleteState.confirmationKey)
            {
                output << "Invalid key! Deletion aborted.\n";
                deleteState.confirmationKey.Clear();
                deleteState.additionalPlayers.Clear();
                return output;
            }

            tString deletedPlayers;
            tString notFoundPlayers;

            for (const auto& playerName : deleteState.additionalPlayers)
            {
                // validation
                auto it = playerStatsMap.find(playerName);
                if (it != playerStatsMap.end())
                {
                    it->second.deleted = true; // database will delete
                    if (!deletedPlayers.empty()) {
                        deletedPlayers << ", ";
                    }
                    deletedPlayers << playerName;
                }
                else
                {
                    if (!notFoundPlayers.empty()) {
                        notFoundPlayers << ", ";
                    }
                    notFoundPlayers << playerName;
                }
            }

            if (!deletedPlayers.empty()) {
                output << "Stats for player(s) \"" << deletedPlayers << "\" marked as deleted.\n";
            }
            if (!notFoundPlayers.empty()) {
                output << "Player(s) \"" << notFoundPlayers << "\" not found! Deletion skipped.\n";
            }

            deleteState.confirmationKey.Clear();
            deleteState.additionalPlayers.Clear();
            return output;
        }

        deleteState.additionalPlayers = input.Split(",");

        if (deleteState.additionalPlayers.Empty())
        {
            output << "Invalid input. Usage: <playerToDelete1>,<playerToDelete2>,...\n";
            return output;
        }

        // confirmation key
        deleteState.confirmationKey << (rand() % 10) << (rand() % 10) << (rand() % 10);
        output << "Please enter the key: " << deleteState.confirmationKey << " to confirm deletion of stats for ";

        for (size_t i = 0; i < deleteState.additionalPlayers.Len(); ++i)
        {
            output << deleteState.additionalPlayers[i];
            if (i < deleteState.additionalPlayers.Len() - 1)
            {
                output << ", ";
            }
        }

        output << ".\n";

        return output;
    }


    static tString consolidatePlayerStats(tString input)
    {
        tString output;
        input = input.ToLower().TrimWhitespace();

        // confirmation 
        if (!consolidateState.confirmationKey.empty())
        {
            if (input != consolidateState.confirmationKey)
            {
                output << "Invalid key! Stored settings removed\n";
                consolidateState.confirmationKey.Clear();
                return output;
            }

            std::vector<PlayerDataBase> playersToConsolidateStats;

            for (const auto& playerName : consolidateState.additionalPlayers)
            {
                if (playerStatsMap.find(playerName) != playerStatsMap.end())
                {
                    PlayerData &stats = playerStatsMap[playerName];
                    playersToConsolidateStats.push_back(stats);
                    stats.deleted = true;
                }
                else
                {
                    output << "Player " << playerName << " not found!\n";
                    return output;
                }
            }

            PlayerData &targetPlayer = playerStatsMap[consolidateState.targetPlayerName];
            targetPlayer.consolidateStats(playersToConsolidateStats);
            output << "Stats consolidated successfully!\n";

            consolidateState.confirmationKey.Clear();
            consolidateState.targetPlayerName.Clear();
            consolidateState.additionalPlayers.Clear();
            return output;
        }

        int firstSpacePos = input.StrPos(tString(" "));
        if (firstSpacePos == -1)
        {
            output << "Invalid input. Usage: <playerToConsolidateTo> <player1>,<player2>,...\n";
            return output;
        }

        consolidateState.targetPlayerName = input.SubStr(0, firstSpacePos);
        tString playersList = input.SubStr(firstSpacePos + 1);
        consolidateState.additionalPlayers = playersList.Split(",");

        // validation
        if (playerStatsMap.find(consolidateState.targetPlayerName) == playerStatsMap.end())
        {
            output << "Main player not found!\n";
            return output;
        }

        for (auto& playerName : consolidateState.additionalPlayers)
        {
            if (playerStatsMap.find(playerName) == playerStatsMap.end())
            {
                output << "Player " << playerName << " not found!\n";
                return output;
            }
        }

        // confirmation key
        consolidateState.confirmationKey << (rand() % 10) << (rand() % 10) << (rand() % 10);
        output << "Please enter the key: " << consolidateState.confirmationKey << " to proceed with the consolidation.\n";
        return output;
    }

    static bool shouldEnforceLocalName(ePlayerNetID * player)
    {
        return player && player->isLocal() && se_playerStatsLocalForcedName;
    }

    static tString getEnforcedLocalName(ePlayerNetID * player)
    {
        ePlayer * local_p = ePlayer::NetToLocalPlayer(player);
        tString name;
        name << local_p->Name();

        if (local_p)
            return name.ToLower();

        return tString("");
    }

    static PlayerData& getStats(ePlayerNetID * player)
    {
        tString name;

        if (shouldEnforceLocalName(player))
            name << getEnforcedLocalName(player);
        else
            name << player->GetRealName().ToLower();

        return playerStatsMap[name];
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
            if (kv.second.deleted)
                continue;

            std::string currentNameLower = kv.first.stdString();
            std::transform(currentNameLower.begin(), currentNameLower.end(), currentNameLower.begin(), ::tolower);

            if (currentNameLower == playerNameLower)
            {
                if (se_playerStatsLocalForcedName)
                {
                    ePlayerNetID *ourPlayer = ePlayerNetID::GetPlayerByRealName(playerName, true);
                    if (ourPlayer && shouldEnforceLocalName(ourPlayer))
                        return playerStatsMap[getEnforcedLocalName(ourPlayer)];
                }

                return playerStatsMap[kv.first];
            }
        }

        // sort players by last_seen if their names match the substring
        std::vector<std::pair<tString, PlayerData>> matchedPlayers;

        for (const auto &kv : playerStatsMap)
        {
            if (kv.second.deleted) {
                continue;
            }

            std::string currentNameLower = kv.first.stdString();
            std::transform(currentNameLower.begin(), currentNameLower.end(), currentNameLower.begin(), ::tolower);

            if (currentNameLower.find(playerNameLower) != std::string::npos)
            {
                matchedPlayers.push_back(kv);
            }
        }

        if (!matchedPlayers.empty())
        {
            std::sort(matchedPlayers.begin(), matchedPlayers.end(),
                    [](const auto &a, const auto &b) {
                        return a.second.last_seen > b.second.last_seen;
                    });

            return playerStatsMap[matchedPlayers.front().first];
        }

        // remove from map if not found
        auto it = playerStatsMap.find(playerName.ToLower());
        if(it != playerStatsMap.end())
        {
            playerStatsMap.erase(it);
        }

        static PlayerData defaultPlayerData;
        return defaultPlayerData;
    }


    static void addKill(ePlayerNetID *player)
    {
        PlayerData &stats = getStats(player);

        if (!stats.alive)
            stats.kills_while_dead++;

        stats.kills++;
        stats.current_kill_streak++;
        if (stats.current_kill_streak > stats.max_kill_streak)
            stats.max_kill_streak = stats.current_kill_streak;
    }


    static void addDeath(ePlayerNetID *player)
    {
        PlayerData &stats = getStats(player);
        stats.deaths++;
        stats.current_kill_streak = 0;
        stats.alive = false;
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
        tString name;
        name << ((shouldEnforceLocalName(player) ? getEnforcedLocalName(player) : player->GetRealName().ToLower()));

        PlayerData &stats = playerStatsMap[name];
        stats.r = r;
        stats.g = g;
        stats.b = b;
        stats.name = name;
    }

    static void setColor(ePlayerNetID * player)
    {
        setColor(player, player->r, player->g, player->b);
    }

    static void playerInit(PlayerData &stats, ePlayerNetID *player)
    {
        stats.last_seen           = time(NULL);
        stats.human               = player->IsHuman();
        stats.is_local            = player->isLocal();
        stats.in_server           = true;

        stats.seen_this_session   = true;
        stats.current_kill_streak = 0;

        stats.times_joined++;
        if (!stats.seen_this_session)
            players_record_this_session++;

        stats.in_game = player->CurrentTeam() != nullptr;
    }

    static void playerRenamed(ePlayerNetID *player)
    {
        if (shouldEnforceLocalName(player))
            return;

        PlayerData &oldStats = getStats(player->lastName);
        PlayerData &newStats = getStats(player);

        playerLeft(oldStats);
        playerInit(newStats, player);
        newStats.name_history.push_back(player->lastName.stdString());
        setColor(player);

    }

    static void playerJoined(ePlayerNetID * player)
    {
        PlayerData &stats = getStats(player);
        playerInit(stats, player);
        setColor(player);
    }

    static void playerLeft(ePlayerNetID * player)
    {
        PlayerData &stats = getStats(player);
        playerLeft(stats);
    }

    static void playerLeft(PlayerData &stats)
    {
        stats.last_seen                   = time(NULL);
        stats.in_server                   = false;
        stats.current_kill_streak         = 0;
        stats.spec_time_this_session      = 0;
        stats.play_time_this_session      = 0;
        stats.messages_this_session       = 0;
    }

    static void addScore(ePlayerNetID * player, int score)
    {
        PlayerData &stats = getStats(player);
        stats.total_score += score;
    }

    static void addMessage(ePlayerNetID * player, tString message)
    {
        PlayerData &stats = getStats(player);

        if (!message.empty())
            stats.chat_messages.push_back(message.stdString());

        stats.total_messages++;
        stats.messages_this_session++;
    }

    static void updateStatsMatchEnd(ePlayerNetID *matchWinner);
    static void updateStatsRoundStart();
    static void updateStatsRoundEnd();

    static void loadStatsFromDB();
    static void saveStatsToDB();
    static void reloadStatsFromDB();


    static REAL getTotalPlayersLogged(bool current = true)
    {
        return current ? players_record_this_session : static_cast<REAL>(playerStatsMap.size());
    }

    static std::unordered_map<tString, PlayerData> playerStatsMap;
    static int players_record_this_session;
};

using PlayerDataColumnMapping = ColumnMapping<PlayerData>;
extern const std::vector<PlayerDataColumnMapping > ePlayerStatsMappings;

class ePlayerStatsDBAction  : public tDatabase<PlayerData, PlayerDataColumnMapping >
{
public:
    ePlayerStatsDBAction (sqlite3* db)
        : tDatabase<PlayerData, PlayerDataColumnMapping >(db, "ePlayerStats", ePlayerStatsMappings) {}

    PlayerData& getTargetObject(const tString &name) override
    {
        return ePlayerStats::getStats(name);
    }

    std::vector<PlayerData> getAllObjects() override
    {
        std::vector<PlayerData> playerDataVec;
        for (auto& pair : ePlayerStats::playerStatsMap)
        {
            PlayerData &stats = pair.second;

            stats.name = pair.first;
            if (stats.rounds_played >= 1 || stats.total_spec_time >= 100 )
                playerDataVec.push_back(stats);
        }
        return playerDataVec;
    }

    void postLoadActions(PlayerData& playerData) override
    {
        playerData.data_from_db = playerData;
    }

    void preSaveAction(PlayerData& playerData) override
    {
        if (playerData.deleted)
        {
            markForDeletion(playerData);
        }
    }

};

#endif
