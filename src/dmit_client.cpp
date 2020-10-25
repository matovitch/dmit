#include "dmit/src/file.hpp"


#include "dmit/srl/serializable.hpp"
#include "dmit/srl/src/file.hpp"

#include "dmit/com/logger.hpp"

#include "nng/nng.hpp"
#include "nng/protocol/reqrep0/req.h"

#include "cmp/cmp.h"

#include "ketopt/ketopt.h"

#include <iostream>
#include <cstdlib>
#include <cstdint>

static const int K_REPLY = 43;

bool writeQuery(const dmit::src::File& file, cmp_ctx_t* ctx)
{
    return dmit::srl::serialize(file, ctx);
}

void displayNngError(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << "error: " << functionName << " returned '" << nng_strerror(errorCode) << "'\n";
}

enum : char
{
    K_OPTION_INVALID   = ':',
    K_OPTION_HELP      = 'h',
    K_OPTION_VERSION   = 'v',
    K_OPTION_FILE_PATH = 'f',
    K_OPTION_URL       = 'u',
};

static const ko_longopt_t K_OPTIONS_LONG[] =
{
    { "help"      , ko_no_argument       , K_OPTION_HELP       },
    { "version"   , ko_no_argument       , K_OPTION_VERSION    },
    { "file-path" , ko_required_argument , K_OPTION_FILE_PATH  },
    { "url"       , ko_required_argument , K_OPTION_URL        },
    { nullptr     , ko_no_argument       , K_OPTION_INVALID    } // sentinel required
};

static const char* K_OPTIONS_SHORT = "hvf:u:";

void displayHelp()
{
    DMIT_COM_LOG_OUT << "dmit_client\n\n";
    DMIT_COM_LOG_OUT << "Usage:\n";
    DMIT_COM_LOG_OUT << "    " << "-h, --help                  Show this screen\n";
    DMIT_COM_LOG_OUT << "    " << "-v, --version               Display version\n";
    DMIT_COM_LOG_OUT << "    " << "-u, --url        URL        Connect to SERVER_URL\n";
    DMIT_COM_LOG_OUT << "    " << "-f, --file-path  FILE_PATH  Load FILE_PATH\n";
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

int main(int argc, char** argv)
{
    // Decode the arguments

    bool        hasHelp    = false;
    bool        hasVersion = false;
    const char* filePath   = nullptr;
    const char* url        = nullptr;

    ketopt_t ketoptStatus = KETOPT_INIT;
    int      ketoptOption;

    while ((ketoptOption = ketopt(&ketoptStatus,
                                  argc,
                                  argv,
                                  0,
                                  K_OPTIONS_SHORT,
                                  K_OPTIONS_LONG)) != -1)
    {
        hasHelp    |= (ketoptOption == K_OPTION_HELP    );
        hasVersion |= (ketoptOption == K_OPTION_VERSION );

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

    if (!filePath || !url)
    {
        DMIT_COM_LOG_ERR << "error: unrecognized command line\n";
        displayHelp();
        return EXIT_FAILURE;
    }

    // Read the file

    const auto& fileErrOpt = dmit::src::file::make(filePath);

    if (fileErrOpt.hasError())
    {
        displayFileError(fileErrOpt.error(), filePath);
        return EXIT_FAILURE;
    }

    const auto& file = fileErrOpt.value();

    int returnCode = EXIT_SUCCESS;

    {
        // Open socket

        nng::Socket socket;
        int         errorCode;

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

        // Write query

        auto queryOpt = dmit::srl::asNngBuffer(file, writeQuery);

        if (!queryOpt)
        {
            DMIT_COM_LOG_ERR << "error: failed to craft query\n";
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Send query

        if ((errorCode = nng_send(socket._asNng, &(queryOpt.value()), 0)) != 0)
        {
            displayNngError("nng_send", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Wait for reply

        nng::Buffer bufferReply;

        if ((errorCode = nng_recv(socket._asNng, &bufferReply, 0)) != 0)
        {
            displayNngError("nng_recv", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Decode reply

        cmp_ctx_t cmpReply = dmit::srl::cmpContextFromNngBuffer(bufferReply);

        uint64_t reply;

        if (!cmp_read_u64(&cmpReply, &reply))
        {
            DMIT_COM_LOG_ERR << "error: badly formed reply\n";
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Exit

        returnCode = (reply == K_REPLY) ? EXIT_SUCCESS
                                        : EXIT_FAILURE;
    }

    CLEAN_UP:
        nng_fini();

    return returnCode;
}
