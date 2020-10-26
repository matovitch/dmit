#pragma once

#include "sqlite3/sqlite3.h"

namespace dmit::db
{

struct Connection
{
    Connection(int& errorCode);

    ~Connection();

    sqlite3* _asSqlite3 = nullptr;
};

} // namespace dmit::db
