#include "dmit/com/logger.hpp"
#include "dmit/com/assert.hpp"

#include "nng/nng.hpp"
#include "nng/protocol/reqrep0/req.h"

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

    // Command OK, now doing the work

    int returnCode = EXIT_SUCCESS;

    {
        // 1. Open socket

        nng::Socket socket;
        int         errorCode;

        if ((errorCode = nng_req0_open(&socket._asNng)) != 0)
        {
            fatal("nng_req0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 2. Dial URL

        if ((errorCode = nng_dial(socket._asNng, url, nullptr, 0)) != 0)
        {
            fatal("nng_dial", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 3. Write the query

        uint8_t query[K_QUERY_SIZE];

        cmp_ctx_t cmp1 = {0};

        cmp_init(&cmp1, &query, reader, skipper, writer);

        cmp_write_u64(&cmp1, K_QUERY);

        // 4. Send it

        if ((errorCode = nng_send(socket._asNng, query, sizeof(query), 0)) != 0)
        {
            fatal("nng_send", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 5. Wait for the reply

        nng::Buffer buffer;

        if ((errorCode = nng_recv(socket._asNng, &buffer, 0)) != 0)
        {
            fatal("nng_recv", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 6. Decode it

        DMIT_COM_ASSERT(buffer._size == K_REPLY_SIZE);

        cmp_ctx_t cmp2 = {0};

        cmp_init(&cmp2, buffer._asBytes, reader, skipper, writer);

        uint64_t reply;

        cmp_read_u64(&cmp2, &reply);

        DMIT_COM_ASSERT(reply == K_REPLY);
    }

    FINI:
        nng_fini();

    return returnCode;
}
