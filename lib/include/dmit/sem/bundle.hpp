#pragma once

#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem::bundle
{

ast::Bundle make(const uint64_t index,
                 const std::vector<com::UniqueId >& moduleOrder,
                 const std::vector<uint32_t      >& moduleBundles,
                 const FactMap& factMap,
                 ast::State::NodePool& nodePool);

} // namespace dmit::sem::bundle
