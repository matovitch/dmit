#pragma once

#include "dmit/lex/predicate.hpp"
#include "dmit/lex/token.hpp"
#include "dmit/lex/node.hpp"
#include "dmit/lex/goto.hpp"

namespace dmit::lex
{

using IsDigit                = TIsBetween<'0', '9'>;
using IsDigitExceptZero      = TIsBetween<'1', '9'>;

using IsWhitespace           = TIsIn<' ', '\t', '\r', '\n'>;
using IsExponent             = TIsIn<'e', 'E'>;
using IsPlusOrMinus          = TIsIn<'+', '-'>;
using IsNewLine              = TIs<'\n'>;

using IsPlus                 = TIs<'+'>;
using IsMinus                = TIs<'-'>;
using IsStar                 = TIs<'*'>;
using IsSlash                = TIs<'/'>;
using IsPercent              = TIs<'%'>;
using IsLeftBra              = TIs<'{'>;
using IsLeftKet              = TIs<'<'>;
using IsLeftPar              = TIs<'('>;
using IsLeftSqr              = TIs<'['>;
using IsRightBra             = TIs<'}'>;
using IsRightKet             = TIs<'>'>;
using IsRightPar             = TIs<')'>;
using IsRightSqr             = TIs<']'>;
using IsDot                  = TIs<'.'>;
using IsComma                = TIs<','>;
using IsColon                = TIs<':'>;
using IsSemiColon            = TIs<';'>;
using IsEqual                = TIs<'='>;
using IsBang                 = TIs<'!'>;
using IsQuestion             = TIs<'?'>;
using IsTilde                = TIs<'~'>;
using IsCaret                = TIs<'^'>;
using IsPipe                 = TIs<'|'>;
using IsAmpersand            = TIs<'&'>;
using IsQuoteDouble          = TIs<'"'>;
using IsQuoteSimple          = TIs<'\''>;

using IsUnderscoreOrAlpha    = TOr<TIsBetween <'A', 'Z'>,
                                   TIsBetween <'a', 'z'>,
                                   TIs        <'_'>>;

using IsUnderscoreOrAlphaNum = TOr<IsUnderscoreOrAlpha,
                                   IsDigit>;

enum
{
    NODE_INITIAL,
    NODE_BLOCK_COMMENT,
    NODE_BLOCK_DATA,
    NODE_COMMENT_SLASH,
    NODE_COMMENT_LINE,
    NODE_COMMENT_STAR,
    NODE_COMMENT,
    NODE_WHITESPACE,
    NODE_IDENTIFIER,
    NODE_BRA_LEFT,
    NODE_KET_LEFT,
    NODE_PAR_LEFT,
    NODE_SQR_LEFT,
    NODE_BRA_RIGHT,
    NODE_KET_RIGHT,
    NODE_PAR_RIGHT,
    NODE_SQR_RIGHT,
    NODE_SEMI_COLON,
    NODE_DOT,
    NODE_COMMA,
    NODE_COLON,
    NODE_EQUAL,
    NODE_PLUS,
    NODE_MINUS,
    NODE_STAR,
    NODE_SLASH,
    NODE_PERCENT,
    NODE_BANG,
    NODE_QUESTION,
    NODE_TILDE,
    NODE_CARET,
    NODE_PIPE,
    NODE_AMPERSAND,
    NODE_NUMBER,
    NODE_MINUS_KET_RIGHT,
    NODE_KET_RIGHT_EQUAL,
    NODE_KET_LEFT_EQUAL,
    NODE_KET_LEFT_BIS,
    NODE_KET_RIGHT_BIS,
    NODE_BANG_EQUAL,
    NODE_PLUS_EQUAL,
    NODE_MINUS_EQUAL,
    NODE_STAR_EQUAL,
    NODE_SLASH_EQUAL,
    NODE_PERCENT_EQUAL,
    NODE_KET_LEFT_BIS_EQUAL,
    NODE_KET_RIGHT_BIS_EQUAL,
    NODE_AMPERSAND_EQUAL,
    NODE_CARET_EQUAL,
    NODE_PIPE_EQUAL,
    NODE_EQUAL_BIS,
    NODE_PIPE_BIS,
    NODE_AMPERSAND_BIS,
    NODE_COLON_BIS,
    NODE_DECIMAL_0,
    NODE_DECIMAL_1,
    NODE_DECIMAL_2,
    NODE_DECIMAL_3,
    NODE_SLASH_STAR,
    NODE_STAR_SLASH,
    NODE_QUOTE_DOUBLE,
    NODE_QUOTE_SIMPLE,
    NODE_DATA
};

template <>
struct TNodeIndex<NODE_INITIAL>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsWhitespace        , NODE_WHITESPACE   >,
        TGoto<IsUnderscoreOrAlpha , NODE_IDENTIFIER   >,
        TGoto<IsLeftPar           , NODE_PAR_LEFT     >,
        TGoto<IsRightPar          , NODE_PAR_RIGHT    >,
        TGoto<IsSemiColon         , NODE_SEMI_COLON   >,
        TGoto<IsDot               , NODE_DOT          >,
        TGoto<IsComma             , NODE_COMMA        >,
        TGoto<IsEqual             , NODE_EQUAL        >,
        TGoto<IsColon             , NODE_COLON        >,
        TGoto<IsLeftBra           , NODE_BRA_LEFT     >,
        TGoto<IsRightBra          , NODE_BRA_RIGHT    >,
        TGoto<IsPlus              , NODE_PLUS         >,
        TGoto<IsMinus             , NODE_MINUS        >,
        TGoto<IsLeftKet           , NODE_KET_LEFT     >,
        TGoto<IsRightKet          , NODE_KET_RIGHT    >,
        TGoto<IsStar              , NODE_STAR         >,
        TGoto<IsSlash             , NODE_SLASH        >,
        TGoto<IsPercent           , NODE_PERCENT      >,
        TGoto<IsBang              , NODE_BANG         >,
        TGoto<IsQuestion          , NODE_QUESTION     >,
        TGoto<IsTilde             , NODE_TILDE        >,
        TGoto<IsCaret             , NODE_CARET        >,
        TGoto<IsPipe              , NODE_PIPE         >,
        TGoto<IsAmpersand         , NODE_AMPERSAND    >,
        TGoto<IsLeftSqr           , NODE_SQR_LEFT     >,
        TGoto<IsRightSqr          , NODE_SQR_RIGHT    >,
        TGoto<IsDigit             , NODE_NUMBER       >,
        TGoto<IsQuoteDouble       , NODE_QUOTE_DOUBLE >,
        TGoto<IsQuoteSimple       , NODE_QUOTE_SIMPLE >
    >;
};


