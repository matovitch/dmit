#pragma once

#include "dmit/lex/reader.hpp"
#include "dmit/lex/result.hpp"
#include "dmit/lex/token.hpp"

#include <cstdint>

namespace dmit
{

namespace lex
{

static constexpr int INITIAL_STATE = 0;

template <int INDEX>
struct TStateIndex;

template <int INDEX>
using TGetState = typename TStateIndex<INDEX>::Type;

template <int STATE_INDEX>
void tGoto(Reader& reader,
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
        
        if (!reader)
        {
            return;
        }

        tGoto<INITIAL_STATE>(reader, result);
    }
};

template <uint8_t MATCH, class Goto, class... Gotos>
struct TState<MATCH, Goto, Gotos...>
{
    using Predicate = typename Goto::Predicate;
    
    static constexpr int NEXT_STATE = Goto::NEXT_STATE;
    
    void operator()(Reader& reader, Result& result) const
    {
        if (!reader)
        {
            result.push(MATCH, reader.offset());
            return;    
        }
        
        if (!Predicate{}(*reader))
        {
            return TState<MATCH, Gotos...>{}(reader, result);   
        }
        
        ++reader;
        
        tGoto<NEXT_STATE>(reader, result);
    }    
};

} // namespace lex

} // namespace dmit
