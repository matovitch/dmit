#include "dmit/ast/lexeme.hpp"

#include "dmit/com/assert.hpp"

namespace dmit::ast
{

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

} // namespace dmit::ast
