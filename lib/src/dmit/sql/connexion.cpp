#include "dmit/sql/connexion.hpp"

#include "sqlite3/sqlite3.h"

namespace dmit::sql
{

Connexion::~Connexion()
{
    if (_asSqlite3Ptr)
    {
        sqlite3_close(_asSqlite3Ptr);
    }
}

} // namespace dmit::sql
