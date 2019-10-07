#pragma once

#include "dmit/lex/predicate.hpp"
#include "dmit/lex/token.hpp"
#include "dmit/lex/tstate.hpp"
#include "dmit/lex/goto.hpp"

namespace dmit
{

namespace lex
{

using IsDigit                = TIsBetween<'0', '9'>;
using IsDigitExceptZero      = TIsBetween<'1', '9'>;

using IsWhitespace           = TIsIn<' ', '\t', '\r', '\n'>;
using IsExponent             = TIsIn<'e', 'E'>;
using IsPlusOrMinus          = TIsIn<'+', '-'>;

using IsPlus                 = TIs<'+'>;
using IsMinus                = TIs<'-'>;
using IsStar                 = TIs<'*'>;
using IsSlash                = TIs<'/'>;
using IsParenL               = TIs<'('>;
using IsParenR               = TIs<')'>;
using IsDot                  = TIs<'.'>;
using IsEqual                = TIs<'='>;

using IsUnderscoreOrAlpha    = TOr<TIsBetween <'A', 'Z'>,
                                   TIsBetween <'a', 'z'>,
                                   TIs        <'_'>>;

using IsUnderscoreOrAlphaNum = TOr<IsUnderscoreOrAlpha,
                                   IsDigit>;

enum
{
    STATE_INITIAL,
    STATE_WHITESPACE,
    STATE_IDENTIFIER,
    STATE_PLUS,
    STATE_MINUS,
    STATE_STAR,
    STATE_SLASH,
    STATE_PAREN_L,
    STATE_PAREN_R,
    STATE_DOT,
    STATE_EQUAL,
    STATE_NUMBER,
    STATE_DECIMAL_0,
    STATE_DECIMAL_1,
    STATE_DECIMAL_2,
    STATE_DECIMAL_3
};

template <>
struct TStateIndex<STATE_INITIAL>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<IsWhitespace        , STATE_WHITESPACE >,
        TGoto<IsUnderscoreOrAlpha , STATE_IDENTIFIER >,
        TGoto<IsPlus              , STATE_PLUS       >,
        TGoto<IsMinus             , STATE_MINUS      >,
        TGoto<IsStar              , STATE_STAR       >,
        TGoto<IsSlash             , STATE_SLASH      >,
        TGoto<IsParenL            , STATE_PAREN_L    >,
        TGoto<IsParenR            , STATE_PAREN_R    >,
        TGoto<IsDot               , STATE_DOT        >,        
        TGoto<IsEqual             , STATE_EQUAL      >,        
        TGoto<IsDigit             , STATE_NUMBER     >
    >;
};

template <>
struct TStateIndex<STATE_WHITESPACE>
{
    using Type = TState
    <
        Token::WHITESPACE,
        TGoto<IsWhitespace, STATE_WHITESPACE>
    >;
};

template <>
struct TStateIndex<STATE_IDENTIFIER>
{
    using Type = TState
    <
        Token::IDENTIFIER,
        TGoto<IsUnderscoreOrAlphaNum, STATE_IDENTIFIER>
    >;
};

template <>
struct TStateIndex<STATE_PLUS>
{
    using Type = TState
    <
        Token::PLUS
    >;
};

template <>
struct TStateIndex<STATE_MINUS>
{
    using Type = TState
    <
        Token::MINUS
    >;
};

template <>
struct TStateIndex<STATE_STAR>
{
    using Type = TState
    <
        Token::STAR
    >;
};

template <>
struct TStateIndex<STATE_SLASH>
{
    using Type = TState
    <
        Token::SLASH
    >;
};

template <>
struct TStateIndex<STATE_PAREN_L>
{
    using Type = TState
    <
        Token::PAREN_L
    >;
};

template <>
struct TStateIndex<STATE_PAREN_R>
{
    using Type = TState
    <
        Token::PAREN_R
    >;
};

template <>
struct TStateIndex<STATE_DOT>
{
    using Type = TState
    <
        Token::DOT,
        TGoto<IsDigit, STATE_DECIMAL_0>
    >;
};

template <>
struct TStateIndex<STATE_EQUAL>
{
    using Type = TState
    <
        Token::EQUAL
    >;
};

template <>
struct TStateIndex<STATE_NUMBER>
{
    using Type = TState
    <
        Token::INTEGER,
        TGoto<IsDigit , STATE_NUMBER    >,
        TGoto<IsDot   , STATE_DECIMAL_0 >
    >;
};

template <>
struct TStateIndex<STATE_DECIMAL_0>
{
    using Type = TState
    <
        Token::DECIMAL,
        TGoto<IsDigit    , STATE_DECIMAL_0>,
        TGoto<IsExponent , STATE_DECIMAL_1>
    >;
};

template <>
struct TStateIndex<STATE_DECIMAL_1>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<IsPlusOrMinus , STATE_DECIMAL_2>,
        TGoto<IsDigit       , STATE_DECIMAL_3>
    >;
};

template <>
struct TStateIndex<STATE_DECIMAL_2>
{
    using Type = TState
    <
        Token::UNKNOWN,
        TGoto<IsDigit, STATE_DECIMAL_3>
    >;
};

template <>
struct TStateIndex<STATE_DECIMAL_3>
{
    using Type = TState
    <
        Token::DECIMAL,
        TGoto<IsDigit, STATE_DECIMAL_3>
    >;
};

} // namespace lex

} // namespace dmit
