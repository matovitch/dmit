#include "dmit/sql/database_ram.hpp"

#include "dmit/sql/canary_buffer.hpp"

#include "dmit/com/assert.hpp"

#include "sqlite3/sqlite3.h"
#include "sqlite3/memvfs.h"

namespace dmit::sql
{

bool DatabaseRAM::_isVfsInitialized = false;

void DatabaseRAM::initVfs()
{
    sqlite3* mySqlite3;

    DMIT_COM_ASSERT(sqlite3_open(":memory:", &mySqlite3) == SQLITE_OK);

    DMIT_COM_ASSERT(sqlite3_memvfs_init(mySqlite3, nullptr, nullptr) == SQLITE_OK_LOAD_PERMANENTLY);

    sqlite3_close(mySqlite3);

    _isVfsInitialized = true;
}

DatabaseRAM::DatabaseRAM(CanaryBuffer& canaryBuffer)
{
    if (!_isVfsInitialized)
    {
        initVfs();
    }

    char* uri = sqlite3_mprintf("file:whatever?ptr=0x%p&max=%lld&freeonclose=0",
                                canaryBuffer._data,
                                (long long)canaryBuffer._size);

    DMIT_COM_ASSERT(sqlite3_open_v2(uri,
                                    &_connexion._asSqlite3Ptr,
                                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI,
                                    "memvfs") != SQLITE_OK);
    sqlite3_free(uri);
}

} // namespace dmit::sql