template <> struct TNodeIndex<NODE_KET_RIGHT_BIS_EQUAL > { using Type = TNode<Token::KET_RIGHT_BIS_EQUAL >;};
template <> struct TNodeIndex<NODE_KET_LEFT_BIS_EQUAL  > { using Type = TNode<Token::KET_LEFT_BIS_EQUAL  >;};
template <> struct TNodeIndex<NODE_AMPERSAND_EQUAL     > { using Type = TNode<Token::AMPERSAND_EQUAL     >;};
template <> struct TNodeIndex<NODE_MINUS_KET_RIGHT     > { using Type = TNode<Token::MINUS_KET_RIGHT     >;};
template <> struct TNodeIndex<NODE_KET_RIGHT_EQUAL     > { using Type = TNode<Token::KET_RIGHT_EQUAL     >;};
template <> struct TNodeIndex<NODE_KET_LEFT_EQUAL      > { using Type = TNode<Token::KET_LEFT_EQUAL      >;};
template <> struct TNodeIndex<NODE_AMPERSAND_BIS       > { using Type = TNode<Token::AMPERSAND_BIS       >;};
template <> struct TNodeIndex<NODE_PERCENT_EQUAL       > { using Type = TNode<Token::PERCENT_EQUAL       >;};
template <> struct TNodeIndex<NODE_MINUS_EQUAL         > { using Type = TNode<Token::MINUS_EQUAL         >;};
template <> struct TNodeIndex<NODE_BANG_EQUAL          > { using Type = TNode<Token::BANG_EQUAL          >;};
template <> struct TNodeIndex<NODE_SLASH_EQUAL         > { using Type = TNode<Token::SLASH_EQUAL         >;};
template <> struct TNodeIndex<NODE_CARET_EQUAL         > { using Type = TNode<Token::CARET_EQUAL         >;};
template <> struct TNodeIndex<NODE_PIPE_EQUAL          > { using Type = TNode<Token::PIPE_EQUAL          >;};
template <> struct TNodeIndex<NODE_STAR_EQUAL          > { using Type = TNode<Token::STAR_EQUAL          >;};
template <> struct TNodeIndex<NODE_PLUS_EQUAL          > { using Type = TNode<Token::PLUS_EQUAL          >;};
template <> struct TNodeIndex<NODE_SEMI_COLON          > { using Type = TNode<Token::SEMI_COLON          >;};
template <> struct TNodeIndex<NODE_SLASH_STAR          > { using Type = TNode<Token::SLASH_STAR          >;};
template <> struct TNodeIndex<NODE_STAR_SLASH          > { using Type = TNode<Token::STAR_SLASH          >;};
template <> struct TNodeIndex<NODE_BRA_RIGHT           > { using Type = TNode<Token::BRA_RIGHT           >;};
template <> struct TNodeIndex<NODE_COLON_BIS           > { using Type = TNode<Token::COLON_BIS           >;};
template <> struct TNodeIndex<NODE_EQUAL_BIS           > { using Type = TNode<Token::EQUAL_BIS           >;};
template <> struct TNodeIndex<NODE_PAR_RIGHT           > { using Type = TNode<Token::PAR_RIGHT           >;};
template <> struct TNodeIndex<NODE_SQR_RIGHT           > { using Type = TNode<Token::SQR_RIGHT           >;};
template <> struct TNodeIndex<NODE_BRA_LEFT            > { using Type = TNode<Token::BRA_LEFT            >;};
template <> struct TNodeIndex<NODE_PAR_LEFT            > { using Type = TNode<Token::PAR_LEFT            >;};
template <> struct TNodeIndex<NODE_SQR_LEFT            > { using Type = TNode<Token::SQR_LEFT            >;};
template <> struct TNodeIndex<NODE_PIPE_BIS            > { using Type = TNode<Token::PIPE_BIS            >;};
template <> struct TNodeIndex<NODE_QUESTION            > { using Type = TNode<Token::QUESTION            >;};
template <> struct TNodeIndex<NODE_COMMENT             > { using Type = TNode<Token::COMMENT             >;};
template <> struct TNodeIndex<NODE_COMMA               > { using Type = TNode<Token::COMMA               >;};
template <> struct TNodeIndex<NODE_TILDE               > { using Type = TNode<Token::TILDE               >;};
template <> struct TNodeIndex<NODE_DATA                > { using Type = TNode<Token::DATA                >;};
template <> struct TNodeIndex<NODE_QUOTE_SIMPLE        > { using Type = TNode<Token::QUOTE_SIMPLE        >;};

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

