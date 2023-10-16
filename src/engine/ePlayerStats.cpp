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

    con << eChatCommand::CommandText("Player Stats")
        << "Saving stats to database..\n";

    ePlayerStatsDBAction  playerDataAction(db);
    playerDataAction.Save();

    eChatBotStatsDBAction chatbotStatsAction(db);
    chatbotStatsAction.Save();

    sqlite3_close(db);

    if (se_playerTriggerMessages && se_playerMessageTriggersStatsSave)
        eChatBot::InitiateAction(nullptr, tString("$statssaved"), true);
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
            gCycle *cycle = ePlayerNetID::NetPlayerToCycle(currentPlayer);

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
        vec.push_back(str);
        return vec;
    }

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(DELIMITER, prev)) != std::string::npos)
    {
        vec.push_back(str.substr(prev, pos - prev));
        prev = pos + DELIMITER.length();
    }
    vec.push_back(str.substr(prev));
    return vec;
}

std::string serializeVector(const std::vector<std::string> &vec)
{
    return std::accumulate(vec.begin(), vec.end(), std::string(),
                           [](const std::string &a, const std::string &b)
                           {
                               return a + (a.length() > 0 ? DELIMITER : "") + b;
                           });
}


const std::set<std::string> PlayerData::valueMapdisplayFields =
{
    "all", "rgb", "chats", "kills", "deaths", "match_wins",
    "match_losses", "round_wins", "round_losses", "rounds_played",
    "matches_played", "play_time", "spec_time", "times_joined",
    "kd", "chat_count", "fastest", "score", "seen", "hidden"
};

void insertFunction(std::map<std::string, std::pair<std::string, PlayerData::StatFunction>>& map,
                    const std::vector<std::string>& keys,
                    const std::string& label,
                    PlayerData::StatFunction func)
{
    for (const auto& key : keys)
        map[key] = {label, func};
}

auto initValueMap = []() {
    std::map<std::string, std::pair<std::string, PlayerData::StatFunction>> tempMap;

    insertFunction(tempMap, {"rgb"}, "RGB", [](PlayerDataBase *self) {
        return self->rgbString();
    });

    insertFunction(tempMap, {"r"}, "R", [](PlayerDataBase *self) {
        tString result("");
        result << self->r;
        return result;
    });

    insertFunction(tempMap, {"g"}, "G", [](PlayerDataBase *self) {
        tString result("");
        result << self->g;
        return result;
    });

    insertFunction(tempMap, {"b"}, "B", [](PlayerDataBase *self) {
        tString result("");
        result << self->b;
        return result;
    });

    insertFunction(tempMap, {"chat_count","cc"}, "Chat Count", [](PlayerDataBase *self) {
        tString result("");
        result << self->total_messages;
        return result;
    });

    insertFunction(tempMap, {"kills", "k"}, "Kills", [](PlayerDataBase *self) {
        tString result("");
        result << self->kills;
        return result;
    });

    insertFunction(tempMap, {"deaths", "d"}, "Deaths", [](PlayerDataBase *self) {
        tString result("");
        result << self->deaths;
        return result;
    });

    insertFunction(tempMap, {"match_wins", "mw"}, "Match Wins", [](PlayerDataBase *self) {
        tString result("");
        result << self->match_wins;
        return result;
    });

    insertFunction(tempMap, {"match_losses", "ml"}, "Match Losses", [](PlayerDataBase *self) {
        tString result("");
        result << self->match_losses;
        return result;
    });

    insertFunction(tempMap, {"round_wins", "rw"}, "Round Wins", [](PlayerDataBase *self) {
        tString result("");
        result << self->round_wins;
        return result;
    });

    insertFunction(tempMap, {"round_losses", "rl"}, "Round Losses", [](PlayerDataBase *self) {
        tString result("");
        result << self->round_losses;
        return result;
    });

    insertFunction(tempMap, {"rounds_played", "rp"}, "Rounds Played", [](PlayerDataBase *self) {
        tString result("");
        result << self->rounds_played;
        return result;
    });

    insertFunction(tempMap, {"matches_played", "mp"}, "Maches Played", [](PlayerDataBase *self) {
        tString result("");
        result << self->matches_played;
        return result;
    });

    insertFunction(tempMap, {"play_time", "pt"}, "Play Time", [](PlayerDataBase *self) {
        tString result("");
        result << st_GetFormatTime(self->getTotalPlayTime(), false);
        return result;
    });

    insertFunction(tempMap, {"spec_time", "st"}, "Spec Time", [](PlayerDataBase *self) {
        tString result("");
        result << st_GetFormatTime(self->getTotalSpecTime(), false);
        return result;
    });

    insertFunction(tempMap, {"times_joined", "tj"}, "Times Joined", [](PlayerDataBase *self) {
        tString result("");
        result << self->times_joined;
        return result;
    });

    insertFunction(tempMap, {"kd"}, "Kill Death Ratio", [](PlayerDataBase *self) {
        tString result("");
        result << self->getKDRatio();
        return result;
    });

    insertFunction(tempMap, {"chats", "c"}, "Chats", [](PlayerDataBase *self) {
        tString result("");
        result = tString(self->getChatMessages());
        return result;
    });

    insertFunction(tempMap, {"fastest", "f", "fastest_speed", "speed"}, "Fastest Speed", [](PlayerDataBase *self) {
        tString result("");
        result << self->getSpeed();
        return result;
    });

    insertFunction(tempMap, {"total_score", "score"}, "Total Score", [](PlayerDataBase *self) {
        tString result("");
        result << self->total_score;
        return result;
    });

    insertFunction(tempMap, {"last_seen", "seen"}, "Last Seen", [](PlayerDataBase *self) {
        tString result("");
        result << self->getLastSeenAgoStr();
        return result;
    });

    insertFunction(tempMap, {"privated_messages", "privated", "private", "hide", "hidden"}, "Hidden Stats", [](PlayerDataBase *self) {
        tString result("");
        result << tString(self->getHiddenStats());
        return result;
    });

    return tempMap;
};
std::map<std::string, std::pair<std::string, PlayerData::StatFunction>> PlayerData::valueMap = initValueMap();

const std::vector<PlayerDataColumnMapping > ePlayerStatsMappings =
{
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

    {"privated_stats", "TEXT",
        [](sqlite3_stmt* stmt, int& col, const PlayerData& stats) { sqlite3_bind_text(stmt, col++, serializeVector(stats.privated_stats).c_str(), -1, SQLITE_STATIC); },
        [](sqlite3_stmt* stmt, int& col, PlayerData& stats) {
            const char *chatSerialized = reinterpret_cast<const char *>(sqlite3_column_text(stmt, col++));
            if (chatSerialized)
                stats.privated_stats = deserializeVector(chatSerialized);
        }
    },
};
