#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/error.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/db/database.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/cmp/cmp.hpp"

#include "dmit/com/unique_id.hpp"

#include "cmp/cmp.h"

#include <cstdint>
#include <cstdlib>
#include <string>

namespace dmit::drv::srv
{

void addFile(dmit::nng::Socket& socket,
             cmp_ctx_t* context,
             dmit::db::Database& database)
{
    // 1. Deserialize query

    uint32_t filePathSize;

    if (!dmit::cmp::readStrSize(context, &filePathSize)) {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, Reply::KO);
        return;
    }

    std::string filePath;

    filePath.resize(filePathSize);

    if (!dmit::cmp::readBytes(context, filePath.data(), filePathSize))
    {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, Reply::KO);
        return;
    }

    uint32_t unitSourceSize;

    if (!dmit::cmp::readBinSize(context, &unitSourceSize)) {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, Reply::KO);
        return;
    }

    std::vector<uint8_t> unitSource(unitSourceSize);

    if (!dmit::cmp::readBytes(context, unitSource.data(), unitSourceSize))
    {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, Reply::KO);
        return;
    }

    // 2. Update database

    const dmit::com::UniqueId unitId {unitSource};
    const dmit::com::UniqueId fileId {filePath};

    bool isUnitInDb;
    bool isFileInDb;

    int errorCode;

    if ((errorCode = database.hasFile(fileId, isFileInDb)) != SQLITE_OK)
    {
        displaySqlite3Error("hasFile", errorCode);
        replyWith(socket, Reply::KO);
        return;
    }

    if ((errorCode = database.hasUnit(unitId, isUnitInDb)) != SQLITE_OK)
    {
        displaySqlite3Error("hasUnit", errorCode);
        replyWith(socket, Reply::KO);
        return;
    }

    if (!isFileInDb && isUnitInDb)
    {
        errorCode = database.insertFile(fileId, unitId, filePath);
    }

    if (!isFileInDb && !isUnitInDb)
    {
        errorCode = database.insertFileAndUnit(fileId, unitId, filePath, unitSource);
    }

    if (isFileInDb && !isUnitInDb)
    {
        errorCode = database.updateFileAndInsertUnit(fileId, unitId, unitSource);
    }

    if (errorCode != SQLITE_OK)
    {
        displaySqlite3Error("(update|insert)FileWithUnit", errorCode);
        replyWith(socket, Reply::KO);
        return;
    }

    // 3. Reply OK at the end

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
