#pragma once

#include "dmit/drv/server_reply.hpp"
#include "dmit/drv/reply.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/import_graph.hpp"
#include "dmit/sem/fact_map.hpp"
#include "dmit/sem/analyze.hpp"
#include "dmit/sem/bundle.hpp"

#include "dmit/ast/from_path_and_source.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/db/database.hpp"

#include "dmit/fmt/logger.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/unique_id.hpp"

#include <cstdint>
#include <vector>

namespace dmit::drv::srv
{

struct AstBuilder
{
    using ReturnType = ast::State;

    AstBuilder(const std::vector<std::vector<uint8_t>>& paths,
               const std::vector<std::vector<uint8_t>>& sources) :
        _paths{paths},
        _sources{sources}
    {}

    ast::State run(const uint64_t index)
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

struct BundleBuilder
{
    using ReturnType = ast::Bundle;

    BundleBuilder(const std::vector<com::UniqueId > & moduleOrder,
                  const std::vector<uint32_t      > & moduleBundles,
                  const sem::FactMap                & factMap) :
        _moduleOrder   {moduleOrder},
        _moduleBundles {moduleBundles},
        _factMap       {factMap}
    {}

    ast::Bundle run(const uint64_t index)
    {
        return sem::bundle::make(index,
                                 _moduleOrder,
                                 _moduleBundles,
                                 _factMap,
                                 _nodePool);
    }

    uint32_t size() const
    {
        return _moduleBundles.size() - 1;
    }

    const std::vector<com::UniqueId >& _moduleOrder;
    const std::vector<uint32_t      >& _moduleBundles;

    const sem::FactMap& _factMap;

    ast::State::NodePool _nodePool;
};

struct SemanticAnalysis
{
    using ReturnType = int8_t;

    SemanticAnalysis(sem::InterfaceMap        & interfaceMap,
                     std::vector<ast::Bundle> & bundles) :
        _interfaceMap{interfaceMap},
        _bundles{bundles}
    {}

    int8_t run(const uint64_t index)
    {
        if (index)
        {
            while (_interfaceAtomCount.load(std::memory_order_acquire) < index);

            sem::analyze(_bundles[index - 1]);
        }

        for (auto& bundle : _bundles)
        {
            _interfaceMap.registerBundle(bundle);

            int interfaceCount = _interfaceAtomCount.load(std::memory_order_relaxed);

            _interfaceAtomCount.store(interfaceCount + 1, std::memory_order_release);
        }

        return 0;
    }

    uint32_t size() const
    {
        return _bundles.size() + 1;
    }

    std::atomic<int> _interfaceAtomCount = 0;

    sem::InterfaceMap        & _interfaceMap;
    std::vector<ast::Bundle> & _bundles;
};

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

    std::vector<ast::State> asts;

    asts.reserve(unitIds.size());

    com::TParallelFor<AstBuilder> parallelAstBuilder(paths, sources);

    for (int i = 0; i < paths.size(); ++i)
    {
        asts.emplace_back(parallelAstBuilder.result(i));
    }

    // 3. Declare modules and resolve imports

    sem::FactMap factMap;

    for (auto& ast : asts)
    {
        factMap.declareModulesAndLocateImports(ast);
    }

    for (auto& ast : asts)
    {
        factMap.solveImports(ast);
    }

    // 4. Make the import graph

    sem::ImportGraph importGraph;

    for (auto& ast : asts)
    {
        importGraph.registerAst(ast);
    }

    std::vector<com::UniqueId > moduleOrder;
    std::vector<uint32_t      > moduleBundles;

    importGraph.makeBundles(moduleOrder,
                            moduleBundles);

    // 5. Make the bundles

    std::vector<ast::Bundle> bundles;

    bundles.reserve(moduleBundles.size() - 1);

    com::TParallelFor<BundleBuilder> parallelBundleBuilder(moduleOrder,
                                                           moduleBundles,
                                                           factMap);
    for (int i = 0; i < moduleBundles.size() - 1; ++i)
    {
        bundles.emplace_back(parallelBundleBuilder.result(i));
    }

    // 6. End of semantic analysis

    sem::InterfaceMap interfaceMap;

    com::TParallelFor<SemanticAnalysis> parallelSemanticAnalysis(interfaceMap,
                                                                 bundles);
    for (auto& bundle : bundles)
    {
        DMIT_COM_LOG_OUT << bundle << '\n';
    }

    // 7. Write reply

    replyWith(socket, Reply::OK);
}

} // namespace dmit::drv::srv
