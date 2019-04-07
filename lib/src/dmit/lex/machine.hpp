#pragma once

#include "dmit/lex/predicate.hpp"
#include "dmit/lex/token.hpp"
#include "dmit/lex/state.hpp"
#include "dmit/lex/goto.hpp"

namespace dmit
{

namespace lex
{

using isDigit           = TIsBetween<'0', '9'>;
using isDigitExceptZero = TIsBetween<'1', '9'>;

using isExponent        = TIsIn<'e', 'E'>;
using isPlusOrMinus     = TIsIn<'+', '-'>;

using isDot             = TIs<'.'>;

template <>
struct TStateIndex<1>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<isDigitExceptZero , 2>,
        TGoto<isPlusOrMinus     , 3>,
        TGoto<isDot             , 4>
    >;
};

template <>
struct TStateIndex<2>
{
    using Type = TState
    <
        Token::INT,
        TGoto<isDigit , 2>,
        TGoto<isDot   , 5>
    >;
};

template <>
struct TStateIndex<3>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<isDigitExceptZero , 2>,
        TGoto<isDot             , 4>
    >;
};

template <>
struct TStateIndex<4>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<isDigit , 5>
    >;
};

template <>
struct TStateIndex<5>
{
    using Type = TState
    <
        Token::FLOAT,
        TGoto<isDigit    , 5>,
        TGoto<isExponent , 6>
    >;
};

template <>
struct TStateIndex<6>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<isPlusOrMinus , 7>,
        TGoto<isDigit       , 8>
    >;
};

template <>
struct TStateIndex<7>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<isDigit, 8>
    >;
};

template <>
struct TStateIndex<8>
{
    using Type = TState
    <
        Token::FLOAT,
        TGoto<isDigit, 8>
    >;
};

} // namespace lex

} // namespace dmit
