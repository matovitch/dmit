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
#include "dmit/com/storage.hpp"
#include "dmit/com/logger.hpp"

#include <filesystem>
#include <cstdint>
#include <vector>

namespace dmit::drv::srv
{

void make(nng::Socket& socket, db::Database& database, com::parallel_for::ThreadPool& threadPool)
{
    // 1. Retrive sources from db

    int errorCode;

    std::vector<com::UniqueId          > unitIds ;
    std::vector<std::filesystem::path  > paths   ;
    std::vector<com::TStorage<uint8_t >> sources ;

    if ((errorCode = database.selectUnitIdsPathsSources(unitIds,
                                                        paths,
                                                        sources)) != SQLITE_OK)
    {
        displaySqlite3Error("selectUnitIdsPathsSources", errorCode);
        replyWith(socket, Reply::KO);
        return;
    }

    // 2. Make the ASTs

    dmit::com::parallel_for::TThreadContexts<dmit::ast::FromPathAndSource> astThreadContexts{threadPool};

    dmit::com::TParallelFor<dmit::ast::Builder> parallelAstBuilder{astThreadContexts, paths, sources};

    threadPool.notify_and_wait(parallelAstBuilder);

    auto&& asts = parallelAstBuilder._outputs;

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

    com::parallel_for::TThreadContexts<ast::State::NodePool> bundleThreadContexts{threadPool};

    com::TParallelFor<sem::bundle::Builder> parallelBundleBuilder{bundleThreadContexts,
                                                                  moduleOrder,
                                                                  moduleBundles,
                                                                  factMap};
    threadPool.notify_and_wait(parallelBundleBuilder);

    auto&& bundles = parallelBundleBuilder._outputs;

    // 6. Semantic analysis

    sem::InterfaceMap interfaceMap;

    com::parallel_for::TThreadContexts<sem::Context> semThreadContexts{threadPool};

    com::TParallelFor<sem::Analyzer> parallelSemanticAnalyzer{semThreadContexts,
                                                              interfaceMap,
                                                              bundles};
    sem::analyze(threadPool, parallelSemanticAnalyzer);

    DMIT_COM_LOG_OUT << interfaceMap << '\n';

    for (auto& bundle : bundles)
    {
        DMIT_COM_LOG_OUT << bundle << '\n';
    }

    // 7. Code generation

    dmit::com::parallel_for::TThreadContexts<dmit::gen::PoolWasm> genThreadContexts{threadPool};

    dmit::com::TParallelFor<dmit::gen::EmitterNew> parallelGenerationEmitter{genThreadContexts,
                                                                                bundles};
    threadPool.notify_and_wait(parallelGenerationEmitter);

    auto&& bins = parallelGenerationEmitter._outputs;

    // 8. Write reply

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
