#pragma once

#include "sqlite3/sqlite3.h"

namespace dmit::sql
{

struct Connexion
{
    ~Connexion();

    sqlite3* _asSqlite3Ptr = nullptr;
};

} // namespace dmit::sql
