#pragma once

#include "dmit/lex/token.hpp"
#include "dmit/lex/state.hpp"

#include "dmit/src/reader.hpp"

#include <cstdint>

namespace dmit::lex
{

static constexpr int INITIAL_NODE  = 0;
static constexpr int COMMENT_BLOCK = 1;

template <int INDEX>
struct TNodeIndex;

template <int INDEX>
using TGetState = typename TNodeIndex<INDEX>::Type;

template <int NODE_INDEX>
void tGoto(src::Reader& reader,
           State& state)
{
    TGetState<NODE_INDEX>{}(reader,
                            state);
}

template <uint8_t MATCH, class... GotosOrMatch>
struct TNode;

template <uint8_t MATCH>
struct TNode<MATCH>
{
    void operator()(src::Reader& reader, State& state) const
    {
        if constexpr (MATCH == Token::UNKNOWN)
        {
            reader.advance();
        }

        if constexpr (MATCH == Token::SLASH_STAR)
        {
            state._count++;
        }

        if constexpr (MATCH == Token::STAR_SLASH)
        {
            state._count = state._count ? state._count - 1 : 0;
        }

        if (state._count)
        {
            return tGoto<COMMENT_BLOCK>(reader, state);
        }

        state.push(MATCH, reader.offset());

        if (!reader.isValid())
        {
            return;
        }

        tGoto<INITIAL_NODE>(reader, state);
    }
};

template <uint8_t MATCH, class Goto, class... Gotos>
struct TNode<MATCH, Goto, Gotos...>
{
    using Predicate = typename Goto::Predicate;

    static constexpr int NEXT_NODE = Goto::NEXT_NODE;

    void operator()(src::Reader& reader, State& state) const
    {
        if (!reader.isValid())
        {
            state.push(MATCH, reader.offset());
            return;
        }

        if (!Predicate{}(reader.look()))
        {
            return TNode<MATCH, Gotos...>{}(reader, state);
        }

        reader.advance();

        tGoto<NEXT_NODE>(reader, state);
    }
};

} // namespace dmit::lex
