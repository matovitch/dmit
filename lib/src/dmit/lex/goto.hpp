#pragma once

namespace dmit::lex
{

template <class GotoPredicate, int GOTO_NEXT_NODE>
struct TGoto
{
    using Predicate = GotoPredicate;

    static constexpr int NEXT_NODE = GOTO_NEXT_NODE;
};

} // namespace dmit::lex
