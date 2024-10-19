#ifndef ArmageTron_TDATABASE_H
#define ArmageTron_TDATABASE_H

#include "sqlite3.h"
#include "../tron/gHelper/gHelperUtilities.h"

namespace tDatabaseUtility
{
    static void DebugLog(const std::string &table, const std::string &message, bool utility = false);
}

template <typename T>
struct ColumnMapping
{
    std::string columnName;
    std::string columnType;
    std::function<void(sqlite3_stmt *, int &, const T &)> bindFunc;
    std::function<void(sqlite3_stmt *, int &, T &)> extractFunc;
};


template <typename T>
class TableDefinition {
public:
    struct PrimaryKey
    {
        std::string columnName;
        std::function<std::string(const T &)> getValueFunc;

        PrimaryKey() {}

        virtual ~PrimaryKey() {}

        std::string getPrimaryColumn() const { return columnName; }
        virtual std::string getPrimaryKeyValue(const T &object) const 
        {
            return getValueFunc ? getValueFunc(object) : "";
        }
    };

    std::vector<ColumnMapping<T>> columnMappings;
    PrimaryKey primaryKey;

    TableDefinition(const PrimaryKey& pk, const std::vector<ColumnMapping<T>>& mappings)
        : primaryKey(pk), columnMappings(mappings) {}
};

template <typename T, typename TD>
class tDatabase
{
protected:
    sqlite3 *db;
    std::string tableName;
    std::vector<T> objectsToDelete;
    TD tableDefinition;

public:
    tDatabase(sqlite3* db, const std::string& tableName, const TD& tableDef)
        : db(db), tableName(tableName), tableDefinition(tableDef) {}

    virtual T &getTargetObject(const tString &name) = 0; // specific fetch logic

    virtual std::vector<T> getAllObjects() = 0; // all objects to save

    void markForDeletion(const T &object)
    {
        objectsToDelete.push_back(object);
    }

    void DeleteBatch(const std::vector<T> &objects)
    {
        if (objects.empty())
        {
            tDatabaseUtility::DebugLog(tableName, "No objects to delete in batch.");
            return;
        }

        std::string idColumn = tableDefinition.primaryKey.getPrimaryColumn();

        std::stringstream deleteSql;
        deleteSql << "DELETE FROM " << tableName << " WHERE " << idColumn << " IN (";

        for (size_t i = 0; i < objects.size(); ++i)
        {
            deleteSql << "?";
            if (i < objects.size() - 1)
                deleteSql << ",";
        }
        deleteSql << ");";

        tDatabaseUtility::DebugLog(tableName, "Preparing batch delete SQL: " + deleteSql.str());

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, deleteSql.str().c_str(), -1, &stmt, 0);

        if (rc == SQLITE_OK)
        {
            int bindIndex = 1;
            for (const auto &obj : objects)
            {
                std::string identifier = tableDefinition.primaryKey.getPrimaryKeyValue(obj);
                sqlite3_bind_text(stmt, bindIndex++, identifier.c_str(), -1, SQLITE_STATIC);
            }

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                tDatabaseUtility::DebugLog(tableName, "Failed to delete batch from " + tableName + ". SQLite error code: " + std::to_string(rc) + ", Error message: " + sqlite3_errmsg(db));
            }
            else
            {
                tDatabaseUtility::DebugLog(tableName, "Successfully deleted batch from " + tableName);
            }

