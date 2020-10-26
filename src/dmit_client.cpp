#include "dmit/drv/query.hpp"

#include "dmit/src/file.hpp"

#include "dmit/cmp/drv/query.hpp"
#include "dmit/cmp/src/file.hpp"
#include "dmit/cmp/cmp.hpp"

#include "dmit/com/logger.hpp"

#include "dmit/nng/nng.hpp"

#include "nng/nng.h"
#include "nng/protocol/reqrep0/req.h"

#include "cmp/cmp.h"

#include "ketopt/ketopt.h"

#include <iostream>
#include <cstdlib>
#include <cstdint>

static const int K_REPLY_ACK = 42;

bool queryCreateOrUpdateFile(cmp_ctx_t* context, const dmit::src::File& file)
{
    if (!dmit::cmp::write(context, dmit::drv::Query::CREATE_OR_UPDATE_FILE))
    {
        return false;
    }

    return dmit::cmp::write(context, file);
}

bool queryStopServer(cmp_ctx_t* context)
{
    return dmit::cmp::write(context, dmit::drv::Query::STOP_SERVER);
}

void displayNngError(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << "error: " << functionName << " returned '" << nng_strerror(errorCode) << "'\n";
}

enum : char
{
    K_OPTION_INVALID     = ':',
    K_OPTION_HELP        = 'h',
    K_OPTION_VERSION     = 'v',
    K_OPTION_STOP_SERVER = 's',
    K_OPTION_URL         = 'u',
    K_OPTION_FILE_PATH   = 'f',
};

static const ko_longopt_t K_OPTIONS_LONG[] =
{
    { "help"        , ko_no_argument       , K_OPTION_HELP        },
    { "version"     , ko_no_argument       , K_OPTION_VERSION     },
    { "stop-server" , ko_no_argument       , K_OPTION_STOP_SERVER },
    { "url"         , ko_required_argument , K_OPTION_URL         },
    { "file-path"   , ko_required_argument , K_OPTION_FILE_PATH   },
    { nullptr       , ko_no_argument       , K_OPTION_INVALID     } // sentinel required
};

static const char* K_OPTIONS_SHORT = "hvsu:f:";

void displayHelp()
{
    DMIT_COM_LOG_OUT << "dmit_client\n\n";
    DMIT_COM_LOG_OUT << "Usage:\n";
    DMIT_COM_LOG_OUT << "    " << "-h, --help                   Show this screen\n";
    DMIT_COM_LOG_OUT << "    " << "-v, --version                Display version\n";
    DMIT_COM_LOG_OUT << "    " << "-s, --stop-server            Stop the server\n";
    DMIT_COM_LOG_OUT << "    " << "-u, --url         URL        Connect to URL\n";
    DMIT_COM_LOG_OUT << "    " << "-f, --file-path   FILE_PATH  Load FILE_PATH\n";
}

void displayVersion()
{
    DMIT_COM_LOG_OUT << "dmit_client, version 0.1\n";
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

int stopServer(dmit::nng::Socket& socket)
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

    uint64_t reply;

    if (!dmit::cmp::readU64(&cmpContextReply, &reply) || reply != K_REPLY_ACK)
    {
        DMIT_COM_LOG_ERR << "error: badly formed reply\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int createOrUpdateFile(dmit::nng::Socket& socket, const char* filePath)
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

    uint64_t reply;

    if (!dmit::cmp::readU64(&cmpContextReply, &reply) || reply != K_REPLY_ACK)
    {
        DMIT_COM_LOG_ERR << "error: badly formed reply\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char** argv)
{
    // Decode the arguments

    bool        hasHelp       = false;
    bool        hasVersion    = false;
    bool        hasStopServer = false;
    const char* filePath      = nullptr;
    const char* url           = nullptr;

    ketopt_t ketoptStatus = KETOPT_INIT;
    int      ketoptOption;

    while ((ketoptOption = ketopt(&ketoptStatus,
                                  argc,
                                  argv,
                                  0,
                                  K_OPTIONS_SHORT,
                                  K_OPTIONS_LONG)) != -1)
    {
        hasHelp       |= (ketoptOption == K_OPTION_HELP        );
        hasVersion    |= (ketoptOption == K_OPTION_VERSION     );
        hasStopServer |= (ketoptOption == K_OPTION_STOP_SERVER );

        if (ketoptOption == K_OPTION_FILE_PATH ) { filePath = ketoptStatus.arg; }
        if (ketoptOption == K_OPTION_URL       ) { url      = ketoptStatus.arg; }
    }

    // Display help/version if needed

    if (hasHelp)
    {
        displayHelp();
        return EXIT_SUCCESS;
    }

    if (hasVersion)
    {
        displayVersion();
        return EXIT_SUCCESS;
    }

    // Check the command is properly formed

    if (!url)
    {
        DMIT_COM_LOG_ERR << "error: url option is necessary to reach the server";
        displayHelp();
        return EXIT_FAILURE;
    }

    int returnCode = EXIT_SUCCESS;

    {
        // Open socket

        dmit::nng::Socket socket;
        int errorCode;

        if ((errorCode = nng_req0_open(&socket._asNng)) != 0)
        {
            displayNngError("nng_req0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Dial URL

        if ((errorCode = nng_dial(socket._asNng, url, nullptr, 0)) != 0)
        {
            displayNngError("nng_dial", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Send Queries

        if (hasStopServer)
        {
            returnCode = stopServer(socket);
            goto CLEAN_UP;
        }

        if (filePath)
        {
            returnCode = createOrUpdateFile(socket, filePath);
            goto CLEAN_UP;
        }
    }

    CLEAN_UP:
        nng_fini();

    return returnCode;
}
