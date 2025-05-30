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

extern std::string DB_DELIMITER();

extern bool se_playerStats, se_playerStatsLog;

std::vector<std::string> deserializeVector(const std::string &str);
std::string serializeVector(const std::vector<std::string> &vec);
std::string join(const std::vector<std::string> &vec, const std::string &delimiter);

extern bool se_playerStatsLocalForcedName,
            se_playerStatsStoreBotMessages;

extern REAL  se_playerStatsRageQuitTime;

class PlayerDataBase
{
public:
    // Player
    int r,g,b;
    tString name;
    int total_messages                 = 0;
    REAL total_play_time               = 0;
    REAL total_spec_time               = 0;
    int times_joined                   = 0;
    time_t last_seen                   = 0;
    time_t last_seen_notification_time = 0;
    bool is_local                      = false;
    bool human                         = true;
    bool in_server                     = false;
    bool seen_this_session             = false;
    bool in_game                       = false; // current team?
    std::string nickname               = "";

    int times_banned                   = 0;
    bool banned_a_player_this_round    = false;
    int bans_given                     = 0;
    int rage_quits                     = 0;

    std::vector<std::string> chat_messages;
    std::vector<std::string> privated_stats;
    std::vector<std::string> name_history;
    std::vector<std::string> acheivement_history;
    std::vector<std::string> notifications;
    std::vector<std::string> sent_notifications;




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
    bool new_max_kill_streak        = false;
    int kills_while_dead            = 0;
    bool alive                      = false;

    // Database
    bool deleted                    = false;

    void Reset()
    {
        // Player
        r = 0; g = 0; b = 0;
        name = "";
        total_messages = 0;
        total_play_time = 0.0;
        total_spec_time = 0.0;
        times_joined = 0;
        last_seen = 0;
        is_local = false;
        human = true;
        in_server = false;
        seen_this_session = false;
        in_game = false;

        // Clearing vectors
        chat_messages.clear();
        privated_stats.clear();
        name_history.clear();
        acheivement_history.clear();

        // Cycle
        kills = 0;
        deaths = 0;
        match_wins = 0;
        match_losses = 0;
        round_wins = 0;
        round_losses = 0;
        rounds_played = 0;
        matches_played = 0;
        total_score = 0;
        fastest_speed = 0.0;
        current_kill_streak = 0;
        max_kill_streak = 0;
        kills_while_dead = 0;
        alive = false;

        // Database
        deleted = false;
    }

    using StatFunction = std::function<tString(PlayerDataBase *)>;
    using StatInfo = std::pair<std::string, PlayerDataBase::StatFunction>;
    static std::map<std::string, StatInfo> valueMap;

    tString rgbString( bool cyclePreview = false )
    {
        tString output;
        output << "("
               << r << ", "
               << g << ", "
               << b
               << ")";
        if (cyclePreview)
            output << " "
                   << ColorsCommand::cycleColorPreview(r,g,b, false);
        return output;
    }

    REAL getLastSeenAgo()
    {
        if (last_seen == 0 || in_server)
            return 0;

        time_t now = time(NULL);
        double lastSeen = difftime(now, last_seen);

        return lastSeen;
    }

    REAL calculatedScore()
    {
        return total_score;
    }

    tString getLastSeenAgoStr(bool exact = false)
    {
        REAL lastSeen = getLastSeenAgo();

        if (lastSeen == 0)
        {
            if (in_server)
                return tString("Now");
            else
                return tString("Never seen");
        }

        return exact ? st_GetFormatTime(lastSeen) : getTimeAgoString(lastSeen);
    }

    REAL getPlayTime()
    {
        if (in_game)
            return total_play_time + se_GameTime();

        return total_play_time;
    }

    tString getPlayTimeStr()
    {
        REAL playTime = getPlayTime();

        return playTime > 0 ? st_GetFormatTime(playTime, false, false) : tString("0");
    }

    REAL getSpecTime()
    {
        if (in_server && !in_game)
            return total_spec_time + se_GameTime();

        return total_spec_time;
    }

