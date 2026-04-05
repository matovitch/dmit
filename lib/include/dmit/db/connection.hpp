#pragma once

#include "dmit/nng/nng.hpp"

extern "C"
{
    #include "sqlite3.h"
}

#include <optional>

namespace dmit::db
{

struct Connection
{
    Connection(int& errorCode);

    ~Connection();

    std::optional<nng::Buffer> asNngBuffer();

    sqlite3* _asSqlite3 = nullptr;
};

} // namespace dmit::db
