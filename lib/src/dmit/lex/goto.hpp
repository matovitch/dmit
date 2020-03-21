#pragma once

namespace dmit
{

namespace lex
{

template <class GotoPredicate, int GOTO_NEXT_NODE>
struct TGoto
{
    using Predicate = GotoPredicate;

    static constexpr int NEXT_NODE = GOTO_NEXT_NODE;
};

} // namespace lex
} // namespace dmit
