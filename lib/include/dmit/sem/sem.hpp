#pragma once

#include "dmit/sem/context.hpp"

namespace dmit::sem
{

void declareModulesAndLocateImports(Context& context);

void solveImports(Context& context);

} // namespace dmit::sem
