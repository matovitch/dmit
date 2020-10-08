#pragma once

#include "dmit/ast/state.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

void analyze(const uint8_t* const source,
             const std::vector<uint32_t>& offsets,
             dmit::ast::State& ast);

} // namespace dmit::sem