template<>
struct TNodeIndex<NODE_QUOTE_DOUBLE>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsQuoteDouble , NODE_DATA         >,
        TGoto<IsAnything    , NODE_QUOTE_DOUBLE >
    >;
};

template <>
struct TNodeIndex<NODE_PLUS>
{
    using Type = TNode
    <
        Token::PLUS,
        TGoto<IsEqual, NODE_PLUS_EQUAL>
    >;
};

template <>
struct TNodeIndex<NODE_STAR>
{
    using Type = TNode
    <
        Token::STAR,
        TGoto<IsEqual, NODE_STAR_EQUAL>,
        TGoto<IsSlash, NODE_STAR_SLASH>
    >;
};

template <>
struct TNodeIndex<NODE_SLASH>
{
    using Type = TNode
    <
        Token::SLASH,
        TGoto<IsSlash , NODE_COMMENT_LINE >,
        TGoto<IsEqual , NODE_SLASH_EQUAL  >,
        TGoto<IsStar  , NODE_SLASH_STAR   >
    >;
};

template <>
struct TNodeIndex<NODE_COMMENT_LINE>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsNewLine  , NODE_COMMENT      >,
        TGoto<IsAnything , NODE_COMMENT_LINE >
    >;
};

template <>
struct TNodeIndex<NODE_BLOCK_COMMENT>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsSlash    , NODE_COMMENT_SLASH >,
        TGoto<IsStar     , NODE_COMMENT_STAR  >,
        TGoto<IsAnything , NODE_BLOCK_COMMENT >
    >;
};

