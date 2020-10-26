#pragma once

#include "dmit/db/query_register.hpp"
#include "dmit/db/connection.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace dmit::db
{

class Database
{

public:

    Database(int& errorCode);

    int hasFile(const com::UniqueId& fileId, bool& result);

    int insertFile(const com::UniqueId        & fileId,
                   const std::vector<uint8_t> & fileContent,
                   const std::string          & filePath);

    int updateFile(const com::UniqueId        & fileId,
                   const std::vector<uint8_t> & fileContent);

private:

    Connection _connection;
    QueryRegister _queryRegister;
};


} // namespace dmit::db
