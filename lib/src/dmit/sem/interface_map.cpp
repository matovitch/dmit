#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/copy_shallow.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include <vector>

namespace dmit::sem
{

InterfaceMap::InterfaceMap(const std::vector<ast::Bundle>& bundles, ast::State::NodePool& astNodePool) :
    _astNodePool{astNodePool}
{
    for (const auto& bundle : bundles)
    {
        if (!bundle._views._size)
        {
            continue;
        }

        auto& views = _viewsPool.make();

        _astNodePool.make(views, bundle._views._size);

        for (uint32_t i = 0; i < bundle._views._size; i++)
        {
            _asSimpleMap.emplace(bundle._nodePool.get(bundle._views[i])._id, views[i]);
        }
    }
}

void InterfaceMap::registerBundle(ast::Bundle& bundle)
{
    // 1. Do the semantic analysis

    // 2. Copy the views

    for (uint32_t i = 0; i < bundle._views._size; i++)
    {
        const auto viewId = bundle._nodePool.get(bundle._views[i])._id;

        ast::copyShallow(bundle._views[i],
                         bundle._nodePool,
                         _asSimpleMap.at(viewId),
                         _astNodePool);
    }
}

} // namespace dmit::sem
