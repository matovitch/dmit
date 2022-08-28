#include "dmit/ast/bundle.hpp"

#include <cstdint>

namespace dmit::ast
{

Bundle::Bundle(NodePool& nodePool) :
    _nodePool{nodePool}
{}

uint32_t Bundle::nbDefinition() const
{
    uint32_t result = 0;

    for (int i = 0; i < _views._size; i++)
    {
        auto& modules = _nodePool.get(_views[i])._modules;

        for (int j = 0; j < modules._size; j++)
        {
            result += _nodePool.get(modules[i])._definitions._size;
        }
    }

    return result;
}

} // namespace dmit::ast
