#include "dmit/db/query_register.hpp"

#include "dmit/db/connection.hpp"

extern "C"
{
    #include "sqlite3/sqlite3.h"
}

#include <cstdint>

namespace dmit::db
{

static const char K_QUERY_TRANSACTION_BEGIN[] =
R"(
    BEGIN TRANSACTION;
)";

static const char K_QUERY_TRANSACTION_ROLLBACK[] =
R"(
    ROLLBACK;
)";

static const char K_QUERY_TRANSACTION_COMMIT[] =
R"(
    COMMIT;
)";

static const char K_QUERY_SELECT_FILE[] =
R"(
    SELECT 1 FROM Files WHERE file_id = ?1;
)";

static const char K_QUERY_INSERT_FILE[] =
R"(
    INSERT INTO Files (file_id, unit_id, path) VALUES (?1, ?2, ?3);
)";

static const char K_QUERY_UPDATE_FILE[] =
R"(
    UPDATE Files SET unit_id = ?2 WHERE file_id = ?1;
)";

static const char K_QUERY_SELECT_UNIT[] =
R"(
    SELECT 1 FROM Units WHERE unit_id = ?2;
)";

static const char K_QUERY_INSERT_UNIT[] =
R"(
    INSERT INTO Units (unit_id, source) VALUES (?2, ?4);
)";

static const char K_QUERY_CLEAN[] =
R"(
    DELETE FROM Units WHERE Units.unit_id IN (
        SELECT Units.unit_id FROM Units LEFT JOIN Files ON
            Files.unit_id = Units.unit_id
        WHERE Files.unit_id IS NULL
    );
)";

static const char K_QUERY_SELECT_UNIT_IDS_PATHS_SOURCES[] =
R"(
    SELECT Files.unit_id,
           Files.path,
           Units.source FROM Files INNER JOIN Units ON Files.unit_id =
                                                       Units.unit_id;
)";

static const char* K_QUERIES[QueryRegister::SIZE] =
{
    K_QUERY_TRANSACTION_BEGIN,
    K_QUERY_TRANSACTION_ROLLBACK,
    K_QUERY_TRANSACTION_COMMIT,
    K_QUERY_SELECT_FILE,
    K_QUERY_INSERT_FILE,
    K_QUERY_UPDATE_FILE,
    K_QUERY_SELECT_UNIT,
    K_QUERY_INSERT_UNIT,
    K_QUERY_CLEAN,
    K_QUERY_SELECT_UNIT_IDS_PATHS_SOURCES
};

static const std::size_t K_QUERY_SIZES[QueryRegister::SIZE] =
{
    sizeof(K_QUERY_TRANSACTION_BEGIN),
    sizeof(K_QUERY_TRANSACTION_ROLLBACK),
    sizeof(K_QUERY_TRANSACTION_COMMIT),
    sizeof(K_QUERY_SELECT_FILE),
    sizeof(K_QUERY_INSERT_FILE),
    sizeof(K_QUERY_UPDATE_FILE),
    sizeof(K_QUERY_SELECT_UNIT),
    sizeof(K_QUERY_INSERT_UNIT),
    sizeof(K_QUERY_CLEAN),
    sizeof(K_QUERY_SELECT_UNIT_IDS_PATHS_SOURCES)
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
