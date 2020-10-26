#include "dmit/db/connection.hpp"

#include "dmit/com/assert.hpp"

#include "sqlite3/sqlite3.h"

namespace dmit::db
{

static const char K_QUERY_CREATE_SCHEMA[] =
    R"(
        CREATE TABLE Files (
            id      BLOB PRIMARY KEY,
            path    TEXT,
            content BLOB
        );
    )";

Connection::Connection(int& errorCode)
{
    if((errorCode = sqlite3_open_v2(nullptr,
                                    &(_asSqlite3),
                                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_MEMORY,
                                    nullptr)) != SQLITE_OK)
    {
        return;
    }

    errorCode = sqlite3_exec(_asSqlite3, K_QUERY_CREATE_SCHEMA, nullptr, nullptr, nullptr);
}

Connection::~Connection()
{
    DMIT_COM_ASSERT(sqlite3_close(_asSqlite3) == SQLITE_OK);
}

} // namespace dmit::db
