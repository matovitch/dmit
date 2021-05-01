#pragma once

#include "dmit/drv/error.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/cmp/drv/reply.hpp"
#include "dmit/cmp/cmp.hpp"

#include "dmit/nng/nng.hpp"


namespace dmit::drv::srv
{

bool writeReply(cmp_ctx_t* context, const dmit::drv::Reply reply)
{
    return dmit::cmp::write(context, reply);
}

void replyWith(dmit::nng::Socket& socket, const dmit::drv::Reply reply)
{
    // 1. Write reply

    auto replyOpt = dmit::cmp::asNngBuffer(writeReply, reply);

    if (!replyOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to craft reply\n";
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
