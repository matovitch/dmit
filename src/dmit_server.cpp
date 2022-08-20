#include "dmit/drv/server_add_file.hpp"
#include "dmit/drv/server_db_clean.hpp"
#include "dmit/drv/server_db_get.hpp"
#include "dmit/drv/server_stop.hpp"
#include "dmit/drv/server_make.hpp"

#include "dmit/drv/error.hpp"
#include "dmit/drv/query.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/db/database.hpp"

#include "dmit/fmt/logger.hpp"

#include "dmit/cmp/cmp.hpp"

#include "dmit/nng/nng.hpp"

extern "C"
{
    #include "nng/nng.h"
    #include "nng/protocol/reqrep0/rep.h"

    #include "ketopt/ketopt.h"
}

#include <cstdlib>

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

    // Check the command is properly formed

    if (!url)
    {
        DMIT_COM_LOG_ERR << "error: url option is mandatory to create the server";
        displayHelp();
        return EXIT_FAILURE;
    }

    // Create the in-memory database

    int errorCode;

    dmit::db::Database database{errorCode};

    if (errorCode != SQLITE_OK)
    {
        dmit::drv::displaySqlite3Error("database", errorCode);
        return EXIT_FAILURE;
    }

    int returnCode;

    {
        // 1. Open socket

        dmit::nng::Socket socket;

        if ((errorCode = nng_rep0_open(&socket._asNng)) != 0)
        {
            dmit::drv::displayNngError("nng_rep0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // 2. Listen onto URL

        if ((errorCode = nng_listen(socket._asNng, url, nullptr, 0)) != 0)
        {
            dmit::drv::displayNngError("nng_listen", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // 3. Loop awaiting requests

        bool isStopping = false;

        while (!isStopping)
        {
            // 3.1. Await a query

            dmit::nng::Buffer bufferQuery;

            if ((errorCode = nng_recv(socket._asNng, &bufferQuery, 0)) != 0)
            {
                dmit::drv::displayNngError("nng_recv", errorCode);
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }

            // 3.2 Decode it

            auto cmpContextQuery = dmit::cmp::contextFromNngBuffer(bufferQuery);

            uint8_t query;

            if (!dmit::cmp::readU8(&cmpContextQuery, &query))
            {
                DMIT_COM_LOG_ERR << "error: badly formed query\n";
                returnCode = EXIT_FAILURE;
                goto CLEAN_UP;
            }

            // 3.3 Process and reply

            if (query == dmit::drv::Query::ADD_FILE)
            {
                DMIT_COM_LOG_OUT << "Registering file...\n";
                dmit::drv::srv::addFile(socket, &cmpContextQuery, database);
            }

            if (query == dmit::drv::Query::STOP)
            {
                DMIT_COM_LOG_OUT << "Stopping process...\n";
                dmit::drv::srv::stop(socket, returnCode, isStopping);
            }

            if (query == dmit::drv::Query::DATABASE_GET)
            {
                DMIT_COM_LOG_OUT << "Retrieving database...\n";
                dmit::drv::srv::databaseGet(socket, database);
            }

            if (query == dmit::drv::Query::DATABASE_CLEAN)
            {
                DMIT_COM_LOG_OUT << "Cleaning database...\n";
                dmit::drv::srv::databaseClean(socket, database);
            }

            if (query == dmit::drv::Query::MAKE)
            {
                DMIT_COM_LOG_OUT << "Compiling registered files...\n";
                dmit::drv::srv::make(socket, database);
            }
        }
    }

    CLEAN_UP:
        nng_fini();

    return returnCode;
}
