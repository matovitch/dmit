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
using IsParLeft              = TIs<'('>;
using IsParRight             = TIs<')'>;
using IsDot                  = TIs<'.'>;
using IsComma                = TIs<','>;
using IsColon                = TIs<':'>;
using IsSemiColon            = TIs<';'>;
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
    STATE_PAR_LEFT,
    STATE_PAR_RIGHT,
    STATE_SEMI_COLON,
    STATE_DOT,
    STATE_COMMA,
    STATE_EQUAL,
    STATE_COLON,
    STATE_PLUS,
    STATE_MINUS,
    STATE_STAR,
    STATE_SLASH,
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
        TGoto<IsParLeft           , STATE_PAR_LEFT   >,
        TGoto<IsParRight          , STATE_PAR_RIGHT  >,
        TGoto<IsSemiColon         , STATE_SEMI_COLON >,
        TGoto<IsDot               , STATE_DOT        >,
        TGoto<IsComma             , STATE_COMMA      >,        
        TGoto<IsEqual             , STATE_EQUAL      >,
        TGoto<IsColon             , STATE_COLON      >,
        TGoto<IsPlus              , STATE_PLUS       >,
        TGoto<IsMinus             , STATE_MINUS      >,
        TGoto<IsStar              , STATE_STAR       >,
        TGoto<IsSlash             , STATE_SLASH      >,
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
struct TStateIndex<STATE_PAR_LEFT>
{
    using Type = TState
    <
        Token::PAR_LEFT
    >;
};

template <>
struct TStateIndex<STATE_PAR_RIGHT>
{
    using Type = TState
    <
        Token::PAR_RIGHT
    >;
};

template <>
struct TStateIndex<STATE_COMMA>
{
    using Type = TState
    <
        Token::COMMA
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
struct TStateIndex<STATE_COLON>
{
    using Type = TState
    <
        Token::COLON
    >;
};

template <>
struct TStateIndex<STATE_SEMI_COLON>
{
    using Type = TState
    <
        Token::SEMI_COLON
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
