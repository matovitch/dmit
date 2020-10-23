#include "dmit/com/logger.hpp"
#include "dmit/com/assert.hpp"

#include "nng/nng.h"
#include <nng/protocol/reqrep0/req.h>

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

    char*  _asBytes = nullptr;
    size_t _size;
};

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

    int returnCode = EXIT_SUCCESS;

    {
        // 1. Open socket and dial url

        Socket socket;
        int    errorCode;

        if ((errorCode = nng_req0_open(&socket._asNng)) != 0)
        {
            fatal("nng_req0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        if ((errorCode = nng_dial(socket._asNng, url, nullptr, 0)) != 0)
        {
            fatal("nng_dial", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 2. Send the command

        uint8_t command[sizeof(uint64_t)];

        PUT64(command, DATECMD);

        if ((errorCode = nng_send(socket._asNng, command, sizeof(command), 0)) != 0)
        {
            fatal("nng_send", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        // 3. Wait for the reply

        Buffer buffer;

        if ((errorCode = nng_recv(socket._asNng, &buffer._asBytes, &buffer._size, NNG_FLAG_ALLOC)) != 0)
        {
            fatal("nng_recv", errorCode);
            returnCode = EXIT_FAILURE;
            goto FINI;
        }

        DMIT_COM_ASSERT(buffer._size == sizeof(uint64_t));

        // 4. Display it

        uint64_t now;
        GET64(buffer._asBytes, now);

        DMIT_COM_LOG_OUT << now << '\n';
    }

    FINI:
        nng_fini();

    return returnCode;
}
