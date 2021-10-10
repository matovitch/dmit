#include "dmit/sem/bundle.hpp"

#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/copy_deep.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem::bundle
{

void makeView(const com::UniqueId  & viewId,
              const FactMap        & factMap,
              ast::node::TIndex<ast::node::Kind::VIEW> viewIdx,
              ast::State::NodePool & nodePool)
{
    auto& view = nodePool.get(viewIdx);

    com::blit(viewId, view._id);

    auto key = viewId;

    auto fit = factMap._asRobinMap.find(key);

    auto count = fit->second._count;

    nodePool.make(view._modules, count);

    do
    {
        ast::copyDeep(as<ast::node::Kind::MODULE>(fit->second._index),
                      *(fit->second._nodePool),
                      view._modules[--count],
                      nodePool);

        com::murmur::combine(key, key);

        fit = factMap._asRobinMap.find(key);
    }
    while (fit != factMap._asRobinMap.end());
}

ast::Bundle make(const uint64_t index,
                 const std::vector<com::UniqueId > & moduleOrder,
                 const std::vector<uint32_t      > & moduleBundles,
                 const FactMap& factMap,
                 ast::State::NodePool& nodePool)
{
    ast::Bundle bundle{nodePool};

    const auto bundleHead = moduleOrder.data() + index;

    const int64_t bundleSize = moduleBundles[index + 1] -
                               moduleBundles[index + 0];

    nodePool.make(bundle._views, bundleSize);

    for (int i = 0; i < bundleSize; ++i)
    {
        makeView(bundleHead[i], factMap, bundle._views[i], nodePool);
    }

    return bundle;
}

} // namespace dmit::sem::bundle
