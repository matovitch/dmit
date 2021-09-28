#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"

#include "robin/map.hpp"

namespace dmit::sem
{

namespace fact_map
{

com::UniqueId next(com::UniqueId key);

} // namespace fact_map

struct Fact
{
    ast::State::NodePool* _nodePool;
    ast::node::Location   _location;
    uint32_t              _count;
};

struct FactMap
{
    template <class Type>
    using TMap = robin::map::TMake<com::UniqueId,
                                   Type,
                                   com::unique_id::Hasher,
                                   com::unique_id::Comparator, 4, 3>;

    void emplace(com::UniqueId, ast::State::NodePool&, ast::node::Location);

    TMap<Fact> _asRobinMap;
};

} // namespace dmit::sem
