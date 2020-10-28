#include "dmit/db/database.hpp"

#include "dmit/db/query_register.hpp"
#include "dmit/db/connection.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/com/unique_id.hpp"

#include "sqlite3/sqlite3.h"

#include <optional>
#include <cstdint>
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
    auto queryClean = _queryRegister[QueryRegister::CLEAN];

    int resultCode;

    if ((resultCode = executeStatement(queryClean)) != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::hasFile(const com::UniqueId& fileId, bool& result)
{
    auto querySelectFile = _queryRegister[QueryRegister::SELECT_FILE];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(querySelectFile,
                                       QueryRegister::FILE_ID,
                                       &fileId,
                                       sizeof(decltype(fileId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(querySelectFile);

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
    auto querySelectUnit = _queryRegister[QueryRegister::SELECT_UNIT];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(querySelectUnit,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(querySelectUnit);

    if (resultCode != SQLITE_DONE &&
        resultCode != SQLITE_ROW)
    {
        return resultCode;
    }

    result = (resultCode != SQLITE_DONE);

    return SQLITE_OK;
}

int Database::updateFileWithUnit(const com::UniqueId        & fileId,
                                 const com::UniqueId        & unitId,
                                 const std::vector<uint8_t> & unitSource)
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

    if ((errorCode = insertUnit(unitId, fileId, unitSource)) != SQLITE_OK)
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

int Database::insertFileWithUnit(const com::UniqueId        & fileId,
                                 const com::UniqueId        & unitId,
                                 const std::string          & filePath,
                                 const std::vector<uint8_t> & unitSource)
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

    if ((errorCode = insertUnit(unitId, fileId, unitSource)) != SQLITE_OK)
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
    auto queryInsertFile = _queryRegister[QueryRegister::INSERT_FILE];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(queryInsertFile,
                                       QueryRegister::FILE_ID,
                                       &fileId,
                                       sizeof(decltype(fileId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(queryInsertFile,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_text(queryInsertFile,
                                       QueryRegister::FILE_PATH,
                                       filePath.data(),
                                       filePath.size(),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(queryInsertFile);

    if (resultCode != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::updateFile(const com::UniqueId & fileId,
                         const com::UniqueId & unitId)
{
    auto queryUpdateFile = _queryRegister[QueryRegister::UPDATE_FILE];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(queryUpdateFile,
                                       QueryRegister::FILE_ID,
                                       &fileId,
                                       sizeof(decltype(fileId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(queryUpdateFile,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(queryUpdateFile);

    if (resultCode != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

int Database::insertUnit(const com::UniqueId        & unitId,
                         const com::UniqueId        & fileId,
                         const std::vector<uint8_t> & unitSource)
{
    auto queryInsertUnit = _queryRegister[QueryRegister::INSERT_UNIT];

    int errorCode;

    if ((errorCode = sqlite3_bind_blob(queryInsertUnit,
                                       QueryRegister::UNIT_ID,
                                       &unitId,
                                       sizeof(decltype(unitId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(queryInsertUnit,
                                       QueryRegister::FILE_ID,
                                       &fileId,
                                       sizeof(decltype(fileId)),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(queryInsertUnit,
                                       QueryRegister::UNIT_SOURCE,
                                       unitSource.data(),
                                       unitSource.size(),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int resultCode = executeStatement(queryInsertUnit);

    if (resultCode != SQLITE_DONE)
    {
        return resultCode;
    }

    return SQLITE_OK;
}

} // namespace dmit::db
