#pragma once

#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/state.hpp"

#include <cstdint>

namespace dmit::sem
{

void declareModulesAndLocateImports(ast::State& ast, FactMap& factMap);

void solveImports(ast::State& ast, FactMap& factMap);

} // namespace dmit::sem
