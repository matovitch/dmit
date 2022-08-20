#include "dmit/drv/client_add_file.hpp"
#include "dmit/drv/client_db_clean.hpp"
#include "dmit/drv/client_db_get.hpp"
#include "dmit/drv/client_make.hpp"
#include "dmit/drv/client_stop.hpp"
#include "dmit/drv/reply.hpp"
#include "dmit/drv/error.hpp"
#include "dmit/drv/query.hpp"

#include "dmit/fmt/logger.hpp"

#include "dmit/nng/nng.hpp"

extern "C"
{
    #include "nng/nng.h"
    #include "nng/protocol/reqrep0/req.h"

    #include "ketopt/ketopt.h"
}

#include <cstdlib>

enum
{
    K_OPTION_INVALID        = ':'   ,
    K_OPTION_HELP           = 'h'   ,
    K_OPTION_VERSION        = 'v'   ,
    K_OPTION_STOP           = 's'   ,
    K_OPTION_URL            = 'u'   ,
    K_OPTION_FILE_PATH      = 'f'   ,
    K_OPTION_MAKE           = 'm'   ,
    K_OPTION_DATABASE_GET   = 0x100 , // These options have no char representation
    K_OPTION_DATABASE_CLEAN         ,
};

static const ko_longopt_t K_OPTIONS_LONG[] =
{
    { "help"      , ko_no_argument       , K_OPTION_HELP           },
    { "version"   , ko_no_argument       , K_OPTION_VERSION        },
    { "stop"      , ko_no_argument       , K_OPTION_STOP           },
    { "make"      , ko_no_argument       , K_OPTION_MAKE           },
    { "db-get"    , ko_no_argument       , K_OPTION_DATABASE_GET   },
    { "db-clean"  , ko_no_argument       , K_OPTION_DATABASE_CLEAN },
    { "url"       , ko_required_argument , K_OPTION_URL            },
    { "file-path" , ko_required_argument , K_OPTION_FILE_PATH      },
    { nullptr     , ko_no_argument       , K_OPTION_INVALID        } // sentinel required
};

static const char* K_OPTIONS_SHORT = "hvsmu:f:";

void displayHelp()
{
    DMIT_COM_LOG_OUT << "dmit_client\n\n";
    DMIT_COM_LOG_OUT << "Usage:\n";
    DMIT_COM_LOG_OUT << "    " << "-h, --help                   Show this screen\n";
    DMIT_COM_LOG_OUT << "    " << "-v, --version                Display version\n";
    DMIT_COM_LOG_OUT << "    " << "-s, --stop                   Stop the server\n";
    DMIT_COM_LOG_OUT << "    " << "-m, --make                   Compile all files\n";
    DMIT_COM_LOG_OUT << "    " << "    --db-get                 Dump the database\n";
    DMIT_COM_LOG_OUT << "    " << "    --db-clean               Clean the database\n";
    DMIT_COM_LOG_OUT << "    " << "-u, --url         URL        Connect to URL\n";
    DMIT_COM_LOG_OUT << "    " << "-f, --file-path   FILE_PATH  Load FILE_PATH\n";
}

void displayVersion()
{
    DMIT_COM_LOG_OUT << "dmit_client, version 0.1\n";
}

int main(int argc, char** argv)
{
    // Decode the arguments

    bool hasHelp          = false;
    bool hasVersion       = false;
    bool hasStop          = false;
    bool hasDatabaseGet   = false;
    bool hasDatabaseClean = false;
    bool hasMake          = false;

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
        hasHelp          |= (ketoptOption == K_OPTION_HELP           );
        hasVersion       |= (ketoptOption == K_OPTION_VERSION        );
        hasStop          |= (ketoptOption == K_OPTION_STOP           );
        hasDatabaseGet   |= (ketoptOption == K_OPTION_DATABASE_GET   );
        hasDatabaseClean |= (ketoptOption == K_OPTION_DATABASE_CLEAN );
        hasMake          |= (ketoptOption == K_OPTION_MAKE           );

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
            dmit::drv::displayNngError("nng_req0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Dial URL

        if ((errorCode = nng_dial(socket._asNng, url, nullptr, 0)) != 0)
        {
            dmit::drv::displayNngError("nng_dial", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // Send Queries

        if (hasStop)
        {
            returnCode = dmit::drv::clt::stop(socket);
            goto CLEAN_UP;
        }

        if (hasDatabaseGet)
        {
            returnCode = dmit::drv::clt::databaseGet(socket);
            goto CLEAN_UP;
        }

        if (hasDatabaseClean)
        {
            returnCode = dmit::drv::clt::databaseClean(socket);
            goto CLEAN_UP;
        }

        if (hasMake)
        {
            returnCode = dmit::drv::clt::make(socket);
            goto CLEAN_UP;
        }

        if (filePath)
        {
            returnCode = dmit::drv::clt::addFile(socket, filePath);
            goto CLEAN_UP;
        }
    }

    CLEAN_UP:
        nng_fini();

    return returnCode;
}
