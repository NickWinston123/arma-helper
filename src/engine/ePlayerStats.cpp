#include "tDatabase.h"
#include "ePlayerStats.h"
#include "ePlayer.h"
#include "eChatbot.h"
#include "eChatCommands.h"
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

tString se_playerStatsDataBaseFile("stats.db");
static tConfItem<tString> se_playerStatsDataBaseFileConf("PLAYER_STATS_DB_FILE", se_playerStatsDataBaseFile);

bool se_playerMessageTriggersStatsSave = false;
static tConfItem<bool> se_playerMessageTriggersStatsSaveConf("PLAYER_MESSAGE_TRIGGER_STATS_SAVED", se_playerMessageTriggersStatsSave);

bool se_playerStatsLocalForcedName = false;
static tConfItem<bool> se_playerStatsLocalForcedNameConf("PLAYER_STATS_LOCAL_FORCED_NAME", se_playerStatsLocalForcedName);

int ePlayerStats::players_record_this_session = 0;

void ePlayerStats::loadStatsFromDB()
{
    sqlite3 *db = tDatabaseUtility::OpenDatabase(se_playerStatsDataBaseFile);
    if (!db)
        return;

    ePlayerStatsDBAction  playerDataAction(db);
    playerDataAction.Load();

    eChatBotStatsDBAction chatbotStatsAction(db);
    chatbotStatsAction.Load();

    sqlite3_close(db);
}

void ePlayerStats::saveStatsToDB()
{
    sqlite3 *db = tDatabaseUtility::OpenDatabase(se_playerStatsDataBaseFile);
    if (!db)
        return;

    con << tThemedTextBase.LabelText("Player Stats")
        << "Saving stats to database..\n";

    ePlayerStatsDBAction  playerDataAction(db);
    playerDataAction.Save();

    eChatBotStatsDBAction chatbotStatsAction(db);
    chatbotStatsAction.Save();

    sqlite3_close(db);

    if (se_playerTriggerMessages && se_playerMessageTriggersStatsSave)
        eChatBot::InitiateAction(nullptr, tString("$statssaved"), true);
}

void ePlayerStats::updateStatsMatchEnd(ePlayerNetID *matchWinner)
{
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
    {
        ePlayerNetID *currentPlayer = se_PlayerNetIDs[i];
        PlayerData &stats = getStats(currentPlayer);
        PlayerDataBase &statsThisSession = stats.thisSession();

        stats.in_game = currentPlayer->CurrentTeam() != nullptr;
        statsThisSession.in_game = stats.in_game;

        if (stats.in_game)
        {
            if (currentPlayer == matchWinner)
            {
                stats.match_wins++;
                statsThisSession.match_wins++;
            }
            else
            {
                stats.match_losses++;
                statsThisSession.match_losses++;
            }
            stats.matches_played++;
            statsThisSession.matches_played++;
        }
    }
}

void ePlayerStats::updateStatsRoundEnd()
{
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
    {
        ePlayerNetID *currentPlayer = se_PlayerNetIDs[i];
        PlayerData &stats = getStats(currentPlayer);
        PlayerDataBase &statsThisSession = stats.thisSession();

        stats.in_game = currentPlayer->CurrentTeam() != nullptr;
        statsThisSession.in_game = stats.in_game;

        if (stats.in_game)
        {
            gCycle *cycle = ePlayerNetID::NetPlayerToCycle(currentPlayer);

            if (roundWinnerProcessed)
            {
                if (cycle && cycle->Alive())
                {
                    stats.round_wins++;
                    statsThisSession.round_wins++;
                }
                else
                {
                    stats.round_losses++;
                    statsThisSession.round_losses++;
                }
                stats.rounds_played++;
                statsThisSession.rounds_played++;
            }

            stats.total_play_time += se_GameTime();
            statsThisSession.total_play_time += se_GameTime();
        }
        else
        {
            stats.total_spec_time += se_GameTime();
            statsThisSession.total_spec_time += se_GameTime();
        }
    }
}

void ePlayerStats::updateStatsRoundStart()
{
    for (int i = se_PlayerNetIDs.Len() - 1; i >= 0; --i)
    {
        ePlayerNetID *currentPlayer = se_PlayerNetIDs[i];
        PlayerData &stats = getStats(currentPlayer);
        PlayerDataBase &statsThisSession = stats.thisSession();

        stats.in_game = currentPlayer->CurrentTeam() != nullptr;
        statsThisSession.in_game = stats.in_game;

        if (stats.in_game)
        {
            stats.alive = true;
            statsThisSession.alive = true;
            stats.banned_a_player_this_round = false;
        }
    }
}

void ePlayerStats::reloadStatsFromDB()
{
    saveStatsToDB();
    playerStatsMap.clear(); // clear the current stats
    loadStatsFromDB();      // reload stats from DB
}

