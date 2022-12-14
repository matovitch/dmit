#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/gen/emitter.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/import_graph.hpp"
#include "dmit/sem/fact_map.hpp"
#include "dmit/sem/analyzer.hpp"
#include "dmit/sem/bundle.hpp"

#include "dmit/ast/builder.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/db/database.hpp"

#include "dmit/fmt/logger.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/unique_id.hpp"
#include "dmit/com/logger.hpp"

#include <cstdint>
#include <vector>

namespace dmit::drv::srv
{

void make(nng::Socket& socket, db::Database& database)
{
    // 1. Retrive sources from db

    int errorCode;

    std::vector<com::UniqueId        > unitIds ;
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

    // 2. Make the ASTs

    com::TParallelFor<ast::Builder> parallelAstBuilder{paths, sources};

    auto&& asts = parallelAstBuilder.makeVector();

    // 3. Resolve imports

    sem::FactMap factMap;

    factMap.solveImports(asts);

    // 4. Make the import graph

    sem::ImportGraph importGraph;

    importGraph.registerAsts(asts);

    std::vector<com::UniqueId > moduleOrder;
    std::vector<uint32_t      > moduleBundles;

    importGraph.makeBundles(moduleOrder,
                            moduleBundles);

    // 5. Make the bundles

    com::TParallelFor<sem::bundle::Builder> parallelBundleBuilder{moduleOrder,
                                                                  moduleBundles,
                                                                  factMap};
    auto&& bundles = parallelBundleBuilder.makeVector();

    // 6. Semantic analysis

    sem::InterfaceMap interfaceMap;

    com::TParallelFor<sem::Analyzer> parallelSemanticAnalyzer{interfaceMap,
                                                              bundles};
    sem::analyze(parallelSemanticAnalyzer);

    DMIT_COM_LOG_OUT << interfaceMap << '\n';

    for (auto& bundle : bundles)
    {
        DMIT_COM_LOG_OUT << bundle << '\n';
    }

    // 7. Code generation

    com::TParallelFor<gen::Emitter> parallelGenerationEmitter{bundles};
    auto&& bins = parallelGenerationEmitter.makeVector();

    // 8. Write reply

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
