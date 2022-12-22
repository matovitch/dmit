#pragma once

#include "dmit/lex/token.hpp"
#include "dmit/lex/state.hpp"

#include "dmit/com/unique_id.hpp"

#include "dmit/src/reader.hpp"

#include <cstdint>

namespace dmit::lex
{

static constexpr int INITIAL_NODE  = 0;
static constexpr int BLOCK_COMMENT = 1;
static constexpr int BLOCK_DATA    = 2;

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

        state.push(MATCH, reader.offset());

        if (!reader.isValid())
        {
            return;
        }

        tGoto<INITIAL_NODE>(reader, state);
    }
};

template <>
struct TNode<Token::QUOTE_SIMPLE>
{
    void operator()(src::Reader& reader, State& state) const
    {
        auto head = reader._head;

        while (reader.isValid() && reader.look() != '\'')
        {
            reader.advance();
        }

        auto size = static_cast<uint32_t>(reader._head - head);
        auto id = com::UniqueId{head, size};

        if (reader.isValid() && state._id == id)
        {
            state.push(Token::DATA, reader.offset());
            state._id = com::UniqueId{"#none"};
            reader.advance();
            return reader.isValid() ? tGoto<INITIAL_NODE>(reader, state)
                                    : [](){}();
        }
        else if (state._id == com::UniqueId{"#none"})
        {
            state._id = id;
            reader.advance();
        }

        return reader.isValid() ? tGoto<BLOCK_DATA>(reader, state)
                                : state.push(Token::UNKNOWN, reader.offset());
    }
};

template <>
struct TNode<Token::SLASH_STAR>
{
    void operator()(src::Reader& reader, State& state) const
    {
        state._count++;
        tGoto<BLOCK_COMMENT>(reader, state);
    }
};

template <>
struct TNode<Token::STAR_SLASH>
{
    void operator()(src::Reader& reader, State& state) const
    {
        state._count = state._count ? state._count - 1 : 0;

        if (state._count)
        {
            return tGoto<BLOCK_COMMENT>(reader, state);
        }

        state.push(Token::COMMENT, reader.offset());

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
            return state.push(MATCH, reader.offset());
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