std::unordered_map<tString, PlayerData> ePlayerStats::playerStatsMap;

const std::string DELIMITER = "-+H4CK3RM@N5-+";

std::string join(const std::vector<std::string> &vec, const std::string &delimiter)
{
    return std::accumulate(std::begin(vec), std::end(vec), std::string(),
                           [&](const std::string &ss, const std::string &s)
                           {
                               return ss.empty() ? s : ss + delimiter + s;
                           });
}


std::vector<std::string> deserializeVector(const std::string &str)
{
    std::vector<std::string> vec;

    if (str.find(DELIMITER) == std::string::npos)
    {
        if (!str.empty()) {
            vec.push_back(str);
        }
        return vec;
    }

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(DELIMITER, prev)) != std::string::npos)
    {
        std::string segment = str.substr(prev, pos - prev);
        if (!segment.empty())
        {
            vec.push_back(segment);
        }
        prev = pos + DELIMITER.length();
    }

    std::string lastSegment = str.substr(prev);
    if (!lastSegment.empty()) {
        vec.push_back(lastSegment);
    }

    return vec;
}

std::string serializeVector(const std::vector<std::string> &vec)
{
    return std::accumulate(vec.begin(), vec.end(), std::string(),
                           [](const std::string &a, const std::string &b)
                           {
                               if (b.empty()) return a;
                               return a + (a.length() > 0 ? DELIMITER : "") + b;
                           });
}


const std::set<std::string> PlayerData::valueMapdisplayFields =
{
    "all", "rgb", "chats", "kills", "deaths", "match_wins",
    "match_losses", "round_wins", "round_losses", "rounds_played",
    "matches_played", "play_time", "spec_time", "times_joined",
    "kd", "chat_count", "fastest", "score", "seen", "hidden",
    "highest_kill_streak", "kill_streak", "kills_while_dead", "name_history",
    "nick", "bans", "bans_given"
};

void insertFunction(std::map<std::string, std::pair<std::string, PlayerDataBase::StatFunction>>& map,
                    const std::vector<std::string>& keys,
                    const std::string& label,
                    PlayerDataBase::StatFunction func)
{
    for (const auto& key : keys)
        map[key] = {label, func};
}

auto initValueMap = []() {
    std::map<std::string, std::pair<std::string, PlayerDataBase::StatFunction>> tempMap;

    insertFunction(tempMap,
    {"rgb"}, "RGB",
    [](PlayerDataBase *self)
    {
        return self->rgbString();
    });

    insertFunction(tempMap,
    {"r"}, "R",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->r;
        return result;
    });

    insertFunction(tempMap,
    {"g"}, "G",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->g;
        return result;
    });

    insertFunction(tempMap,
    {"b"}, "B",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->b;
        return result;
    });

    insertFunction(tempMap,
    {"chat_count","cc"}, "Chat Count",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->getMessageCount();
        return result;
    });

    insertFunction(tempMap,
    {"kills", "k"}, "Kills",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->kills;
        return result;
    });

    insertFunction(tempMap,
    {"deaths", "d"}, "Deaths",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->deaths;
        return result;
    });

    insertFunction(tempMap,
    {"match_wins", "mw"}, "Match Wins",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->match_wins;
        return result;
    });

    insertFunction(tempMap,
    {"match_losses", "ml"}, "Match Losses",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->match_losses;
        return result;
    });

    insertFunction(tempMap,
    {"round_wins", "rw"}, "Round Wins",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->round_wins;
        return result;
    });

    insertFunction(tempMap,
    {"round_losses", "rl"}, "Round Losses",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->round_losses;
        return result;
    });

    insertFunction(tempMap,
    {"rounds_played", "rp"}, "Rounds Played",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->rounds_played;
        return result;
    });

    insertFunction(tempMap,
    {"matches_played", "mp"}, "Maches Played",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->matches_played;
        return result;
    });

    insertFunction(tempMap,
    {"play_time", "pt"}, "Play Time",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->getPlayTimeStr();
        return result;
    });

    insertFunction(tempMap,
    {"spec_time", "st"}, "Spectate Time",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->getSpecTimeStr();
        return result;
    });

    insertFunction(tempMap,
    {"times_joined", "tj"}, "Times Joined",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->times_joined;
        return result;
    });

    insertFunction(tempMap,
    {"kd"}, "Kill Death Ratio",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->getKDRatio();
        return result;
    });

    insertFunction(tempMap,
    {"chats", "c"}, "Chats",
    [](PlayerDataBase *self)
    {
        tString result("");
        result = tString(self->getChatMessages());
        return result;
    });

    insertFunction(tempMap,
    {"fastest", "f", "fastest_speed", "speed"}, "Fastest Speed",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->getSpeed();
        return result;
    });

    insertFunction(tempMap,
    {"total_score", "score"}, "Total Score",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->total_score;
        return result;
    });

    insertFunction(tempMap,
    {"last_seen", "seen"}, "Last Seen",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->getLastSeenAgoStr();
        return result;
    });

    insertFunction(tempMap,
    {"privated_messages", "privated", "private", "hide", "hidden"}, "Hidden Stats",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << tString(self->getHiddenStats());
        return result;
    });

    insertFunction(tempMap,
    {"current_kill_streak","kill_streak", "ks","cks"}, "Current Kill Streak",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->current_kill_streak;
        return result;
    });

    insertFunction(tempMap,
    {"highest_kill_streak","max_kill_streak", "mks", "hks"}, "Highest Kill Streak",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->max_kill_streak;
        return result;
    });

    insertFunction(tempMap,
    {"kills_while_dead","kwd","dead_kills","dk"}, "Kills While Dead",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << self->kills_while_dead;
        return result;
    });

    insertFunction(tempMap,
    {"alive","a"}, "Alive",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << (self->alive ? "Yes" : "No");
        return result;
    });

    insertFunction(tempMap,
    {"local","is_local"}, "Local",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << (self->is_local ? "Yes" : "No");
        return result;
    });

    insertFunction(tempMap,
    {"names", "name_history", "nh"}, "Name History",
    [](PlayerDataBase *self)
    {
        tString result("");
        result = tString(self->getNameHistory());
        return result;
    });

    insertFunction(tempMap,
    {"nick", "nicknames"}, "Nickname",
    [](PlayerDataBase *self)
    {
        tString result("");
        result = tString(self->nickname);
        return result;
    });

    insertFunction(tempMap,
    {"bans", "times_banned"}, "Times Banned",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << (self->times_banned);
        return result;
    });

    insertFunction(tempMap,
    {"bans_given"}, "Bans Given",
    [](PlayerDataBase *self)
    {
        tString result("");
        result << (self->bans_given);
        return result;
    });

    return tempMap;
};

