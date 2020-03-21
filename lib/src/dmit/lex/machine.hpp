#pragma once

#include "dmit/lex/predicate.hpp"
#include "dmit/lex/token.hpp"
#include "dmit/lex/node.hpp"
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
using IsBraLeft              = TIs<'{'>;
using IsKetLeft              = TIs<'<'>;
using IsParLeft              = TIs<'('>;
using IsBraRight             = TIs<'}'>;
using IsKetRight             = TIs<'>'>;
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
    NODE_INITIAL,
    NODE_WHITESPACE,
    NODE_IDENTIFIER,
    NODE_BRA_LEFT,
    NODE_KET_LEFT,
    NODE_PAR_LEFT,
    NODE_BRA_RIGHT,
    NODE_KET_RIGHT,
    NODE_PAR_RIGHT,
    NODE_SEMI_COLON,
    NODE_DOT,
    NODE_COMMA,
    NODE_COLON,
    NODE_EQUAL,
    NODE_PLUS,
    NODE_MINUS,
    NODE_STAR,
    NODE_SLASH,
    NODE_NUMBER,
    NODE_ARROW,
    NODE_DECIMAL_0,
    NODE_DECIMAL_1,
    NODE_DECIMAL_2,
    NODE_DECIMAL_3
};

template <>
struct TNodeIndex<NODE_INITIAL>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsWhitespace        , NODE_WHITESPACE >,
        TGoto<IsUnderscoreOrAlpha , NODE_IDENTIFIER >,
        TGoto<IsParLeft           , NODE_PAR_LEFT   >,
        TGoto<IsParRight          , NODE_PAR_RIGHT  >,
        TGoto<IsSemiColon         , NODE_SEMI_COLON >,
        TGoto<IsDot               , NODE_DOT        >,
        TGoto<IsComma             , NODE_COMMA      >,
        TGoto<IsEqual             , NODE_EQUAL      >,
        TGoto<IsColon             , NODE_COLON      >,
        TGoto<IsBraLeft           , NODE_BRA_LEFT   >,
        TGoto<IsBraRight          , NODE_BRA_RIGHT  >,
        TGoto<IsPlus              , NODE_PLUS       >,
        TGoto<IsMinus             , NODE_MINUS      >,
        TGoto<IsKetLeft           , NODE_KET_LEFT   >,
        TGoto<IsKetRight          , NODE_KET_RIGHT  >,
        TGoto<IsStar              , NODE_STAR       >,
        TGoto<IsSlash             , NODE_SLASH      >,
        TGoto<IsDigit             , NODE_NUMBER     >
    >;
};

template <> struct TNodeIndex<NODE_SEMI_COLON > { using Type = TNode<Token::SEMI_COLON >;};
template <> struct TNodeIndex<NODE_BRA_RIGHT  > { using Type = TNode<Token::BRA_RIGHT  >;};
template <> struct TNodeIndex<NODE_KET_RIGHT  > { using Type = TNode<Token::KET_RIGHT  >;};
template <> struct TNodeIndex<NODE_PAR_RIGHT  > { using Type = TNode<Token::PAR_RIGHT  >;};
template <> struct TNodeIndex<NODE_BRA_LEFT   > { using Type = TNode<Token::BRA_LEFT   >;};
template <> struct TNodeIndex<NODE_KET_LEFT   > { using Type = TNode<Token::KET_LEFT   >;};
template <> struct TNodeIndex<NODE_PAR_LEFT   > { using Type = TNode<Token::PAR_LEFT   >;};
template <> struct TNodeIndex<NODE_COLON      > { using Type = TNode<Token::COLON      >;};
template <> struct TNodeIndex<NODE_EQUAL      > { using Type = TNode<Token::EQUAL      >;};
template <> struct TNodeIndex<NODE_COMMA      > { using Type = TNode<Token::COMMA      >;};
template <> struct TNodeIndex<NODE_SLASH      > { using Type = TNode<Token::SLASH      >;};
template <> struct TNodeIndex<NODE_ARROW      > { using Type = TNode<Token::ARROW      >;};
template <> struct TNodeIndex<NODE_PLUS       > { using Type = TNode<Token::PLUS       >;};
template <> struct TNodeIndex<NODE_STAR       > { using Type = TNode<Token::STAR       >;};

template <>
struct TNodeIndex<NODE_WHITESPACE>
{
    using Type = TNode
    <
        Token::WHITESPACE,
        TGoto<IsWhitespace, NODE_WHITESPACE>
    >;
};

template <>
struct TNodeIndex<NODE_IDENTIFIER>
{
    using Type = TNode
    <
        Token::IDENTIFIER,
        TGoto<IsUnderscoreOrAlphaNum, NODE_IDENTIFIER>
    >;
};

template <>
struct TNodeIndex<NODE_MINUS>
{
    using Type = TNode
    <
        Token::MINUS,
        TGoto<IsKetRight, NODE_ARROW>
    >;
};

template <>
struct TNodeIndex<NODE_DOT>
{
    using Type = TNode
    <
        Token::DOT,
        TGoto<IsDigit, NODE_DECIMAL_0>
    >;
};

template <>
struct TNodeIndex<NODE_NUMBER>
{
    using Type = TNode
    <
        Token::INTEGER,
        TGoto<IsDigit , NODE_NUMBER    >,
        TGoto<IsDot   , NODE_DECIMAL_0 >
    >;
};

template <>
struct TNodeIndex<NODE_DECIMAL_0>
{
    using Type = TNode
    <
        Token::DECIMAL,
        TGoto<IsDigit    , NODE_DECIMAL_0>,
        TGoto<IsExponent , NODE_DECIMAL_1>
    >;
};

template <>
struct TNodeIndex<NODE_DECIMAL_1>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsPlusOrMinus , NODE_DECIMAL_2>,
        TGoto<IsDigit       , NODE_DECIMAL_3>
    >;
};

template <>
struct TNodeIndex<NODE_DECIMAL_2>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsDigit, NODE_DECIMAL_3>
    >;
};

template <>
struct TNodeIndex<NODE_DECIMAL_3>
{
    using Type = TNode
    <
        Token::DECIMAL,
        TGoto<IsDigit, NODE_DECIMAL_3>
    >;
};

} // namespace lex
} // namespace dmit
