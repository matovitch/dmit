#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/sem/import_graph.hpp"
#include "dmit/sem/fact_map.hpp"
#include "dmit/sem/sem.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/db/database.hpp"

#include "dmit/fmt/logger.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/unique_id.hpp"

#include "robin/map.hpp"

#include <cstdint>
#include <vector>

namespace dmit::drv::srv
{

using AstMap = robin::map::TMake<com::UniqueId,
                                 ast::State,
                                 com::unique_id::Hasher,
                                 com::unique_id::Comparator, 4, 3>;
struct AstBuilder
{
    using ReturnType = ast::State;

    AstBuilder(const std::vector<std::vector<uint8_t>>& paths,
               const std::vector<std::vector<uint8_t>>& sources) :
        _paths{paths},
        _sources{sources}
    {}

    dmit::ast::State run(const uint64_t index)
    {
        return _astFromPathAndSource.make(_paths   [index],
                                          _sources [index]);
    }

    uint32_t size() const
    {
        return _paths.size();
    }

    const std::vector<std::vector<uint8_t>>& _paths   ;
    const std::vector<std::vector<uint8_t>>& _sources ;

    ast::FromPathAndSource _astFromPathAndSource;
};

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

    dmit::com::TParallelFor<AstBuilder> parallelAstBuilder(paths, sources);
    AstMap astMap;

    for (int i = 0; i < paths.size(); i++)
    {
        astMap.emplace(unitIds[i], parallelAstBuilder.result(i));

        DMIT_COM_LOG_OUT << astMap.at(unitIds[i]) << '\n';
    }

    dmit::sem::FactMap factMap;

    for (auto it  = astMap.begin() ;
              it != astMap.end()   ; ++it)
    {
        dmit::sem::declareModulesAndLocateImports(it->second, factMap);
    }

    for (auto it  = astMap.begin() ;
              it != astMap.end()   ; ++it)
    {
        dmit::sem::solveImports(it->second, factMap);
    }

    dmit::sem::ImportGraph importGraph;

    for (auto it  = astMap.begin() ;
              it != astMap.end()   ; ++it)
    {
        importGraph.registerAst(it->second);
    }

    std::vector<com::UniqueId > moduleOrder;
    std::vector<uint32_t      > moduleBundles;

    importGraph.makeBundles(moduleOrder,
                            moduleBundles);

    DMIT_COM_LOG_OUT << "moduleOrder   .size(): " << moduleOrder   .size() << '\n';
    DMIT_COM_LOG_OUT << "moduleBundles .size(): " << moduleBundles .size() << '\n';

    // 2. Write reply

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
