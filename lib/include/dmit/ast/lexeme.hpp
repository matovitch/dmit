#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/src/slice.hpp"

#include "dmit/com/assert.hpp"

namespace dmit::ast
{

template <uint8_t LOG2_SIZE>
src::Slice getSliceFromLexeme(const node::TIndex<ast::node::Kind::LEXEME>& lexemeIdx,
                              const node::TPool<LOG2_SIZE>& nodePool)
{
    const auto& lexeme = nodePool.get(lexemeIdx);

    const auto& source = nodePool.get(lexeme._source);

    const auto lexIdxBis = lexeme._index;
    const auto& lexOffsets = source._lexOffsets;

    DMIT_COM_ASSERT(lexIdxBis > 1);

    const auto head   = source._srcContent.data() + lexOffsets[0];
    const auto offset = lexOffsets.data() + lexOffsets.size() - 1 - lexIdxBis;

    return src::Slice
    {
        {},
        head - *(offset - 1),
        head - *(offset - 0)
    };
}

} // namespace dmit::ast
