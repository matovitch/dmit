#pragma once

#include "dmit/ast/state.hpp"

#include "dmit/sem/context.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

void analyze(ast::State& ast,
             Context& context);

} // namespace dmit::sem
