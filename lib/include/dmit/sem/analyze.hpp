#pragma once

#include "dmit/sem/interface_map.hpp"


#include "dmit/ast/bundle.hpp"

#include <cstdint>

namespace dmit::sem
{

int8_t analyze(InterfaceMap&, ast::Bundle&);

} // namespace dmit::sem
