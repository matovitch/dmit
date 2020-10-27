#include "dmit/drv/reply_code.hpp"
#include "dmit/drv/query.hpp"

#include "dmit/db/database.hpp"

#include "dmit/cmp/drv/reply_code.hpp"
#include "dmit/cmp/cmp.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/logger.hpp"

#include "dmit/nng/nng.hpp"

#include "nng/nng.h"
#include "nng/protocol/reqrep0/rep.h"

#include "sqlite3/sqlite3.h"

#include "cmp/cmp.h"

#include "ketopt/ketopt.h"

#include <cstdlib>
#include <cstdint>

void displaySqlite3Error(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << "error: " << functionName << " returned '" << sqlite3_errstr(errorCode) << "'\n";
}

void displayNngError(const char* functionName, int errorCode)
{
    DMIT_COM_LOG_ERR << "error: " << functionName << " returned '" << nng_strerror(errorCode) << "'\n";
}

bool writeReply(cmp_ctx_t* context, const dmit::drv::ReplyCode replyCode)
{
    return dmit::cmp::write(context, replyCode);
}

void replyWith(dmit::nng::Socket& socket, const dmit::drv::ReplyCode replyCode)
{
    // 1. Write reply

    auto replyOpt = dmit::cmp::asNngBuffer(writeReply, replyCode);

    if (!replyOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to craft reply\n";
        return;
    }

    // 2. Send it

    int errorCode;

    if ((errorCode = nng_send(socket._asNng, &(replyOpt.value()), 0)) != 0)
    {
        displayNngError("nng_send", errorCode);
        return;
    }
}

void replyStop(dmit::nng::Socket& socket, int& returnCode, bool& isStopping)
{
    // 1. Process query

    DMIT_COM_LOG_OUT << "Stopping server...\n";

    isStopping = true;

    // 2. Write reply

    auto replyOpt = dmit::cmp::asNngBuffer(writeReply, dmit::drv::ReplyCode::OK);

    if (!replyOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to craft reply\n";
        returnCode = EXIT_FAILURE;
        return;
    }

    // 3. Send it

    int errorCode;

    if ((errorCode = nng_send(socket._asNng, &(replyOpt.value()), 0)) != 0)
    {
        displayNngError("nng_send", errorCode);
        returnCode = EXIT_FAILURE;
        return;
    }

    returnCode = EXIT_SUCCESS;
}

void replyCreateOrUpdateFile(dmit::nng::Socket& socket,
                             cmp_ctx_t* context,
                             dmit::db::Database& database)
{
    // 1. Deserialize query

    uint32_t size;

    if (!dmit::cmp::readArray(context, &size) || size != 2)
    {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    uint32_t filePathSize;

    if (!dmit::cmp::readStrSize(context, &filePathSize)) {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    std::string filePath;

    filePath.resize(filePathSize);

    if (!dmit::cmp::readBytes(context, filePath.data(), filePathSize))
    {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    uint32_t unitSourceSize;

    if (!dmit::cmp::readBinSize(context, &unitSourceSize)) {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    std::vector<uint8_t> unitSource(unitSourceSize);

    if (!dmit::cmp::readBytes(context, unitSource.data(), unitSourceSize))
    {
        DMIT_COM_LOG_ERR << "error: badly formed query\n";
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    // 2. Update database

    const dmit::com::UniqueId unitId {unitSource};
    const dmit::com::UniqueId fileId {filePath};

    bool isFileInDb;
    int errorCode;

    if ((errorCode = database.hasFile(fileId, isFileInDb)) != SQLITE_OK)
    {
        displaySqlite3Error("hasFile", errorCode);
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    if (isFileInDb)
    {
        bool isUnitInDb;

        if ((errorCode = database.hasUnit(unitId, isUnitInDb)) != SQLITE_OK)
        {
            displaySqlite3Error("hasUnit", errorCode);
            replyWith(socket, dmit::drv::ReplyCode::KO);
            return;
        }

        if (!isUnitInDb)
        {
            errorCode = database.updateFileWithUnit(fileId, unitId, unitSource);
        }
    }
    else
    {
        errorCode = database.insertFileWithUnit(fileId, unitId, filePath, unitSource);
    }

    if (errorCode != SQLITE_OK)
    {
        displaySqlite3Error("(update|insert)FileWithUnit", errorCode);
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    // 3. Reply OK at the end

    replyWith(socket, dmit::drv::ReplyCode::OK);
}

void replyDatabaseGet(dmit::nng::Socket& socket, dmit::db::Database& database)
{
    // 1. Write reply

    auto replyOpt = database.asNngBuffer();

    if (!replyOpt)
    {
        DMIT_COM_LOG_ERR << "error: failed to serialize the database\n";
        return;
    }

    // 2. Send it

    int errorCode;

    if ((errorCode = nng_send(socket._asNng, &(replyOpt.value()), 0)) != 0)
    {
        displayNngError("nng_send", errorCode);
        return;
    }
}

void replyDatabaseClean(dmit::nng::Socket& socket, dmit::db::Database& database)
{
    // 1. Process query

    int errorCode;

    if ((errorCode = database.clean()) != SQLITE_OK)
    {
        displaySqlite3Error("clean", errorCode);
        replyWith(socket, dmit::drv::ReplyCode::KO);
        return;
    }

    // 2. Reply OK at the end

    replyWith(socket, dmit::drv::ReplyCode::OK);
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
        displaySqlite3Error("database", errorCode);
        return EXIT_FAILURE;
    }

    int returnCode;

    {
        // 1. Open socket

        dmit::nng::Socket socket;

        if ((errorCode = nng_rep0_open(&socket._asNng)) != 0)
        {
            displayNngError("nng_rep0_open", errorCode);
            returnCode = EXIT_FAILURE;
            goto CLEAN_UP;
        }

        // 2. Listen onto URL

        if ((errorCode = nng_listen(socket._asNng, url, nullptr, 0)) != 0)
        {
            displayNngError("nng_listen", errorCode);
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
                displayNngError("nng_recv", errorCode);
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

            if (query == dmit::drv::Query::CREATE_OR_UPDATE_FILE)
            {
                replyCreateOrUpdateFile(socket, &cmpContextQuery, database);
            }

            if (query == dmit::drv::Query::STOP_SERVER)
            {
                replyStop(socket, returnCode, isStopping);
            }

            if (query == dmit::drv::Query::DATABASE_GET)
            {
                replyDatabaseGet(socket, database);
            }

            if (query == dmit::drv::Query::DATABASE_CLEAN)
            {
                replyDatabaseClean(socket, database);
            }
        }
    }

    CLEAN_UP:
        nng_fini();

    return returnCode;
}
