#include "dmit/com/logger.hpp"
#include "dmit/com/assert.hpp"

#include "nng/nng.h"
#include <nng/protocol/reqrep0/rep.h>

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
    K_OPTION_FILE_PATH   = 'f',
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

#define PUT64(ptr, u)                                \
    do {                                             \
        (ptr)[0] = (uint8_t)(((uint64_t)(u)) >> 56); \
        (ptr)[1] = (uint8_t)(((uint64_t)(u)) >> 48); \
        (ptr)[2] = (uint8_t)(((uint64_t)(u)) >> 40); \
        (ptr)[3] = (uint8_t)(((uint64_t)(u)) >> 32); \
        (ptr)[4] = (uint8_t)(((uint64_t)(u)) >> 24); \
        (ptr)[5] = (uint8_t)(((uint64_t)(u)) >> 16); \
        (ptr)[6] = (uint8_t)(((uint64_t)(u)) >> 8);  \
        (ptr)[7] = (uint8_t)((uint64_t)(u));         \
    } while (0)

#define GET64(ptr, v)                                 \
    v = (((uint64_t)((uint8_t)(ptr)[0])) << 56) + \
        (((uint64_t)((uint8_t)(ptr)[1])) << 48) + \
        (((uint64_t)((uint8_t)(ptr)[2])) << 40) + \
        (((uint64_t)((uint8_t)(ptr)[3])) << 32) + \
        (((uint64_t)((uint8_t)(ptr)[4])) << 24) + \
        (((uint64_t)((uint8_t)(ptr)[5])) << 16) + \
        (((uint64_t)((uint8_t)(ptr)[6])) << 8) +  \
        (((uint64_t)(uint8_t)(ptr)[7]))

#define DATECMD 1

void fatal(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << functionName << ": " << nng_strerror(errorCode) << '\n';
}

struct Socket
{
    ~Socket();

    nng_socket _asNng;
};

Socket::~Socket()
{
    nng_close(_asNng);
}

struct Buffer
{
    ~Buffer();

    void release();

    char*  _asBytes = nullptr;
    size_t _size;
};

void Buffer::release()
{
    _asBytes = nullptr;
}

Buffer::~Buffer()
{
    if (_asBytes)
    {
        nng_free(_asBytes, _size);
    }
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

    int returnCode = EXIT_SUCCESS;

    {
        Socket socket;
        int    errorCode;

        // 1. Open socket and dial url

        if ((errorCode = nng_rep0_open(&socket._asNng)) != 0)
        {
            fatal("nng_rep0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        if ((errorCode = nng_listen(socket._asNng, url, nullptr, 0)) != 0)
        {
            fatal("nng_listen", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 2. Loop awaiting requests

        while (true)
        {
            // 2.1. Check for mail

            Buffer buffer;

            if ((errorCode = nng_recv(socket._asNng, &buffer._asBytes, &buffer._size, NNG_FLAG_ALLOC)) != 0)
            {
                fatal("nng_recv", errorCode);
                returnCode = EXIT_FAILURE;
                goto FINI;
            }

            // 2.2. Write and send the reply

            uint64_t command;

            DMIT_COM_ASSERT(buffer._size == sizeof(uint64_t) && (GET64(buffer._asBytes, command)) == DATECMD);

            PUT64(buffer._asBytes, 42);

            if ((errorCode = nng_send(socket._asNng, buffer._asBytes, buffer._size, NNG_FLAG_ALLOC)) != 0)
            {
                fatal("nng_send", errorCode);
                returnCode = EXIT_FAILURE;
                goto FINI;
            }

            buffer.release();
        }
    }

    FINI:
        nng_fini();

    return returnCode;
}
