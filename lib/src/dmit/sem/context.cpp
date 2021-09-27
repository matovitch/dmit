#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"

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

void FactMap::emplace(com::UniqueId key, ast::node::Location value)
{
    auto fit = _asRobinMap.find(key);

    if (fit == _asRobinMap.end())
    {
        _asRobinMap.emplace(key, Fact{value, 0});
        return;
    }

    emplace(fact_map::next(key), value);
}

Context::Context(ast::State& ast) : _ast{ast} {}

} // namespace dmit::sem