            sqlite3_finalize(stmt);
        }
        else
        {
            tDatabaseUtility::DebugLog(tableName, "Failed to prepare SQL delete statement for batch. SQL: " + deleteSql.str() + " SQLite error code: " + std::to_string(rc) + ", Error message: " + sqlite3_errmsg(db));
        }
    }

    void deleteMarkedObjects(std::vector<T> &allObjects)
    {
        if (!objectsToDelete.empty())
        {
            tDatabaseUtility::DebugLog(tableName, "Deleting marked objects.");

            DeleteBatch(objectsToDelete);

            for (const auto &objToDelete : objectsToDelete)
            {
                auto it = std::find_if(allObjects.begin(), allObjects.end(),
                                       [&objToDelete, this](const T &existingObj)
                                       {
                                           return tableDefinition.primaryKey.getPrimaryKeyValue(objToDelete) == tableDefinition.primaryKey.getPrimaryKeyValue(existingObj); // Dynamic comparison
                                       });

                if (it != allObjects.end())
                {
                    allObjects.erase(it);
                    tDatabaseUtility::DebugLog(tableName, "Deleted object: " + tableDefinition.primaryKey.getPrimaryKeyValue(objToDelete));
                }
            }

            objectsToDelete.clear();
            tDatabaseUtility::DebugLog(tableName, "Cleared objects marked for deletion.");
        }
        else
            tDatabaseUtility::DebugLog(tableName, "No objects marked for deletion.");
    }

    void ensureTableAndColumnsExist()
    {
        char *errMsg = 0;
        int rc;

        std::stringstream checkTableSql;
        checkTableSql << "SELECT name FROM sqlite_master WHERE type='table' AND name='" << tableName << "';";
        tDatabaseUtility::DebugLog(tableName, "Checking if table exists with SQL: " + checkTableSql.str());

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, checkTableSql.str().c_str(), -1, &stmt, NULL);

        if (rc != SQLITE_OK)
        {
            tDatabaseUtility::DebugLog(tableName, "sqlite3_prepare_v2 failed with code: " + std::to_string(rc));
            return;
        }

        bool tableExists = false;
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            tDatabaseUtility::DebugLog(tableName, "Table " + tableName + " exists.");
            tableExists = true;
        }
        sqlite3_finalize(stmt);

        if (!tableExists)
        {
            std::stringstream createTableSql;
            createTableSql << "CREATE TABLE " << tableName << "(";
            bool first = true;
            for (const auto &mapping : tableDefinition.columnMappings)
            {
                if (!first)
                    createTableSql << ", ";
                createTableSql << mapping.columnName << " " << mapping.columnType;
                first = false;
            }
            createTableSql << ");";

            tDatabaseUtility::DebugLog(tableName, "Creating table with SQL: " + createTableSql.str());

            rc = sqlite3_exec(db, createTableSql.str().c_str(), 0, 0, &errMsg);
            if (rc != SQLITE_OK)
            {
                tDatabaseUtility::DebugLog(tableName, "SQL error during table creation: " + std::string(errMsg));
                sqlite3_free(errMsg);
            }
            else
            {
                tDatabaseUtility::DebugLog(tableName, "Table " + tableName + " created successfully.");
            }
        }
        else
        {
            std::unordered_set<std::string> existingColumns;
            std::stringstream tableInfoSql;
            tableInfoSql << "PRAGMA table_info(" << tableName << ");";
            tDatabaseUtility::DebugLog(tableName, "Retrieving table info with SQL: " + tableInfoSql.str());

            rc = sqlite3_prepare_v2(db, tableInfoSql.str().c_str(), -1, &stmt, NULL);

            if (rc == SQLITE_OK)
            {
                while (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    std::string columnName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                    existingColumns.insert(columnName);
                }
            }
            else
            {
                tDatabaseUtility::DebugLog(tableName, "Failed to retrieve table info with code: " + std::to_string(rc));
            }
            sqlite3_finalize(stmt);

            bool allColumnsExist = true;
            bool error = false;
            for (const auto &mapping : tableDefinition.columnMappings)
            {
                if (existingColumns.find(mapping.columnName) == existingColumns.end())
                {
                    allColumnsExist = false;
                    std::stringstream addColumnSql;
                    addColumnSql << "ALTER TABLE " << tableName << " ADD COLUMN " << mapping.columnName << " " << mapping.columnType << ";";
                    tDatabaseUtility::DebugLog(tableName, "Column " + mapping.columnName + " is missing, adding with SQL: " + addColumnSql.str());

                    rc = sqlite3_exec(db, addColumnSql.str().c_str(), 0, 0, &errMsg);
                    if (rc != SQLITE_OK)
                    {
                        error = true;
                        tDatabaseUtility::DebugLog(tableName, "SQL error during column addition: " + std::string(errMsg));
                        sqlite3_free(errMsg);
                    }
                }
            }
            if (allColumnsExist)
                tDatabaseUtility::DebugLog(tableName, "All required columns exist in table " + tableName + ".");
            else if (!error)
                tDatabaseUtility::DebugLog(tableName, "Missing columns were added to table " + tableName + ".");
            else
                tDatabaseUtility::DebugLog(tableName, "Error adding required columns to table " + tableName + ".");
        }
    }

    void Load()
    {
        ensureTableAndColumnsExist();

        std::stringstream selectSql;
        selectSql << "SELECT ";
        for (const auto &mapping : tableDefinition.columnMappings)
            selectSql << mapping.columnName << ",";

        std::string query = selectSql.str();
        query.pop_back();
        query += " FROM " + tableName + ";";

        tDatabaseUtility::DebugLog(tableName, "Loading using SQL: " + query);

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
        if (rc == SQLITE_OK)
        {
            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                tString name = tString((char *)sqlite3_column_text(stmt, 0));

                T &targetObject = getTargetObject(name);

                int column = 0;
                for (const auto &mapping : tableDefinition.columnMappings)
                    mapping.extractFunc(stmt, column, targetObject);

                postLoadActions(targetObject);
            }
        }
        else
        {
            tDatabaseUtility::DebugLog(tableName, "Failed to prepare SQL statement: " + query);
        }
        sqlite3_finalize(stmt);
    }

    void Save()
    {
        ensureTableAndColumnsExist();

        auto objectsToSave = getAllObjects();

        for (auto &obj : objectsToSave)
            preSaveAction(obj);

        // DELETE ITEMS
        deleteMarkedObjects(objectsToSave);

        // INSERT OR REPLACE
        std::stringstream insertSql;
        insertSql << "INSERT OR REPLACE INTO " << tableName << " (";
        for (const auto &mapping : tableDefinition.columnMappings)
            insertSql << mapping.columnName << ",";
        insertSql.seekp(-1, insertSql.cur);
        insertSql << ") VALUES (";

        // placeholders
        for (size_t i = 0; i < tableDefinition.columnMappings.size(); ++i)
        {
            insertSql << "?";
            if (i < tableDefinition.columnMappings.size() - 1)
                insertSql << ",";
        }
        insertSql << ");";

        tDatabaseUtility::DebugLog(tableName, "Saving using SQL: " + insertSql.str());

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, insertSql.str().c_str(), -1, &stmt, 0);
        if (rc == SQLITE_OK)
        {
            for (const auto &obj : objectsToSave)
            {
                int column = 1;
                for (const auto &mapping : tableDefinition.columnMappings)
                    mapping.bindFunc(stmt, column, obj);

                rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    tDatabaseUtility::DebugLog(tableName, "Failed to save object to DB.");

                sqlite3_reset(stmt); // reset the prepared statement
            }
        }
        else
        {
            tDatabaseUtility::DebugLog(tableName, "Failed to prepare SQL statement: " + insertSql.str());
        }
        sqlite3_finalize(stmt);
    }

    void Delete(const T &object)
    {
        ensureTableAndColumnsExist();

        std::string idColumn = tableDefinition.primaryKey.getPrimaryColumn();
        std::string identifier = tableDefinition.primaryKey.getPrimaryKeyValue(object);

        std::stringstream deleteSql;
        deleteSql << "DELETE FROM " << tableName << " WHERE " << idColumn << " = ?;";

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, deleteSql.str().c_str(), -1, &stmt, 0);

        if (rc == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, identifier.c_str(), -1, SQLITE_STATIC);

            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE)
            {
                tDatabaseUtility::DebugLog(tableName, "Failed to delete " + identifier + " from " + tableName + ". SQLite error code: " + std::to_string(rc));
            }
            else
            {
                tDatabaseUtility::DebugLog(tableName, "Successfully deleted " + identifier + " from " + tableName);
            }

            sqlite3_finalize(stmt);
        }
        else
        {
            tDatabaseUtility::DebugLog(tableName, "Failed to prepare SQL delete statement for " + identifier + ". SQL: " + deleteSql.str() + " SQLite error code: " + std::to_string(rc));
        }
    }

    virtual void preSaveAction(T &obj) {}
    virtual void postLoadActions(T &obj) {}
};

