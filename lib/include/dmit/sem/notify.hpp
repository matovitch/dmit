#pragma once

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/bundle.hpp"

namespace dmit::sem
{

void notify(ast::Bundle&, Context&, InterfaceMap&);

} // namespace dmit::sem