std::map<std::string, std::pair<std::string, PlayerDataBase::StatFunction>> PlayerDataBase::valueMap = initValueMap();
std::string joinVector(const std::vector<std::string>& vec) 
{
    if (vec.empty()) return "";

    std::string result = vec[0];
    for (size_t i = 1; i < vec.size(); ++i) {
        result += ", " + vec[i];
    }
    return result;
}

const std::vector<PlayerDataColumnMapping> ePlayerStatsMappings =
    {
        {"name", "TEXT PRIMARY KEY",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         {
             sqlite3_bind_text(stmt, col++, stats.name.c_str(), -1, SQLITE_STATIC);
         },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         {
             stats.name = tString((char *)sqlite3_column_text(stmt, col++));
         }},

        {"matches_played", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.matches_played); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.matches_played = sqlite3_column_int(stmt, col++); }},

        {"rounds_played", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.rounds_played); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.rounds_played = sqlite3_column_int(stmt, col++); }},

        {"total_messages", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.total_messages); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.total_messages = sqlite3_column_int(stmt, col++); }},

        {"b", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.b); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.b = sqlite3_column_int(stmt, col++); }},

        {"g", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.g); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.g = sqlite3_column_int(stmt, col++); }},

        {"r", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.r); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.r = sqlite3_column_int(stmt, col++); }},

        {"round_losses", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.round_losses); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.round_losses = sqlite3_column_int(stmt, col++); }},

        {"round_wins", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.round_wins); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.round_wins = sqlite3_column_int(stmt, col++); }},

        {"total_play_time", "REAL",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_double(stmt, col++, stats.total_play_time); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.total_play_time = sqlite3_column_double(stmt, col++); }},

        {"match_losses", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.match_losses); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.match_losses = sqlite3_column_int(stmt, col++); }},

        {"deaths", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.deaths); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.deaths = sqlite3_column_int(stmt, col++); }},

        {"match_wins", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.match_wins); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.match_wins = sqlite3_column_int(stmt, col++); }},

        {"kills", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.kills); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.kills = sqlite3_column_int(stmt, col++); }},

        {"times_joined", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.times_joined); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.times_joined = sqlite3_column_int(stmt, col++); }},

        {"total_spec_time", "REAL",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_double(stmt, col++, stats.total_spec_time); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.total_spec_time = sqlite3_column_double(stmt, col++); }},

        {"chat_messages", "TEXT",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         {
             std::string serializedData = serializeVector(stats.chat_messages);
             sqlite3_bind_text(stmt, col++, serializedData.c_str(), -1, SQLITE_TRANSIENT);
         },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         {
             const char *chatSerialized = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col++));
             if (chatSerialized)
             {
                 stats.chat_messages = deserializeVector(chatSerialized);
             }
         }},

        {"fastest_speed", "REAL",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_double(stmt, col++, stats.fastest_speed); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.fastest_speed = sqlite3_column_double(stmt, col++); }},

        {"last_seen", "BIGINT",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int64(stmt, col++, static_cast<sqlite3_int64>(stats.last_seen)); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.last_seen = static_cast<time_t>(sqlite3_column_int64(stmt, col++)); }},

        {"human", "BOOLEAN",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.human ? 1 : 0); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.human = sqlite3_column_int(stmt, col++) != 0; }},

        {"total_score", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.total_score); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.total_score = sqlite3_column_int(stmt, col++); }},

        {"privated_stats", "TEXT",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_text(stmt, col++, serializeVector(stats.privated_stats).c_str(), -1, SQLITE_STATIC); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         {
             const char *privatedStats = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col++));
             if (privatedStats)
                 stats.privated_stats = deserializeVector(privatedStats);
         }},

        {"current_kill_streak", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.current_kill_streak); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.current_kill_streak = sqlite3_column_int(stmt, col++); }},

        {"max_kill_streak", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.max_kill_streak); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.max_kill_streak = sqlite3_column_int(stmt, col++); }},

        {"kills_while_dead", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.kills_while_dead); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.kills_while_dead = sqlite3_column_int(stmt, col++); }},

        {"is_local", "BOOLEAN",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.is_local ? 1 : 0); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.is_local = sqlite3_column_int(stmt, col++) != 0; }},

        {"name_history", "TEXT",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         {
             std::string serializedData = serializeVector(stats.name_history);
             sqlite3_bind_text(stmt, col++, serializedData.c_str(), -1, SQLITE_TRANSIENT);
         },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         {
             const char *nameHistorySerialized = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col++));
             if (nameHistorySerialized)
             {
                 stats.name_history = deserializeVector(nameHistorySerialized);
             }
         }},

        {"nickname", "TEXT",
        [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
        {
            const char* nicknameCStr = stats.nickname.c_str();
            sqlite3_bind_text(stmt, col++, nicknameCStr, -1, SQLITE_TRANSIENT);
        },
        [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
        {
            const unsigned char* text = sqlite3_column_text(stmt, col++);
            if (text != nullptr) {
                stats.nickname = reinterpret_cast<const char*>(text);
            }
        }},

        {"times_banned", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.times_banned); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.times_banned = sqlite3_column_int(stmt, col++); }},

        {"bans_given", "INTEGER",
         [](sqlite3_stmt *stmt, int &col, const PlayerData &stats)
         { sqlite3_bind_int(stmt, col++, stats.bans_given); },
         [](sqlite3_stmt *stmt, int &col, PlayerData &stats)
         { stats.bans_given = sqlite3_column_int(stmt, col++); }},
};

