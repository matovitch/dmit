#pragma once

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/bundle.hpp"


namespace dmit::sem
{

void check(ast::Bundle& bundle, Context& context, InterfaceMap& interfaceMap);

} // namespace dmit::sem
