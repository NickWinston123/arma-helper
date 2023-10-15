#ifndef ArmageTron_TDATABASE_H
#define ArmageTron_TDATABASE_H

#include "sqlite3.h"
#include "../tron/gHelper/gHelperUtilities.h"

template <typename T>
struct ColumnMapping 
{
    std::string columnName;
    std::string columnType;
    std::function<void(sqlite3_stmt*, int&, const T&)> bindFunc;
    std::function<void(sqlite3_stmt*, int&, T&)> extractFunc;
};

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

        std::stringstream checkTableSql;
        checkTableSql << "SELECT name FROM sqlite_master WHERE type='table' AND name='" << tableName << "';";

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, checkTableSql.str().c_str(), -1, &stmt, NULL);

        bool tableExists = false;

        if (rc == SQLITE_OK)
        {
            if (sqlite3_step(stmt) == SQLITE_ROW)
                tableExists = true;
            sqlite3_finalize(stmt);
        }

        // table exists ?
        if (!tableExists)
        {
            std::stringstream createTableSql;
            createTableSql << "CREATE TABLE " << tableName << "(";
            bool first = true;
            for (const auto &mapping : mappings)
            {
                if (!first)
                    createTableSql << ", ";
                createTableSql << mapping.columnName << " " << mapping.columnType;
                first = false;
            }
            createTableSql << ");";

            rc = sqlite3_exec(db, createTableSql.str().c_str(), 0, 0, &errMsg);
            if (rc != SQLITE_OK)
            {
                gHelperUtility::DebugLog("SQL error during table creation: " + std::string(errMsg));
                sqlite3_free(errMsg);
            }
        }
        else
        {
            for (const auto &mapping : mappings)
            {
                std::stringstream checkColumnSql;
                checkColumnSql << "PRAGMA table_info(" << tableName << ");";
                rc = sqlite3_prepare_v2(db, checkColumnSql.str().c_str(), -1, &stmt, NULL);

                bool columnExists = false;

                if (rc == SQLITE_OK)
                {
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        std::string columnName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                        if (columnName == mapping.columnName)
                        {
                            columnExists = true;
                            break;
                        }
                    }
                    sqlite3_finalize(stmt);
                }

                if (!columnExists)
                {
                    std::stringstream addColumnSql;
                    addColumnSql << "ALTER TABLE " << tableName << " ADD COLUMN " << mapping.columnName << " " << mapping.columnType << ";";
                    rc = sqlite3_exec(db, addColumnSql.str().c_str(), 0, 0, &errMsg);
                    if (rc != SQLITE_OK)
                    {
                        gHelperUtility::DebugLog("SQL error during column addition: " + std::string(errMsg));
                        sqlite3_free(errMsg);
                    }
                }
            }
        }
    }

    void Load()
    {
        ensureTableAndColumnsExist();

        std::stringstream selectSql;
        selectSql << "SELECT ";
        for (const auto &mapping : mappings)
            selectSql << mapping.columnName << ",";

        std::string query = selectSql.str();
        query.pop_back();
        query += " FROM " + tableName + ";";

        gHelperUtility::DebugLog("Loading using SQL: " + query);

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
        if (rc == SQLITE_OK)
        {
            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                tString name = tString((char *)sqlite3_column_text(stmt, 0));

                T &targetObject = getTargetObject(name);

                int column = 0;
                for (const auto &mapping : mappings)
                    mapping.extractFunc(stmt, column, targetObject);
                
                postLoadActions(targetObject);
            }
        }
        else
        {
            gHelperUtility::DebugLog("Failed to prepare SQL statement: " + query);
        }
        sqlite3_finalize(stmt);
    }

void Save()
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

    gHelperUtility::DebugLog("Saving using SQL: " + insertSql.str());

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, insertSql.str().c_str(), -1, &stmt, 0);
    if (rc == SQLITE_OK)
    {
        for (const auto &obj : getAllObjects())
        {
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

namespace tDatabaseUtility
{
    static sqlite3 *OpenDatabase(tString databaseFile)
    {
        tString databasePath = tDirectories::Var().GetReadPath(databaseFile);

        gHelperUtility::DebugLog(st_GetCurrentTime("[%Y/%m/%d-%H:%M:%S] ").stdString() + "Opening database at '" + databasePath.stdString() + "'");

        sqlite3 *db;
        int rc = sqlite3_open(databasePath, &db);
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