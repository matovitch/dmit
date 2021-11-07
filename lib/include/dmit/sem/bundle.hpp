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

struct Builder
{
    using ReturnType = ast::Bundle;

    Builder(const std::vector<com::UniqueId > & moduleOrder,
            const std::vector<uint32_t      > & moduleBundles,
            const sem::FactMap                & factMap) :
        _moduleOrder   {moduleOrder},
        _moduleBundles {moduleBundles},
        _factMap       {factMap}
    {}

    ast::Bundle run(const uint64_t index)
    {
        return sem::bundle::make(index,
                                 _moduleOrder,
                                 _moduleBundles,
                                 _factMap,
                                 _nodePool);
    }

    uint32_t size() const
    {
        return _moduleBundles.size() - 1;
    }

    const std::vector<com::UniqueId >& _moduleOrder;
    const std::vector<uint32_t      >& _moduleBundles;

    const sem::FactMap& _factMap;

    ast::State::NodePool _nodePool;
};

} // namespace dmit::sem::bundle
