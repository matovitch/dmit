#pragma once

#include "dmit/drv/error.hpp"

#include "dmit/db/database.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/com/logger.hpp"

namespace dmit::drv::srv
{

void databaseGet(dmit::nng::Socket& socket, dmit::db::Database& database)
{
    // 1. Write reply

    auto replyOpt = database.asNngBuffer();

    if (!replyOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to serialize the database\n";
        return;
    }

    // 2. Send it

    int errorCode;

    if ((errorCode = nng_send(socket._asNng, &(replyOpt.value()), 0)) != 0)
    {
        displayNngError("nng_send", errorCode);
        return;
    }
}

} // namespace dmit::drv::srv