    tString getSpecTimeStr()
    {
        REAL specTime = getSpecTime();

        return specTime > 0 ? st_GetFormatTime(specTime, false, false) : tString("0");
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

    tString getAcheivmentsString()
    {
        tString messages;
        bool first = true;

        for (auto messageIt = acheivement_history.rbegin(); messageIt != acheivement_history.rend(); ++messageIt)
        {
            if (!first)
                messages << ", ";
            else
                first = false;

            messages << *messageIt;
        }

        return messages;
    }

    int getMessageCount()
    {
        return total_messages;
    }

    tString getNameHistory()
    {
        tString names;
        bool first = true;

        for (const auto &name : name_history)
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

    std::string getKDRatio(bool round = true) const
    {
        std::ostringstream stream;
        double kills_d = static_cast<double>(kills);
        double deaths_d = static_cast<double>(deaths);

        double result = deaths_d > 0 ? kills_d / deaths_d : kills_d;

        if (round)
        {
            result = customRound(result, 2);
            stream << result;
        }
        else
        {
            stream << std::fixed << std::setprecision(10) << result;
        }

        return stream.str();
    }

    REAL getSpeed(bool round = true) const
    {
        return round ? customRound(fastest_speed, 2) : fastest_speed;
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

    bool isPrivate(tString statLabel)
    {
        if (privated_stats.empty())
            return false;

        if (std::find(privated_stats.begin(), privated_stats.end(), "ALL STATS") != privated_stats.end())
            return true;

        return std::find(privated_stats.begin(), privated_stats.end(), statLabel.stdString()) != privated_stats.end();
    }
};

class PlayerData : public PlayerDataBase
{
public:
    static const std::set<std::string> valueMapdisplayFields;
    PlayerDataBase stats_this_session;
    PlayerDataBase data_from_db;

    PlayerDataBase &thisSession() { return stats_this_session; }

    void consolidateStats(const std::vector<PlayerDataBase> &playersToConsolidate)
    {
        for (const auto &player : playersToConsolidate)
        {
            total_messages += player.total_messages;
            total_play_time += player.total_play_time;
            total_spec_time += player.total_spec_time;
            times_joined += player.times_joined;

            if (player.last_seen > last_seen)
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

            if (player.fastest_speed > fastest_speed)
                fastest_speed = player.fastest_speed;

            current_kill_streak += player.current_kill_streak;

            if (player.max_kill_streak > max_kill_streak)
                max_kill_streak = player.max_kill_streak;

            kills_while_dead += player.kills_while_dead;

            chat_messages.insert(chat_messages.end(), player.chat_messages.begin(), player.chat_messages.end());

            std::set<std::string> uniquePrivateStats(privated_stats.begin(), privated_stats.end());
            for (const auto &player : playersToConsolidate)
                uniquePrivateStats.insert(player.privated_stats.begin(), player.privated_stats.end());

            privated_stats.assign(uniquePrivateStats.begin(), uniquePrivateStats.end());
        }
    }

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
        size_t count = 0;
        for (const auto &name : displayFields)
        {
            result << name;
            if (++count < displayFields.size())
                result << ", ";
        }
        return result;
    }

    void addPrivateStat(tString statLabel)
    {
        std::string label = statLabel.stdString();
        PlayerDataBase &statsThisSession = stats_this_session;

        if (label == "ALL STATS")
        {
            statsThisSession.privated_stats.clear();
            statsThisSession.privated_stats.push_back(label);
            privated_stats.clear();
            privated_stats.push_back(label);
            return;
        }

        auto allStatsIt = std::find(privated_stats.begin(), privated_stats.end(), "ALL STATS");

        if (allStatsIt != privated_stats.end())
        {
            privated_stats.erase(allStatsIt);
            statsThisSession.privated_stats.clear();
        }

        if (!isPrivate(statLabel))
        {
            statsThisSession.privated_stats.push_back(label);
            privated_stats.push_back(label);
        }
    }

    void removePrivateStat(tString statLabel)
    {
        std::string label = statLabel.stdString();
        PlayerDataBase &statsThisSession = stats_this_session;

        if (label == "ALL STATS")
        {
            privated_stats.clear();
            statsThisSession.privated_stats.clear();
            return;
        }

        auto it = std::find(privated_stats.begin(), privated_stats.end(), label);
        if (it != privated_stats.end())
        {
            privated_stats.erase(it);

            auto sessionIt = std::find(statsThisSession.privated_stats.begin(), statsThisSession.privated_stats.end(), label);
            if (sessionIt != statsThisSession.privated_stats.end())
                statsThisSession.privated_stats.erase(sessionIt);
        }
    }
};

class ePlayerStatsAcheivements
{
public:
    enum AcheivementsTypes
    {
        KILLS,
        CHATS,
        JOINS,
        BANS,
    };

    static bool performAction(PlayerData &stats, AcheivementsTypes type)
    {
        if (!se_playerMessageTriggers || !se_playerMessageTriggersAcheivements)
            return false;

        bool stored = false;
        tString response;
        REAL delay = 0.0;
        tString playerName(stats.name);
        eChatBot &bot = eChatBot::getInstance();

        switch (type)
        {
        case KILLS:
            handleKills(bot, stats, playerName, response, delay);
            break;
        case CHATS:
            handleChats(bot, stats, playerName, response, delay);
            break;
        case JOINS:
            handleJoins(bot, stats, playerName, response, delay);
            break;
        case BANS:
            handleBans(bot, stats, playerName, response, delay);
            break;
        default:
            break;
        }

        if (!response.empty())
        {
            stats.acheivement_history.push_back(response.stdString());
            stored = true;

            if ((se_playerMessageTriggers && se_playerMessageTriggersAcheivements) && (!stats.is_local || se_playerMessageTriggersAcheivementsLocal))
            {
                bot.Messager()->SetOutputParams(response, delay);
                bot.Messager()->Send();
            }
        }
        return stored;
    }

private:
    static void handleKills(eChatBot &bot, const PlayerData &stats, const tString &playerName, tString &response, REAL &delay)
    {
        int currentKills = stats.kills;
        int currentKillStreak = stats.current_kill_streak;
        int maxKillStreak = stats.max_kill_streak;

        handleAchievement(bot, stats, playerName,
                          currentKills, se_playerMessageTriggersAcheivementsKillsChangeVal, "$acheivements_kills",
                          response, delay);

        handleAchievement(bot, stats, playerName,
                          currentKillStreak, se_playerMessageTriggersAcheivementsKillStreakChangeVal, "$acheivements_current_killstreak",
                          response, delay);

        if (maxKillStreak > 1 && stats.new_max_kill_streak)
        {
            handleAchievement(bot, stats, playerName,
                              maxKillStreak, se_playerMessageTriggersAcheivementsMaxKillStreak, "$acheivements_max_killstreak",
                              response, delay);
        }
    }

    static void handleChats(eChatBot &bot, const PlayerData &stats, const tString &playerName, tString &response, REAL &delay)
    {
        handleAchievement(bot, stats, playerName,
                          stats.total_messages, se_playerMessageTriggersAcheivementsChatsChangeVal, "$acheivements_chats",
                          response, delay);
    }

    static void handleJoins(eChatBot &bot, const PlayerData &stats, const tString &playerName, tString &response, REAL &delay)
    {
        handleAchievement(bot, stats, playerName,
                          stats.times_joined, se_playerMessageTriggersAcheivementsJoinsChangeVal, "$acheivements_joins",
                          response, delay);
    }

    static void handleBans(eChatBot &bot, const PlayerData &stats, const tString &playerName, tString &response, REAL &delay)
    {
        handleAchievement(bot, stats, playerName,
                          stats.times_banned, se_playerMessageTriggersAcheivementsBansChangeVal, "$acheivements_bans",
                          response, delay);
    }

    static void handleAchievement(eChatBot &bot, const PlayerData &stats, const tString &playerName,
                                  int statValue, int changeVal, std::string trigger,
                                  tString &response, REAL &delay)
    {
        if (statValue != 0 && statValue % changeVal == 0)
        {
            tString value;
            value << statValue;

            eChatBot::findResponse(bot, playerName, tString(trigger), value);

            delay += bot.Messager()->Params().delay;

            if (!response.empty())
                response << " | ";

            response << bot.Messager()->Params().response;
        }
    }
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
    static bool         statsLoaded, statsSaved;
    static std::chrono::system_clock::time_point statsDBCreationDate;
    static std::unordered_map<tString, PlayerData> playerStatsMap;
    static int players_record_this_session;
public:
    static bool shouldEnforceLocalName(ePlayerNetID * player)
    {
        return player && player->isLocal() && se_playerStatsLocalForcedName;
    }

    static std::chrono::system_clock::time_point getStatsDBCreationDate() { return statsDBCreationDate; }
    static void statsLoadedCheck()
    {
        if (!statsWereLoaded())
            loadStatsFromDB();
    }

    static tString getEnforcedLocalName(ePlayerNetID * player)
    {
        ePlayer * local_p = ePlayer::NetToLocalPlayer(player);

        if (local_p) {
            tString name;
            name << tColoredString::RemoveColors(local_p->Name());
            return name.ToLower();
        }
        return tString("");
    }

    static PlayerData& getStats(ePlayerNetID * player)
    {
        tString name;

        if (player)
        {
            if (shouldEnforceLocalName(player))
                name << getEnforcedLocalName(player);
            else
                name << player->GetName().ToLower();
        }
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
                    ePlayerNetID *ourPlayer = ePlayerNetID::GetPlayerByName(playerName, true);
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
                [](const std::pair<tString, PlayerData> &a, const std::pair<tString, PlayerData> &b) {
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

    static PlayerData& getStatsForAnalysis(ePlayerNetID *player)
    {
        return getStatsForAnalysis(player->GetName());
    }

    static void playerInit(PlayerData &stats, ePlayerNetID *player)
    {
        stats.last_seen = time(NULL);
        stats.human = player->IsHuman();
        stats.is_local = player->isLocal();
        stats.in_server = true;

        if (!stats.seen_this_session)
        {
            stats.thisSession().Reset();
            stats.seen_this_session = true;
            players_record_this_session++;
        }
        else
        {
            stats.thisSession().last_seen = stats.last_seen;
            stats.thisSession().human = stats.human;
            stats.thisSession().is_local = stats.is_local;
            stats.thisSession().in_server = stats.in_server;
        }

        stats.times_joined++;
        stats.in_game = player->CurrentTeam() != nullptr;

        ePlayerStatsAcheivements::performAction(stats, ePlayerStatsAcheivements::AcheivementsTypes::JOINS);

        if (!stats.notifications.empty())
        {
            time_t latest = 0;

            const std::string& lastNote = stats.notifications.back();
            tArray<tString> parts = tString(lastNote).Split(tString(DB_DELIMITER()));

            if (parts.Len() >= 3)
            {
                try {
                    latest = std::stoll(parts[2].stdString());
                } catch (...) {}
            }

            if (latest > stats.last_seen_notification_time && player->Owner()) 
            {
                gHelperUtility::Debug("ePlayerStats", "Sending notification for player " + player->GetName().stdString());
                eChatBot::findResponsePlayer(eChatBot::getInstance(), player, se_playerMessageNotificationTrigger, tString(""), tString(""), true);
            }
        }
    }

    static void playerRenamed(ePlayerNetID *player)
    {
        statsLoadedCheck();

        if (shouldEnforceLocalName(player))
            return;

        PlayerData &oldStats = getStats(player->lastName);
        PlayerData &newStats = getStats(player);

        oldStats.name_history.push_back(player->GetName().stdString());
        newStats.name_history.push_back(player->lastName.stdString());

        newStats.thisSession().name_history.push_back(player->lastName.stdString());
        playerLeft(oldStats);
        playerInit(newStats, player);
        setColor(player);

        if (se_playerMessageTriggersContextBuilder)
        {
            tString context;
            context << player->lastName
                    << " renamed to "
                    << player->GetName();
            eChatBot::getInstance().data.StoreContextItem(context);
        }
    }

    static void playerJoined(ePlayerNetID * player)
    {
        statsLoadedCheck();

        PlayerData &stats = getStats(player);
        playerInit(stats, player);
        setColor(player);
    }

    static void playerLeft(ePlayerNetID * player)
    {
        if (!player)
            return;

        statsLoadedCheck();

        PlayerData &stats = getStats(player);
        playerLeft(stats);

        REAL lastDeathTime = tSysTimeFloat() - player->lastCycleDeathTime;

        if  (lastDeathTime > 0.005 && lastDeathTime < se_playerStatsRageQuitTime)
        {
            stats.rage_quits++;

            if (se_playerMessageTriggers && se_playerMessageTriggersRageQuits)
            {
                static const tString valDelim = tString("$val1");

                eChatBot &bot = eChatBot::getInstance();
                bot.Messager()->ResetParams();
                bot.Messager()->Params().triggeredByName = (stats.name.empty() ? player->GetName() : stats.name);
                bot.Messager()->SetInputParams(nullptr, tString("$ragequit"), true);
                bot.Messager()->FindTriggeredResponse();

                tString value;
                value << lastDeathTime;

                if (!bot.Messager()->Params().response.empty())
                    bot.Messager()->Params().response = bot.Messager()->Params().response.Replace(valDelim, value);
                else
                    gHelperUtility::Debug("eChatBot", "No trigger set for '$ragequit' Set one with 'PLAYER_MESSAGE_TRIGGERS_ADD'\n");

                bot.Messager()->Send();

                if (se_playerMessageTriggersContextBuilder && !bot.Messager()->Params().response.empty())
                {
                    eChatBot &bot = eChatBot::getInstance();
                    eChatBot::getInstance().data.StoreContextItem(bot.Messager()->Params().response);
                }
            }
        }
        else if (se_playerMessageTriggersContextBuilder)
        {
            eChatBot &bot = eChatBot::getInstance();
            tString context;
            context << bot.Messager()->Params().triggeredByName
                    << " left the server.";
            eChatBot::getInstance().data.StoreContextItem(context);
        }


    }

    static void playerLeft(PlayerData &stats)
    {
        stats.last_seen                   = time(NULL);
        stats.in_server                   = false;
        stats.current_kill_streak         = 0;

        stats.thisSession().Reset();
    }

    static void addKill(ePlayerNetID *player)
    {
        statsLoadedCheck();

        PlayerData &stats = getStats(player);
        PlayerDataBase &statsThisSession = stats.thisSession();

        if (!stats.alive)
        {
            stats.kills_while_dead++;
            statsThisSession.kills_while_dead++;
        }

        stats.kills++;
        statsThisSession.kills++;

        stats.current_kill_streak++;
        statsThisSession.current_kill_streak++;

        if (stats.current_kill_streak > stats.max_kill_streak)
        {
            stats.max_kill_streak = stats.current_kill_streak;
            statsThisSession.max_kill_streak = std::max(statsThisSession.max_kill_streak, stats.current_kill_streak);
            stats.new_max_kill_streak = true;
        }
        else
        {
            stats.new_max_kill_streak = false;
        }

        ePlayerStatsAcheivements::performAction(stats, ePlayerStatsAcheivements::AcheivementsTypes::KILLS);

        if (se_playerMessageTriggersContextBuilder)
        {
            eChatBot &bot = eChatBot::getInstance();
            tString context;
            context << player->GetName()
                    << " core dumped "
                    << (player->lastKilledPlayer ? player->lastKilledPlayer->GetName() : "unknown");
            eChatBot::getInstance().data.StoreContextItem(context);
        }
    }

    static void addDeath(ePlayerNetID *player)
    {
        statsLoadedCheck();

        PlayerData &stats = getStats(player);
        PlayerDataBase &statsThisSession = stats.thisSession();

        stats.deaths++;
        statsThisSession.deaths++;

        stats.current_kill_streak = 0;
        statsThisSession.current_kill_streak = 0;

        stats.alive = false;

        statsThisSession.alive = false;
        if (se_playerMessageTriggersContextBuilder)
        {
            eChatBot &bot = eChatBot::getInstance();
            tString context;
            context << player->GetName()
                    << " was core dumped by "
                    << (player->lastDiedByPlayer ? player->lastDiedByPlayer->GetName() : "unknown");
            eChatBot::getInstance().data.StoreContextItem(context);
        }
    }

    static void setColor(ePlayerNetID * player, int r, int g, int b)
    {
        tString name;
        name << ((shouldEnforceLocalName(player) ? getEnforcedLocalName(player) : player->GetName().ToLower()));

        PlayerData &stats = playerStatsMap[name];
        stats.r = r;
        stats.g = g;
        stats.b = b;
        stats.name = name;

        PlayerDataBase &statsThisSession = stats.thisSession();
        statsThisSession.r = stats.r;
        statsThisSession.g = stats.g;
        statsThisSession.b = stats.b;
        statsThisSession.name = stats.name;
    }

    static void setColor(ePlayerNetID * player)
    {
        setColor(player, player->r, player->g, player->b);
    }

    static void addScore(ePlayerNetID * player, int score)
    {
        statsLoadedCheck();

        PlayerData &stats = getStats(player);
        PlayerDataBase &statsThisSession = stats.thisSession();

        stats.total_score += score;
        statsThisSession.total_score += score;
    }

    static void addMessage(ePlayerNetID * player, tString message)
    {
        statsLoadedCheck();

        PlayerData &stats = getStats(player);
        PlayerDataBase &statsThisSession = stats.thisSession();

        if ( (!player->isLocal() || !tIsEnabledForPlayer(se_playerMessageEnabledPlayers, player->pID+1) ) || se_playerStatsStoreBotMessages)
        {
            if (!message.empty())
            {
                stats.chat_messages.push_back(message.stdString());
                statsThisSession.chat_messages.push_back(message.stdString());
            }
        }
        stats.total_messages++;
        statsThisSession.total_messages++;
        ePlayerStatsAcheivements::performAction(stats, ePlayerStatsAcheivements::AcheivementsTypes::CHATS);

        if (se_playerMessageTriggersContextBuilder)
        {
            eChatBot &bot = eChatBot::getInstance();
            tString context;
            context << player->GetName()
                    << " said: '"
                    << message
                    << "'";
            eChatBot::getInstance().data.StoreContextItem(context);
        }
    }

    static tString deletePlayerStats(tString input)
    {
        statsLoadedCheck();

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
                ePlayerStats::reloadStatsFromDB();
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

        for (int i = 0; i < deleteState.additionalPlayers.Len(); ++i)
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

    static REAL getTotalPlayersLogged(bool current = true)
    {
        return current ? players_record_this_session : static_cast<REAL>(playerStatsMap.size());
    }

    static std::unordered_map<tString, PlayerData>& GetPlayerStatsMap()
    {
        return playerStatsMap;
    }

    static void updateStatsMatchEnd(ePlayerNetID *matchWinner);
    static void updateStatsRoundStart();
    static void updateStatsRoundEnd();

    static bool loadStatsFromDB();
    static bool saveStatsToDB();
    static bool reloadStatsFromDB();

    static bool statsWereLoaded() { return statsLoaded; }
};

using PlayerDataColumnMapping = ColumnMapping<PlayerData>;
extern const std::vector<PlayerDataColumnMapping > ePlayerStatsMappings;
class ePlayerStatsDBAction : public tDatabase<PlayerData, TableDefinition<PlayerData>>
{
public:
    ePlayerStatsDBAction(sqlite3* db)
        : tDatabase<PlayerData, TableDefinition<PlayerData>>(
            db,
            "ePlayerStats",
            TableDefinition<PlayerData>(
                [] { // Primary key
                    TableDefinition<PlayerData>::PrimaryKey pk;
                    pk.columnName = "name";
                    pk.getValueFunc = [](const PlayerData &object) -> std::string {
                        return object.name.stdString();
                    };
                    return pk;
                }(),
                ePlayerStatsMappings
            )
        )
    {}

    PlayerData& getTargetObject(const tString &name) override
    {
        return ePlayerStats::getStats(name);
    }

    std::vector<PlayerData> getAllObjects() override
    {
        std::vector<PlayerData> playerDataVec;
        time_t currentTime = time(NULL);
        const time_t seventyTwoHoursInSeconds = 72 * 60 * 60;

        for (auto& pair : ePlayerStats::GetPlayerStatsMap())
        {
            PlayerData &stats = pair.second;

            if (currentTime - stats.last_seen > seventyTwoHoursInSeconds)
                stats.nickname = "";

            stats.name = pair.first;
            if (stats.rounds_played >= 1 || stats.total_spec_time >= 100)
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
