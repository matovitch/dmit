#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/pool.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/option_reference.hpp"

namespace dmit::ast
{

struct Bundle : fmt::Formatable
{
    using NodePool = node::TPool<0x10>;

    Bundle(NodePool&);

    NodePool& _nodePool;

    node::TRange<node::Kind::VIEW> _views;

    com::OptionRef<ast::State::NodePool> _interfacePoolOpt;
};

} // namespace dmit::ast
