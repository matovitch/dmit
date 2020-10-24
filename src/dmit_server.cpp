#include "dmit/com/logger.hpp"
#include "dmit/com/assert.hpp"

#include "nng/nng.hpp"
#include "nng/protocol/reqrep0/rep.h"

#include "cmp/cmp.h"

extern "C"
{
    #include "ketopt/ketopt.h"
}

#include <iostream>
#include <cstdlib>
#include <cstdint>

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

bool reader(cmp_ctx_t *ctx, void *data, size_t limit) {
    memcpy(data, ctx->buf, limit);
    char** ctxBytes = (char**)(&(ctx->buf));
    *ctxBytes += limit;
    return true;
}

bool skipper(cmp_ctx_t *ctx, size_t count) {
    char** ctxBytes = (char**)(&(ctx->buf));
    *ctxBytes += count;
    return true;
}

size_t writer(cmp_ctx_t *ctx, const void *data, size_t count) {
    memcpy(ctx->buf, data, count);
    char** ctxBytes = (char**)(&(ctx->buf));
    *ctxBytes += count;
    return count;
}

void fatal(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << functionName << ": " << nng_strerror(errorCode) << '\n';
}

static const int K_QUERY      = 42;
static const int K_REPLY      = 43;
static const int K_QUERY_SIZE =  9;
static const int K_REPLY_SIZE =  9;

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
        nng::Socket socket;
        int         errorCode;

        // 1. Open socket

        if ((errorCode = nng_rep0_open(&socket._asNng)) != 0)
        {
            fatal("nng_rep0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 2. Dial URL

        if ((errorCode = nng_listen(socket._asNng, url, nullptr, 0)) != 0)
        {
            fatal("nng_listen", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 3. Loop awaiting requests

        while (true)
        {
            // 3.1. Expect a query

            nng::Buffer buffer;

            if ((errorCode = nng_recv(socket._asNng, &buffer, 0)) != 0)
            {
                fatal("nng_recv", errorCode);
                returnCode = EXIT_FAILURE;
                goto FINI;
            }

            // 3.2 Decode it

            DMIT_COM_ASSERT(buffer._size == K_QUERY_SIZE);

            cmp_ctx_t cmp1 = {0};

            cmp_init(&cmp1, buffer._asBytes, reader, skipper, writer);

            uint64_t query = 0;

            cmp_read_u64(&cmp1, &query);

            DMIT_COM_ASSERT(query == K_QUERY);

            // 3.3. Write a reply

            uint8_t reply[K_REPLY_SIZE];

            cmp_ctx_t cmp2 = {0};

            cmp_init(&cmp2, &reply, reader, skipper, writer);

            cmp_write_u64(&cmp2, K_REPLY);

            // 3.4 Send it

            if ((errorCode = nng_send(socket._asNng, reply, sizeof(reply), 0)) != 0)
            {
                fatal("nng_send", errorCode);
                returnCode = EXIT_FAILURE;
                goto FINI;
            }
        }
    }

    FINI:
        nng_fini();

    return returnCode;
}
