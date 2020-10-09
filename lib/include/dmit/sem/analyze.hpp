#pragma once

#include "dmit/ast/state.hpp"

#include "dmit/src/partition.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

void analyze(const dmit::src::Partition& partition,
             dmit::ast::State& ast);

} // namespace dmit::sem
