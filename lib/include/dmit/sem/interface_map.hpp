#pragma once

#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"

#include "robin/map.hpp"

#include "pool/pool.hpp"

namespace dmit::sem
{

struct InterfaceMap
{
    InterfaceMap(const std::vector<ast::Bundle>& bundles, ast::State::NodePool& astNodePool);

    void registerBundle(ast::Bundle& bundle);

    using Map = robin::map::TMake<com::UniqueId,
                                  ast::node::TIndex<ast::node::Kind::VIEW>,
                                  com::unique_id::Hasher,
                                  com::unique_id::Comparator, 4, 3>;
    Map _asSimpleMap;
    ast::State::NodePool& _astNodePool;

    pool::TMake<ast::node::TRange<ast::node::Kind::VIEW>, 1> _viewsPool;
};

} // namespace dmit::sem
