#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

namespace dmit::ast
{

void deepCopy(node::TIndex<node::Kind::MODULE>   srceModule,
              State::NodePool                  & srceNodePool,
              node::TIndex<node::Kind::MODULE>   destModule,
              State::NodePool                  & destNodePool);

} // namespace dmit::ast
