#include "dmit/cmp/cmp.hpp"
#include "dmit/cmp/tag.hpp"

#include "dmit/com/logger.hpp"

#include "dmit/nng/nng.hpp"

#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"

#include "cmp/cmp.h"

#include "ketopt/ketopt.h"

#include <iostream>
#include <cstdlib>
#include <cstdint>

static const int K_REPLY = 43;

bool writeReply(const uint64_t reply, cmp_ctx_t* ctx)
{
    return dmit::cmp::write_u64(ctx, reply);
}

void displayNngError(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << functionName << ": " << nng_strerror(errorCode) << '\n';
}

enum : char
{
    K_OPTION_INVALID     = ':',
    K_OPTION_HELP        = 'h',
    K_OPTION_VERSION     = 'v',
    K_OPTION_URL         = 'u',
};

static const ko_longopt_t K_OPTIONS_LONG[] =
{
    { "help"    , ko_no_argument       , K_OPTION_HELP    },
    { "version" , ko_no_argument       , K_OPTION_VERSION },
    { "url"     , ko_required_argument , K_OPTION_URL     },
    { nullptr   , ko_no_argument       , K_OPTION_INVALID } // sentinel required
};

static const char* K_OPTIONS_SHORT = "hvu:";

void displayHelp()
{
    DMIT_COM_LOG_OUT << "dmit_client\n\n";
    DMIT_COM_LOG_OUT << "Usage:\n";
    DMIT_COM_LOG_OUT << "    " << "-h, --help      Show this screen\n";
    DMIT_COM_LOG_OUT << "    " << "-v, --version   Display version\n";
    DMIT_COM_LOG_OUT << "    " << "-u, --url  URL  Serve at URL\n";
}

void displayVersion()
{
    DMIT_COM_LOG_OUT << "dmit_server, version 0.1\n";
}

int main(int argc, char** argv)
{
    // Decode the arguments

    bool        hasHelp    = false;
    bool        hasVersion = false;
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

        if (ketoptOption == K_OPTION_URL)
        {
            url = ketoptStatus.arg;
        }
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

    // Everything fine, now doing the work

    int returnCode = EXIT_SUCCESS;

    {
        // 1. Open socket

        dmit::nng::Socket socket;
        int errorCode;

        if ((errorCode = nng_rep0_open(&socket._asNng)) != 0)
        {
            displayNngError("nng_rep0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // 2. Listen URL

        if ((errorCode = nng_listen(socket._asNng, url, nullptr, 0)) != 0)
        {
            displayNngError("nng_listen", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // 3. Loop awaiting requests

        while (true)
        {
            // 3.1. Expect a query

            dmit::nng::Buffer bufferQuery;

            if ((errorCode = nng_recv(socket._asNng, &bufferQuery, 0)) != 0)
            {
                displayNngError("nng_recv", errorCode);
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }

            // 3.2 Decode it

            auto cmpContextQuery = dmit::cmp::contextFromNngBuffer(bufferQuery);

            uint8_t query = dmit::cmp::Tag::INVALID;

            if (!cmp_read_u8(&cmpContextQuery, &query) || query != dmit::cmp::Tag::FILE)
            {
                DMIT_COM_LOG_ERR << "Badly formed query\n";
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }

            // 3.4. Write reply

            auto replyOpt = dmit::cmp::asNngBuffer(K_REPLY, writeReply);

            if (!replyOpt)
            {
                DMIT_COM_LOG_ERR << "error: failed to craft reply\n";
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }

            // 3.5 And send it

            if ((errorCode = nng_send(socket._asNng, &(replyOpt.value()), 0)) != 0)
            {
                displayNngError("nng_send", errorCode);
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }
        }
    }

    CLEAN_UP:
        nng_fini();

    return returnCode;
}