template <>
struct TNodeIndex<NODE_BLOCK_DATA>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsQuoteSimple, NODE_QUOTE_SIMPLE >,
        TGoto<IsAnything   , NODE_BLOCK_DATA   >
    >;
};

template <>
struct TNodeIndex<NODE_COMMENT_SLASH>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsStar     , NODE_SLASH_STAR   >,
        TGoto<IsAnything , NODE_BLOCK_COMMENT >
    >;
};

template <>
struct TNodeIndex<NODE_COMMENT_STAR>
{
    using Type = TNode
    <
        Token::UNKNOWN,
        TGoto<IsSlash    , NODE_STAR_SLASH    >,
        TGoto<IsAnything , NODE_BLOCK_COMMENT >
    >;
};

template <>
struct TNodeIndex<NODE_PERCENT>
{
    using Type = TNode
    <
        Token::PERCENT,
        TGoto<IsEqual, NODE_PERCENT_EQUAL>
    >;
};

template <>
struct TNodeIndex<NODE_CARET>
{
    using Type = TNode
    <
        Token::CARET,
        TGoto<IsEqual, NODE_CARET_EQUAL>
    >;
};

template <>
struct TNodeIndex<NODE_MINUS>
{
    using Type = TNode
    <
        Token::MINUS,
        TGoto<IsRightKet , NODE_MINUS_KET_RIGHT >,
        TGoto<IsEqual    , NODE_MINUS_EQUAL     >
    >;
};

template <>
struct TNodeIndex<NODE_KET_LEFT>
{
    using Type = TNode
    <
        Token::KET_LEFT,
        TGoto<IsEqual   , NODE_KET_LEFT_EQUAL >,
        TGoto<IsLeftKet , NODE_KET_LEFT_BIS   >
    >;
};

template <>
struct TNodeIndex<NODE_KET_LEFT_BIS>
{
    using Type = TNode
    <
        Token::KET_LEFT_BIS,
        TGoto<IsEqual, NODE_KET_LEFT_BIS_EQUAL>
    >;
};

template <>
struct TNodeIndex<NODE_KET_RIGHT>
{
    using Type = TNode
    <
        Token::KET_RIGHT,
        TGoto<IsEqual    , NODE_KET_RIGHT_EQUAL >,
        TGoto<IsRightKet , NODE_KET_RIGHT_BIS   >
    >;
};

template <>
struct TNodeIndex<NODE_KET_RIGHT_BIS>
{
    using Type = TNode
    <
        Token::KET_RIGHT_BIS,
        TGoto<IsEqual, NODE_KET_RIGHT_BIS_EQUAL>
    >;
};

template <>
struct TNodeIndex<NODE_BANG>
{
    using Type = TNode
    <
        Token::BANG,
        TGoto<IsEqual, NODE_BANG_EQUAL>
    >;
};

template <>
struct TNodeIndex<NODE_EQUAL>
{
    using Type = TNode
    <
        Token::EQUAL,
        TGoto<IsEqual, NODE_EQUAL_BIS>
    >;
};

template <>
struct TNodeIndex<NODE_PIPE>
{
    using Type = TNode
    <
        Token::PIPE,
        TGoto<IsPipe  , NODE_PIPE_BIS   >,
        TGoto<IsEqual , NODE_PIPE_EQUAL >
    >;
};

template <>
struct TNodeIndex<NODE_COLON>
{
    using Type = TNode
    <
        Token::COLON,
        TGoto<IsColon, NODE_COLON_BIS>
    >;
};

template <>
struct TNodeIndex<NODE_AMPERSAND>
{
    using Type = TNode
    <
        Token::AMPERSAND,
        TGoto<IsAmpersand , NODE_AMPERSAND_BIS   >,
        TGoto<IsEqual     , NODE_AMPERSAND_EQUAL >
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
        Token::LIT_INTEGER,
        TGoto<IsDigit , NODE_NUMBER    >,
        TGoto<IsDot   , NODE_DECIMAL_0 >
    >;
};

template <>
struct TNodeIndex<NODE_DECIMAL_0>
{
    using Type = TNode
    <
        Token::LIT_DECIMAL,
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
        Token::LIT_DECIMAL,
        TGoto<IsDigit, NODE_DECIMAL_3>
    >;
};

} // namespace dmit::lex
