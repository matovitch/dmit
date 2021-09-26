#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"

#include "robin/map.hpp"

namespace dmit::sem
{

struct Context
{
    template <class Type>
    using TMap = robin::map::TMake<com::UniqueId,
                                   Type,
                                   com::unique_id::Hasher,
                                   com::unique_id::Comparator, 4, 3>;
    Context(ast::State& ast);

    ast::State& _ast;

    TMap<ast::node::Location> _factMap;
};

} // namespace dmit::sem
