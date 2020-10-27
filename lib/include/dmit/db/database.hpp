#pragma once

#include "dmit/db/query_register.hpp"
#include "dmit/db/connection.hpp"

#include "dmit/nng/nng.hpp"

#include "dmit/com/unique_id.hpp"

#include <optional>
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
    int hasUnit(const com::UniqueId& unitId, bool& result);

    int updateFileWithUnit(const com::UniqueId        & fileId,
                           const com::UniqueId        & unitId,
                           const std::vector<uint8_t> & unitSource);

    int insertFileWithUnit(const com::UniqueId        & fileId,
                           const com::UniqueId        & unitId,
                           const std::string          & filePath,
                           const std::vector<uint8_t> & unitSource);
    int clean();

    std::optional<nng::Buffer> asNngBuffer();

private:

    int transactionBegin();
    int transactionRollback();
    int transactionCommit();

    int insertFile(const com::UniqueId & fileId,
                   const com::UniqueId & fileUnitId,
                   const std::string   & filePath);

    int updateFile(const com::UniqueId & fileId,
                   const com::UniqueId & fileUnitId);

    int insertUnit(const com::UniqueId        & unitId,
                   const com::UniqueId        & fileId,
                   const std::vector<uint8_t> & source);

    Connection _connection;
    QueryRegister _queryRegister;
};

} // namespace dmit::db