namespace tDatabaseUtility
{

    static void DebugLog(const std::string &table, const std::string &message, bool utility)
    {
        std::string sender = utility ? "tDatabaseUtility" : ("tDatabase (" + table + ")");
        gHelperUtility::DebugLog(message, sender);
    }

    static sqlite3* OpenDatabase(tString databaseFile)
    {

        tString databasePath = tDirectories::Var().GetReadPath(databaseFile);

        if (databasePath.empty())
        {
            DebugLog("", "GetReadPath() returned an empty path, attempting to create file in Var path", true);
            databasePath = tDirectories::Var().GetWritePath(databaseFile);
            
            if (databasePath.empty())
            {
                DebugLog("", "Failed to obtain a valid path to create database in Var directory.", true);
                return nullptr;
            }
        }

        DebugLog("", "Opening database at '" + databasePath.stdString() + "'", true);

        sqlite3 *db;
        int rc = sqlite3_open(databasePath.stdString().c_str(), &db);

        if (rc != SQLITE_OK)
        {
            DebugLog("", "Cannot open database: " + std::string(sqlite3_errmsg(db)) + "\n", true);
            sqlite3_close(db);
            return nullptr;
        }

        DebugLog("", "Database successfully opened or created at '" + databasePath.stdString() + "'", true);
        return db;
    }


}

#endif
