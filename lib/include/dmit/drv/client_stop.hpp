#pragma once

#include "dmit/drv/error.hpp"
#include "dmit/drv/query.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/cmp/drv/query.hpp"
#include "dmit/cmp/cmp.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/com/logger.hpp"

extern "C"
{
    #include "cmp/cmp.h"
}

#include <cstdlib>

namespace dmit::drv::clt
{

bool queryStopServer(cmp_ctx_t* context)
{
    return dmit::cmp::write(context, Query::STOP);
}

int stop(dmit::nng::Socket& socket)
{
    // Write query

    auto queryOpt = dmit::cmp::asNngBuffer(queryStopServer);

    if (!queryOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to craft query\n";
        return EXIT_FAILURE;
    }

    // Send query

    int errorCode;

    if ((errorCode = nng_send(socket._asNng, &(queryOpt.value()), 0)) != 0)
    {
        displayNngError("nng_send", errorCode);
        return EXIT_FAILURE;
    }

    // Wait for reply

    dmit::nng::Buffer bufferReply;

    if ((errorCode = nng_recv(socket._asNng, &bufferReply, 0)) != 0)
    {
        displayNngError("nng_recv", errorCode);
        return EXIT_FAILURE;
    }

    // Decode reply

    cmp_ctx_t cmpContextReply = dmit::cmp::contextFromNngBuffer(bufferReply);

    uint8_t replyCode;

    if (!dmit::cmp::readU8(&cmpContextReply, &replyCode) || replyCode != Reply::OK)
    {
        DMIT_COM_LOG_ERR << "error: badly formed reply\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

} // namespace dmit::drv::clt
