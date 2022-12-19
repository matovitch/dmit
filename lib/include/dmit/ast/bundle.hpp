#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/reference.hpp"

#include <cstdint>

namespace dmit::ast
{

struct Bundle : fmt::Formatable
{
    using NodePool = typename State::NodePool;

    Bundle(NodePool&);

    uint32_t nbDefinition() const;

    NodePool& _nodePool;

    node::TRange<node::Kind::VIEW> _views;

    com::TOptionRef<ast::State::NodePool> _interfacePoolOpt;
};

} // namespace dmit::ast
