#include "dmit/db/db.hpp"

namespace dmit::db
{

int makeRamDb(Connection& connection)
{
    return sqlite3_open_v2(nullptr,
                           &(connection._asSqlite3),
                           SQLITE_OPEN_READWRITE | SQLITE_OPEN_MEMORY,
                           nullptr);
}

} // namespace dmit::db
