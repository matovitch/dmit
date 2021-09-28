#include "dmit/sem/fact_map.hpp"

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"

#include <cstdint>

namespace dmit::sem
{

namespace fact_map
{

com::UniqueId next(com::UniqueId key)
{
    com::murmur::combine(key, key);

    return key;
}

} // namespace fact_map

void FactMap::emplace(com::UniqueId key,
                      ast::State::NodePool& astNodePool,
                      ast::node::Location location)
{
    auto fit = _asRobinMap.find(key);

    if (fit == _asRobinMap.end())
    {
        _asRobinMap.emplace(key, Fact{&astNodePool, location, 0});
        return;
    }

    fit->second._count++;

    emplace(fact_map::next(key), astNodePool, location);
}

} // namespace dmit::sem