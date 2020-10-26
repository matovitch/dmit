#include "dmit/db/database.hpp"

#include "dmit/db/query_register.hpp"
#include "dmit/db/connection.hpp"

#include "dmit/com/unique_id.hpp"

#include "sqlite3/sqlite3.h"

#include <cstdint>
#include <string>
#include <vector>

namespace dmit::db
{

Database::Database(int& errorCode) :
    _connection{errorCode},
    _queryRegister{_connection, errorCode}
{}

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

    int returnCode = sqlite3_step(querySelectFile);

    if ((errorCode = sqlite3_reset(querySelectFile)) != SQLITE_OK)
    {
        return errorCode;
    }

    if (returnCode != SQLITE_DONE &&
        returnCode != SQLITE_ROW)
    {
        return returnCode;
    }

    result = (returnCode != SQLITE_DONE);

    return SQLITE_OK;
}

int Database::insertFile(const com::UniqueId        & fileId,
                         const std::vector<uint8_t> & fileContent,
                         const std::string          & filePath)
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

    if ((errorCode = sqlite3_bind_text(queryInsertFile,
                                       QueryRegister::FILE_PATH,
                                       filePath.data(),
                                       filePath.size(),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    if ((errorCode = sqlite3_bind_blob(queryInsertFile,
                                       QueryRegister::FILE_CONTENT,
                                       fileContent.data(),
                                       fileContent.size(),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int returnCode = sqlite3_step(queryInsertFile);

    if ((errorCode = sqlite3_reset(queryInsertFile)) != SQLITE_OK)
    {
        return errorCode;
    }

    if (returnCode != SQLITE_DONE)
    {
        return returnCode;
    }

    return SQLITE_OK;
}

int Database::updateFile(const com::UniqueId        & fileId,
                         const std::vector<uint8_t> & fileContent)
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
                                       QueryRegister::FILE_CONTENT,
                                       fileContent.data(),
                                       fileContent.size(),
                                       nullptr)) != SQLITE_OK)
    {
        return errorCode;
    }

    int returnCode = sqlite3_step(queryUpdateFile);

    if ((errorCode = sqlite3_reset(queryUpdateFile)) != SQLITE_OK)
    {
        return errorCode;
    }

    if (returnCode != SQLITE_DONE)
    {
        return returnCode;
    }

    return SQLITE_OK;
}

} // namespace dmit::db
