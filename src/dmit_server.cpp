#include "dmit/srl/tag.hpp"

#include "dmit/com/logger.hpp"

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

static const int K_REPLY = 43;

bool reader(cmp_ctx_t *ctx, void *data, size_t limit) {
    memcpy(data, ctx->buf, limit);
    ctx->buf = (char*)(ctx->buf) + limit;
    return true;
}

bool skipper(cmp_ctx_t *ctx, size_t count) {
    ctx->buf = (char*)(ctx->buf) + count;
    return true;
}

size_t writer(cmp_ctx_t *ctx, const void *data, size_t count) {
    memcpy(ctx->buf, data, count);
    ctx->buf = (char*)(ctx->buf) + count;
    return count;
}

size_t writerSize(cmp_ctx_t *ctx, const void *data, size_t count) {
    ctx->buf = (char*)(ctx->buf) + count;
    return count;
}

size_t messageSize(cmp_ctx_t *ctx)
{
    return *((size_t*)(&(ctx->buf)));
}

void writeReply(cmp_ctx_t *ctx)
{
    cmp_write_u64(ctx, K_REPLY);
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

        nng::Socket socket;
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

            nng::Buffer queryAsBuffer;

            if ((errorCode = nng_recv(socket._asNng, &queryAsBuffer, 0)) != 0)
            {
                displayNngError("nng_recv", errorCode);
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }

            // 3.2 Decode it

            cmp_ctx_t cmpQuery = {0};

            cmp_init(&cmpQuery, queryAsBuffer._asBytes, reader, skipper, writer);

            uint8_t query = 0;

            if (!cmp_read_u8(&cmpQuery, &query) || query != dmit::srl::Tag::FILE)
            {
                DMIT_COM_LOG_ERR << "Badly formed query\n";
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }

            // 3.3 Estimate reply size

            cmp_ctx_t cmpReplySize = {0};

            cmp_init(&cmpReplySize, nullptr, nullptr, nullptr, writerSize);

            writeReply(&cmpReplySize);

            const size_t replySize = messageSize(&cmpReplySize);

            // 3.4. Write it

            nng::Buffer bufferReply{replySize};

            cmp_ctx_t cmpReply = {0};

            cmp_init(&cmpReply, bufferReply._asBytes, reader, skipper, writer);

            writeReply(&cmpReply);

            // 3.5 And send it

            if ((errorCode = nng_send(socket._asNng, &bufferReply, 0)) != 0)
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
