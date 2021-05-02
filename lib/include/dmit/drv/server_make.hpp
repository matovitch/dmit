#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/db/database.hpp"

#include "dmit/fmt/logger.hpp"

#include "dmit/com/unique_id.hpp"

#include "robin/map.hpp"

#include <cstdint>
#include <vector>

namespace dmit::drv::srv
{

using AstMap = robin::map::TMake<com::UniqueId,
                                 dmit::ast::State,
                                 com::unique_id::Hasher,
                                 com::unique_id::Comparator, 4, 3>;

void make(dmit::nng::Socket& socket, dmit::db::Database& database)
{
    // 1. Process query

    int errorCode;

    std::vector<dmit::com::UniqueId  > unitIds ;
    std::vector<std::vector<uint8_t> > paths   ;
    std::vector<std::vector<uint8_t> > sources ;

    if ((errorCode = database.selectUnitIdsPathsSources(unitIds,
                                                        paths,
                                                        sources)) != SQLITE_OK)
    {
        displaySqlite3Error("selectUnitIdsPathsSources", errorCode);
        replyWith(socket, Reply::KO);
        return;
    }

    ast::FromPathAndSource astFromPathAndSource;
    AstMap                 astMap;

    for (int i = 0; i < paths.size(); i++)
    {
        astMap.emplace(unitIds[i], astFromPathAndSource.make(paths[i], sources[i]));

        DMIT_COM_LOG_OUT << astMap.at(unitIds[i]) << '\n';
    }

    // 2. Write reply

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
