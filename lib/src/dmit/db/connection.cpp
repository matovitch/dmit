#include "dmit/db/connection.hpp"

#include "dmit/com/assert.hpp"

#include "sqlite3/sqlite3.h"

namespace dmit::db
{

Connection::~Connection()
{
    DMIT_COM_ASSERT(sqlite3_close(_asSqlite3) == SQLITE_OK);
}

} // namespace dmit::db
