#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

namespace dmit::ast
{

void copyShallow(node::TIndex<node::Kind::VIEW> srceView,
                 State::NodePool                & srceNodePool,
                 node::TIndex<node::Kind::VIEW> destView,
                 State::NodePool                & destNodePool);

} // namespace dmit::ast