static void se_playerStatsConsolidate(std::istream &s)
{
    if (!se_playerStats)
    {
        con << tThemedTextBase.LabelText("Player Stats")
            << tThemedTextBase.ErrorColor()
            << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return;
    }

    tString params;
    params.ReadLine(s, true);

    con << ePlayerStats::consolidatePlayerStats(params.ToLower().TrimWhitespace());
}

static tConfItemFunc se_playerStatsConsolidate_conf = HelperCommand::tConfItemFunc("PLAYER_STATS_CONSOLIDATE", &se_playerStatsConsolidate);

static void se_playerStatsDelete(std::istream &s)
{
    if (!se_playerStats)
    {
        con << tThemedTextBase.LabelText("Player Stats")
            << tThemedTextBase.ErrorColor()
            << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return;
    }

    tString params;
    params.ReadLine(s, true);

    con << ePlayerStats::deletePlayerStats(params.ToLower().TrimWhitespace());
}
static tConfItemFunc se_playerStatsDelete_conf = HelperCommand::tConfItemFunc("PLAYER_STATS_DELETE", &se_playerStatsDelete);

static void se_playerStatsReload(std::istream &s)
{
    if (!se_playerStats)
    {
        con << tThemedTextBase.LabelText("Player Stats")
            << tThemedTextBase.ErrorColor()
            << "PlayerStats not initialized. PlayerStats are disabled!\n";
        return;
    }

    con << tThemedTextBase.LabelText("Player Stats")
        << tThemedTextBase.MainColor()
        << "Reloading stats..";

    ePlayerStats::reloadStatsFromDB();
}
static tConfItemFunc se_playerStatsReload_conf = HelperCommand::tConfItemFunc("PLAYER_STATS_RELOAD", &se_playerStatsReload);

CommandState ePlayerStats::deleteState;
CommandState ePlayerStats::consolidateState;