#include "dmit/sem/analyze.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/src/partition.hpp"

#include "dmit/com/assert.hpp"
#include "dmit/com/murmur.hpp"
#include "dmit/com/enum.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

namespace
{

template <com::TEnumIntegerType<ast::node::Kind> KIND>
src::Slice getSlice(const ast::node::TIndex<KIND>& node,
                    const dmit::src::Partition& partition,
                    ast::State::NodePool& nodePool)
{
    return partition.getSlice(nodePool.get(nodePool.get(node)._lexeme)._index);
};

void analyze(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx,
             const dmit::src::Partition& partition,
             ast::State::NodePool& nodePool)
{
    auto& function = nodePool.get(functionIdx);

    const auto& functionName = getSlice(function._name, partition, nodePool);

    dmit::com::murmur::hash(functionName._head, functionName.size(), function._id);
}

} // namespace

void analyze(const dmit::src::Partition& partition,
             dmit::ast::State& ast)
{
    auto& functions = ast._program._functions;

    for (uint32_t i = 0; i < functions._size; i++)
    {
        analyze(functions[i], partition, ast._nodePool);
    }
}

} // namespace dmit::sem
