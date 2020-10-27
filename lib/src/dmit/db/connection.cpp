#include "dmit/db/connection.hpp"

#include "dmit/com/assert.hpp"

#include "sqlite3/sqlite3.h"

#include <optional>
#include <cstring>
#include <cstdint>

namespace dmit::db
{

static const char K_QUERY_CREATE_SCHEMA[] =
R"(
    CREATE TABLE Files (
        file_id BLOB PRIMARY KEY,
        unit_id BLOB NOT NULL,
        path    TEXT NOT NULL
    ) WITHOUT ROWID;

    CREATE TABLE Units (
        unit_id BLOB PRIMARY KEY,
        file_id BLOB NUT NULL,
        source  BLOB,
        module  BLOB,
        image   BLOB,
        FOREIGN KEY(file_id) REFERENCES Files(file_id)
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

std::optional<nng::Buffer> Connection::asNngBuffer()
{
    sqlite3_int64 size;

    auto asBytes = sqlite3_serialize(_asSqlite3, "main", &size, 0);

    if (!asBytes)
    {
        return std::nullopt;
    }

    nng::Buffer nngBuffer{static_cast<size_t>(size)};

    memcpy(nngBuffer._asBytes, asBytes, size);

    sqlite3_free(asBytes);

    return std::optional<nng::Buffer>{std::move(nngBuffer)};
}

} // namespace dmit::db
