#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/error.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/nng/nng.hpp"

extern "C"
{
    #include "nng/nng.h"
}

#include <cstdlib>

namespace dmit::drv::srv
{

void stop(dmit::nng::Socket& socket, int& returnCode, bool& isStopping)
{
    // 1. Process query

    isStopping = true;

    // 2. Write reply

    auto replyOpt = dmit::cmp::asNngBuffer(writeReply, dmit::drv::Reply::OK);

    if (!replyOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to craft reply\n";
        returnCode = EXIT_FAILURE;
        return;
    }

    // 3. Send it

    int errorCode;

    if ((errorCode = nng_send(socket._asNng, &(replyOpt.value()), 0)) != 0)
    {
        displayNngError("nng_send", errorCode);
        returnCode = EXIT_FAILURE;
        return;
    }

    returnCode = EXIT_SUCCESS;
}

} // namespace dmit::drv::srv
