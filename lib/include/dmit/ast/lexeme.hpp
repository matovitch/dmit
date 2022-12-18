#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/src/slice.hpp"

#include "dmit/fmt/lex/token.hpp"

#include "dmit/com/assert.hpp"

namespace dmit::ast::lexeme
{

template <uint8_t LOG2_SIZE>
src::Slice getSlice(const node::TIndex<ast::node::Kind::LEXEME>& lexemeIdx,
                    const node::TPool<LOG2_SIZE>& nodePool)
{
    const auto& lexeme = nodePool.get(lexemeIdx);

    const auto& source = nodePool.get(lexeme._source);

    const auto lexIdxBis = lexeme._index;
    const auto& lexOffsets = source._lexOffsets;

    DMIT_COM_ASSERT(lexIdxBis > 1);

    const auto head   = source._srcContent.value().data() + lexOffsets[0];
    const auto offset = lexOffsets.data() + lexOffsets.size() - 1 - lexIdxBis;

    // Mutable member write
    return src::Slice{{},
                      head - *(offset - 1),
                      head - *(offset - 0)};
}

template <uint8_t LOG2_SIZE>
lex::Token getToken(const node::TIndex<ast::node::Kind::LEXEME>& lexemeIdx,
                    const node::TPool<LOG2_SIZE>& nodePool)
{
    const auto& lexeme = nodePool.get(lexemeIdx);

    if (lexeme._token)
    {
        return lexeme._token.value();
    }

    const auto& tokens = nodePool.get(lexeme._source)._lexTokens;

    // Mutable member write
    lexeme._token = tokens[tokens.size() - 1 - lexeme._index];

    return lexeme._token.value();
}

} // namespace dmit::ast::lexeme
