#pragma once

#include "dmit/sem/context.hpp"

#include "dmit/ast/bundle.hpp"

namespace dmit::sem
{

void bind(ast::Bundle&, Context&);

} // namespace dmit::sem
