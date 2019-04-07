#pragma once

namespace dmit
{

namespace lex
{

template <class GotoPredicate, int GOTO_NEXT_STATE_INDEX>
struct TGoto
{
    using Predicate = GotoPredicate;
    
    static constexpr int NEXT_STATE_INDEX = GOTO_NEXT_STATE_INDEX;
};

} // namespace lex

} // namespace dmit
