#include "dmit/db/query_register.hpp"

#include "dmit/db/connection.hpp"

#include "sqlite3/sqlite3.h"

#include <cstdint>

namespace dmit::db
{

static const char K_QUERY_SELECT_FILE[] =
R"(
    SELECT path, content FROM Files WHERE id = ?1;
)";

static const char K_QUERY_INSERT_FILE[] =
R"(
    INSERT INTO Files (id, path, content) VALUES (?1, ?2, ?3);
)";

static const char K_QUERY_UPDATE_FILE[] =
R"(
    UPDATE Files SET content = ?3 WHERE id = ?1;
)";

static const char* K_QUERIES[QueryRegister::SIZE] =
{
    K_QUERY_SELECT_FILE,
    K_QUERY_INSERT_FILE,
    K_QUERY_UPDATE_FILE
};

static const std::size_t K_QUERY_SIZES[QueryRegister::SIZE] =
{
    sizeof(K_QUERY_SELECT_FILE),
    sizeof(K_QUERY_INSERT_FILE),
    sizeof(K_QUERY_UPDATE_FILE)
};

QueryRegister::QueryRegister(Connection& connection, int& errorCode) :
    _asSqlite3Stmts{}
{
    if (errorCode != SQLITE_OK)
    {
        return;
    }

    for (int i = 0; i < SIZE; i++)
    {
        if ((errorCode = sqlite3_prepare_v2(connection._asSqlite3,
                                            K_QUERIES         [i],
                                            K_QUERY_SIZES     [i],
                                            &(_asSqlite3Stmts [i]),
                                            nullptr)) != SQLITE_OK)
        {
            break; // An error occur. Stopping there.
        }
    }
}

sqlite3_stmt* QueryRegister::operator[](int index)
{
    return _asSqlite3Stmts[index];
}


QueryRegister::~QueryRegister()
{
    for (int i = 0; i < SIZE; i++)
    {
        auto sqlite3Stmt = _asSqlite3Stmts[i];

        if (sqlite3Stmt)
        {
            sqlite3_finalize(sqlite3Stmt);
        }
    }
}

} // namespace dmit::db
