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
        TRANSACTION_BEGIN,
        TRANSACTION_ROLLBACK,
        TRANSACTION_COMMIT,
        SELECT_FILE,
        INSERT_FILE,
        UPDATE_FILE,
        SELECT_UNIT,
        INSERT_UNIT,
        CLEAN,
        SIZE
    };

    enum
    {
        FILE_ID = 1,
        UNIT_ID,
        FILE_PATH,
        UNIT_SOURCE
    };

    QueryRegister(Connection&, int& errorCode);

    sqlite3_stmt* operator[](int index);

    ~QueryRegister();

    std::array<sqlite3_stmt*, SIZE> _asSqlite3Stmts;
};

} // namespace dmit::db
