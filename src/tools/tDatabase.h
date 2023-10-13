#ifndef ArmageTron_TDATABASE_H
#define ArmageTron_TDATABASE_H

#include "sqlite3.h"
#include "../tron/gHelper/gHelperUtilities.h"

template <typename T, typename M>
class tDatabase
{
protected:
    sqlite3 *db;
    std::string tableName;
    const std::vector<M> &mappings;

public:
    tDatabase(sqlite3 *db, const std::string &tableName, const std::vector<M> &mappings)
        : db(db), tableName(tableName), mappings(mappings) {}

    virtual T &getTargetObject(const tString &name) = 0; // specific fetch logic

    virtual std::vector<T> getAllObjects() = 0; // all objects to save

    void ensureTableAndColumnsExist()
    {
        char *errMsg = 0;
        int rc;

        std::stringstream createSql;
        createSql << "CREATE TABLE IF NOT EXISTS " << tableName << "(";
        bool first = true;
        for (const auto &mapping : mappings)
        {
            if (!first)
            {
                createSql << ", ";
            }
            createSql << mapping.columnName << " " << mapping.columnType;
            first = false;
        }
        createSql << ");";

        gHelperUtility::DebugLog("Executing SQL: " + createSql.str());

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

    void loadStatsFromDB()
    {
        ensureTableAndColumnsExist();

        std::stringstream selectSql;
        selectSql << "SELECT ";
        for (const auto &mapping : mappings)
            selectSql << mapping.columnName << ",";

        std::string query = selectSql.str();
        query.pop_back();
        query += " FROM " + tableName + ";";

        gHelperUtility::DebugLog("Loading stats using SQL: " + query);

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
        if (rc == SQLITE_OK)
        {
            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                tString name = tString((char *)sqlite3_column_text(stmt, 0));

                gHelperUtility::DebugLog(std::string("name: ") + name.stdString());

                T &targetObject = getTargetObject(name);
                int column = 0;
                for (const auto &mapping : mappings)
                {
                    mapping.extractFunc(stmt, column, targetObject);
                }
                postLoadActions(targetObject);
            }
        }
        else
        {
            gHelperUtility::DebugLog("Failed to prepare SQL statement: " + query);
        }
        sqlite3_finalize(stmt);
    }

void saveStatsToDB()
{
    ensureTableAndColumnsExist();

    // INSERT OR REPLACE
    std::stringstream insertSql;
    insertSql << "INSERT OR REPLACE INTO " << tableName << " (";
    for (const auto &mapping : mappings)
    {
        insertSql << mapping.columnName << ",";
    }
    insertSql.seekp(-1, insertSql.cur); 
    insertSql << ") VALUES (";
    // placeholders
    for (size_t i = 0; i < mappings.size(); ++i)
    {
        insertSql << "?";
        if (i < mappings.size() - 1)
            insertSql << ",";
    }
    insertSql << ");";

    gHelperUtility::DebugLog("Saving stats using SQL: " + insertSql.str());

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, insertSql.str().c_str(), -1, &stmt, 0);
    if (rc == SQLITE_OK)
    {
        for (const auto &obj : getAllObjects())
        {

            gHelperUtility::DebugLog("GOT NAME FROM OBJECT: " + obj.name.stdString());
            int column = 1;
            for (const auto &mapping : mappings)
            {
                mapping.bindFunc(stmt, column, obj);
            }
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                gHelperUtility::DebugLog("Failed to save object to DB.");
            }
            sqlite3_reset(stmt); // reset the prepared statement
        }
    }
    else
    {
        gHelperUtility::DebugLog("Failed to prepare SQL statement: " + insertSql.str());
    }
    sqlite3_finalize(stmt);
}

    virtual void postLoadActions(T &obj) {}
};

namespace tDBUtility
{
    static sqlite3 *getDatabase(tString databaseFile)
    {

        gHelperUtility::DebugLog(st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ").stdString() + "Getting database");

        sqlite3 *db;
        int rc = sqlite3_open(tDirectories::Var().GetReadPath(databaseFile), &db);
        if (rc != SQLITE_OK)
        {

            gHelperUtility::DebugLog(std::string("Cannot open database: ") + sqlite3_errmsg(db) + "\n");
            sqlite3_close(db);
            return nullptr;
        }

        return db;
    }
}
#endif
