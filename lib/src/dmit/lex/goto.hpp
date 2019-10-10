#pragma once

namespace dmit
{

namespace lex
{

template <class GotoPredicate, int GOTO_NEXT_STATE>
struct TGoto
{
    using Predicate = GotoPredicate;

    static constexpr int NEXT_STATE = GOTO_NEXT_STATE;
};

} // namespace lex

} // namespace dmit
