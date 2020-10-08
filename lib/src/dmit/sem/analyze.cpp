#include "dmit/sem/analyze.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/com/assert.hpp"
#include "dmit/com/murmur.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

namespace
{

struct Lexeme
{
    uint32_t size() const
    {
        return static_cast<uint32_t>(_tail - _head);
    }

    const uint8_t* const _head;
    const uint8_t* const _tail;
};

Lexeme getLexeme(const uint32_t index,
                 const std::vector<uint32_t> offsets,
                 const uint8_t* const source)
{
    DMIT_COM_ASSERT(index > 1);

    const auto head   = source + offsets[0];
    const auto offset = offsets.data() + offsets.size() - 1 - index;

    return Lexeme{head - *(offset - 1),
                  head - *(offset - 0)};
}

void analyze(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx,
             const std::vector<uint32_t>& offsets,
             const uint8_t* const source,
             ast::State::NodePool& nodePool)
{
    auto& function = nodePool.get(functionIdx);

    const uint32_t functionNameAsIndex =
        nodePool.get(
            nodePool.get(
                function._name
            )._lexeme
        )._index;

    const auto& functionName = getLexeme(functionNameAsIndex, offsets, source);

    dmit::com::murmur::hash(functionName._head, functionName.size(), function._id);
}

} // namespace

void analyze(const uint8_t* const source,
             const std::vector<uint32_t>& offsets,
             dmit::ast::State& ast)
{
    auto& functions = ast._program._functions;

    for (uint32_t i = 0; i < functions._size; i++)
    {
        analyze(functions[i], offsets, source, ast._nodePool);
    }
}

} // namespace dmit::sem
