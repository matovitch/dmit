#pragma once

#include "dmit/drv/error.hpp"
#include "dmit/drv/query.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/cmp/drv/query.hpp"
#include "dmit/cmp/src/file.hpp"
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

bool queryCreateOrUpdateFile(cmp_ctx_t* context, const dmit::src::File& file)
{
    if (!dmit::cmp::write(context, Query::ADD_FILE))
    {
        return false;
    }

    return dmit::cmp::write(context, file);
}

void displayFileError(const dmit::src::file::Error& fileError, const char* fileName)
{
    if (fileError == dmit::src::file::Error::FILE_NOT_FOUND)
    {
        DMIT_COM_LOG_ERR << "error: cannot find file '" << fileName << "'\n";
    }

    if (fileError == dmit::src::file::Error::FILE_NOT_REGULAR)
    {
        DMIT_COM_LOG_ERR << "error: '" << fileName << "' is not a regular file\n";
    }

    if (fileError == dmit::src::file::Error::FILE_OPEN_FAIL)
    {
        DMIT_COM_LOG_ERR << "error: cannot open '" << fileName << "'\n";
    }

    if (fileError == dmit::src::file::Error::FILE_READ_FAIL)
    {
        DMIT_COM_LOG_ERR << "error: failed to read file '" << fileName << "'\n";
    }
}

int addFile(dmit::nng::Socket& socket, const char* filePath)
{
    // Read the file

    const auto& fileErrOpt = dmit::src::file::make(filePath);

    if (fileErrOpt.hasError())
    {
        displayFileError(fileErrOpt.error(), filePath);
        return EXIT_FAILURE;
    }

    const auto& file = fileErrOpt.value();

    // Write query

    auto queryOpt = dmit::cmp::asNngBuffer(queryCreateOrUpdateFile, file);

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
