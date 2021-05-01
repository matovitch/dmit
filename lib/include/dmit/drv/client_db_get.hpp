#pragma once

#include "dmit/drv/error.hpp"
#include "dmit/drv/query.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/cmp/drv/query.hpp"
#include "dmit/cmp/cmp.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/com/logger.hpp"

#include "cmp/cmp.h"

#include <cstdlib>

namespace dmit::drv::clt
{

bool queryDatabaseGet(cmp_ctx_t* context)
{
    return dmit::cmp::write(context, dmit::drv::Query::DATABASE_GET);
}


int databaseGet(dmit::nng::Socket& socket)
{
    // Write query

    auto queryOpt = dmit::cmp::asNngBuffer(queryDatabaseGet);

    if (!queryOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to craft query\n";
        return EXIT_FAILURE;
    }

    // Send query

    int errorCode;

    if ((errorCode = nng_send(socket._asNng, &(queryOpt.value()), 0)) != 0)
    {
        dmit::drv::displayNngError("nng_send", errorCode);
        return EXIT_FAILURE;
    }

    // Wait for reply

    dmit::nng::Buffer bufferReply;

    if ((errorCode = nng_recv(socket._asNng, &bufferReply, 0)) != 0)
    {
        dmit::drv::displayNngError("nng_recv", errorCode);
        return EXIT_FAILURE;
    }

    // Dump reply

    for (int i = 0; i < bufferReply._size; i++)
    {
        DMIT_COM_LOG_OUT << bufferReply._asBytes[i];
    }

    return EXIT_SUCCESS;
}

} // namespace dmit::drv::clt
