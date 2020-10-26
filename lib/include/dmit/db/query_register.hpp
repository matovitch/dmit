#pragma once

#include "dmit/db/connection.hpp"

#include "sqlite3/sqlite3.h"

#include <array>

namespace dmit::db
{

struct QueryRegister
{
    enum
    {
        SELECT_FILE,
        INSERT_FILE,
        UPDATE_FILE,
        SIZE
    };

    enum
    {
        FILE_ID = 1,
        FILE_PATH,
        FILE_CONTENT
    };

    QueryRegister(Connection&, int& errorCode);

    sqlite3_stmt* operator[](int index);

    ~QueryRegister();

    std::array<sqlite3_stmt*, SIZE> _asSqlite3Stmts;
};

} // namespace dmit::db
