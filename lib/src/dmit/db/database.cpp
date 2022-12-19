#include "dmit/db/database.hpp"

#include "dmit/db/query_register.hpp"
#include "dmit/db/connection.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/storage.hpp"

extern "C"
{
    #include "sqlite3/sqlite3.h"
}

#include <filesystem>
#include <optional>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace dmit::db
{

namespace
{

int executeStatement(sqlite3_stmt* statement)
{
    int resultCode = sqlite3_step  (statement);
    int  errorCode = sqlite3_reset (statement);

    return (errorCode != SQLITE_OK) ? errorCode
                                    : resultCode;
}

} // namespace

Database::Database(int& errorCode) :
    _connection{errorCode},
    _queryRegister{_connection, errorCode}
{}

std::optional<nng::Buffer> Database::asNngBuffer()
{
    return _connection.asNngBuffer();
}

int Database::transactionBegin()
{
    auto queryTransactionBegin = _queryRegister[QueryRegister::TRANSACTION_BEGIN];

    int resultCode;

    if ((resultCode = executeStatement(queryTransactionBegin)) != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::transactionRollback()
{
    auto queryTransactionRollback = _queryRegister[QueryRegister::TRANSACTION_ROLLBACK];

    int resultCode;

    if ((resultCode = executeStatement(queryTransactionRollback)) != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::transactionCommit()
{
    auto queryTransactionCommit = _queryRegister[QueryRegister::TRANSACTION_COMMIT];

    int resultCode;

    if ((resultCode = executeStatement(queryTransactionCommit)) != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::clean()
{
    auto query = _queryRegister[QueryRegister::CLEAN];

    int resultCode;

    if ((resultCode = executeStatement(query)) != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::hasFile(const com::UniqueId& fileId, bool& result)
{
    auto query = _queryRegister[QueryRegister::SELECT_FILE];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::FILE_ID,
                                       &fileId,
                                       sizeof(decltype(fileId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(query);

    if (resultCode != SQLITE_DONE &&
        resultCode != SQLITE_ROW)
    {
        return resultCode;
    }

    result = (resultCode != SQLITE_DONE);

    return SQLITE_OK;
}

int Database::hasUnit(const com::UniqueId& unitId, bool& result)
{
    auto query = _queryRegister[QueryRegister::SELECT_UNIT];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(query);

    if (resultCode != SQLITE_DONE &&
        resultCode != SQLITE_ROW)
    {
        return resultCode;
    }

    result = (resultCode != SQLITE_DONE);

    return SQLITE_OK;
}

int Database::updateFileAndInsertUnit(const com::UniqueId          & fileId,
                                      const com::UniqueId          & unitId,
                                      const com::TStorage<uint8_t> & unitSource)
{
    int errorCodeRollback;
    int errorCode;

    // 1. Begin transaction

    if ((errorCode = transactionBegin()) != SQLITE_OK)
    {
        return errorCode;
    }

    // 2. Update file

    if ((errorCode = updateFile(fileId, unitId)) != SQLITE_OK)
    {
        if ((errorCodeRollback = transactionRollback()) != SQLITE_OK)
        {
            return errorCodeRollback;
        }

        return errorCode;
    }

    // 3. Insert unit

    if ((errorCode = insertUnit(unitId, unitSource)) != SQLITE_OK)
    {
        if ((errorCodeRollback = transactionRollback()) != SQLITE_OK)
        {
            return errorCodeRollback;
        }

        return errorCode;
    }

    // 4. Commit transaction

    if ((errorCode = transactionCommit()) != SQLITE_OK)
    {
        return errorCode;
    }

    return SQLITE_OK;
}

int Database::insertFileAndUnit(const com::UniqueId          & fileId,
                                const com::UniqueId          & unitId,
                                const std::string            & filePath,
                                const com::TStorage<uint8_t> & unitSource)
{
    int errorCodeRollback;
    int errorCode;

    // 1. Begin transaction

    if ((errorCode = transactionBegin()) != SQLITE_OK)
    {
        return errorCode;
    }

    // 2. Insert file

    if ((errorCode = insertFile(fileId, unitId, filePath)) != SQLITE_OK)
    {
        if ((errorCodeRollback = transactionRollback()) != SQLITE_OK)
        {
            return errorCodeRollback;
        }

        return errorCode;
    }

    // 3. Insert unit

    if ((errorCode = insertUnit(unitId, unitSource)) != SQLITE_OK)
    {
        if ((errorCodeRollback = transactionRollback()) != SQLITE_OK)
        {
            return errorCodeRollback;
        }

        return errorCode;
    }

    // 4. Commit transaction

    if ((errorCode = transactionCommit()) != SQLITE_OK)
    {
        return errorCode;
    }

    return SQLITE_OK;
}

int Database::insertFile(const com::UniqueId & fileId,
                         const com::UniqueId & unitId,
                         const std::string   & filePath)
{
    auto query = _queryRegister[QueryRegister::INSERT_FILE];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::FILE_ID,
                                       &fileId,
                                       sizeof(decltype(fileId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_text(query,
                                       QueryRegister::FILE_PATH,
                                       filePath.data(),
                                       filePath.size(),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(query);

    if (resultCode != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::updateFile(const com::UniqueId & fileId,
                         const com::UniqueId & unitId)
{
    auto query = _queryRegister[QueryRegister::UPDATE_FILE];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::FILE_ID,
                                       &fileId,
                                       sizeof(decltype(fileId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(query);

    if (resultCode != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::insertUnit(const com::UniqueId          & unitId,
                         const com::TStorage<uint8_t> & unitSource)
{
    auto query = _queryRegister[QueryRegister::INSERT_UNIT];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(query,
                                       QueryRegister::UNIT_SOURCE,
                                       unitSource.data(),
                                       unitSource._size,
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(query);

    if (resultCode != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::selectUnitIdsPathsSources(std::vector<com::UniqueId          >& unitIds,
                                        std::vector<std::filesystem::path  >& paths,
                                        std::vector<com::TStorage<uint8_t >>& sources)
{
    auto query =
        _queryRegister[QueryRegister::K_QUERY_SELECT_UNIT_IDS_PATHS_SOURCES];

    int resultCode = sqlite3_step(query);

    while (resultCode != SQLITE_DONE)
    {
        if (resultCode != SQLITE_ROW)
        {
            return resultCode;
        }

        // 1. Unit ID

        const void* unitIdAsVoidStar = sqlite3_column_blob(query, 0 /*unit_id*/);

        unitIds.emplace_back();

        memcpy(&(unitIds.back()), unitIdAsVoidStar, sizeof(com::UniqueId));

        // 2. Path

        const uint8_t* pathAsBytes = sqlite3_column_text  (query, 1 /*path*/);
        int            pathSize    = sqlite3_column_bytes (query, 1 /*path*/);

        paths.emplace_back(reinterpret_cast<const char*>(pathAsBytes),
                           reinterpret_cast<const char*>(pathAsBytes) + pathSize);
        // 3. Source

        const void* sourceAsVoidStar = sqlite3_column_blob  (query, 2 /*source*/);
        int         sourceSize       = sqlite3_column_bytes (query, 2 /*source*/);

        sources.emplace_back(sourceSize);
        std::memcpy(sources.back().data(), sourceAsVoidStar, sourceSize);

        resultCode = sqlite3_step(query);
    }

    resultCode = sqlite3_reset(query);

    return resultCode;
}

} // namespace dmit::db
