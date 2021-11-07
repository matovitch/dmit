#pragma once

#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"

#include "topo/pool_set.hpp"
#include "topo/graph.hpp"

#include "robin/map.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

struct ImportGraph
{
    using TopoGraph   = topo::graph::TMake<com::UniqueId, 4>;
    using TopoPoolSet = typename TopoGraph::PoolSet;
    using Module      = typename TopoGraph::NodeListIt;
    using Dependency  = typename TopoGraph::EdgeListIt;

    using ModuleMap = robin::map::TMake<com::UniqueId,
                                        Module,
                                        com::unique_id::Hasher,
                                        com::unique_id::Comparator,
                                        3,
                                        4>;
    ImportGraph();

    Module getOrMakeModule(const com::UniqueId& moduleId);

    void makeDependency(const com::UniqueId& lhsModuleId,
                        const com::UniqueId& rhsModuleId);

    void makeBundles(std::vector<com::UniqueId> & moduleOrder,
                     std::vector<uint32_t     > & moduleBundles);

    void registerAst(ast::State& ast);

    void registerAsts(std::vector<ast::State>& asts);

    TopoPoolSet _topoPoolSet;
    TopoGraph   _topoGraph;

    ModuleMap _moduleMap;
};

} // namespace dmit::sem
