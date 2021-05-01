#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/error.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/db/database.hpp"

#include "dmit/nng/nng.hpp"

namespace dmit::drv::srv
{

void databaseClean(dmit::nng::Socket& socket, dmit::db::Database& database)
{
    // 1. Process query

    int errorCode;

    if ((errorCode = database.clean()) != SQLITE_OK)
    {
        displaySqlite3Error("clean", errorCode);
        replyWith(socket, Reply::KO);
        return;
    }

    // 2. Reply OK at the end

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
