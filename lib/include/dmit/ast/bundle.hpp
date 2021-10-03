#pragma once

#include "dmit/ast/pool.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/fmt/formatable.hpp"

namespace dmit::ast
{

struct Bundle : fmt::Formatable
{
    using NodePool = node::TPool<0x10>;

    Bundle(NodePool&);

    NodePool& _nodePool;

    node::TRange<node::Kind::VIEW> _views;
};

} // namespace dmit::ast
