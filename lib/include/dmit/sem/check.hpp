#pragma once

#include "dmit/sem/context.hpp"

#include "dmit/ast/bundle.hpp"


namespace dmit::sem
{

void check(ast::Bundle& bundle, Context& context);

} // namespace dmit::sem
