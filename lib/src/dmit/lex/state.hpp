#pragma once

#include "dmit/lex/reader.hpp"
#include "dmit/lex/result.hpp"
#include "dmit/lex/token.hpp"

#include <cstdint>

namespace dmit
{

namespace lex
{

template <int INDEX>
struct TStateIndex;

template <int INDEX>
using TGetState = typename TStateIndex<INDEX>::Type;

template <int STATE_INDEX>
void tGotoState(Reader& reader,
                Result& result)
{
    TGetState<STATE_INDEX>{}(reader,
                             result);
}

template <uint8_t MATCH, class... GotosOrMatch>
struct TState;

template <uint8_t MATCH>
struct TState<MATCH>
{
    void operator()(Reader& reader, Result& result) const
    {
        if constexpr (MATCH == Token::UNKNOWN)
        {
            ++reader;    
        }
        
        result.push(MATCH, reader.offset());
        
        tGotoState<1>(reader, result);
    }
};

template <uint8_t MATCH, class Goto, class... Gotos>
struct TState<MATCH, Goto, Gotos...>
{
    using Predicate = typename Goto::Predicate;
    
    static constexpr int NEXT_STATE_INDEX = Goto::NEXT_STATE_INDEX;
    
    void operator()(Reader& reader, Result& result) const
    {
        if (!reader)
        {
            return;    
        }
        
        if (!Predicate{}(*reader))
        {
            return TState<MATCH, Gotos...>{}(reader, result);   
        }
        
        ++reader;
        
        tGotoState<NEXT_STATE_INDEX>(reader, result);
    }    
};

} // namespace lex

} // namespace dmit
