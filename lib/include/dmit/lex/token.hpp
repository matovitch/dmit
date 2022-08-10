#pragma once

#include "dmit/com/enum.hpp"

#include "dmit/fmt/formatable.hpp"

#include <cstdint>

namespace dmit::lex
{

struct Token : com::TEnum<uint8_t>, fmt::Formatable
{
    enum : uint8_t
    {
        WHITESPACE,
        COMMENT,
        IF,
        ELSE,
        LET,
        FUNC,
        CLASS,
        WHILE,
        EXPORT,
        IMPORT,
        MODULE,
        RETURN,
        AMPERSAND,
        AMPERSAND_BIS,
        AMPERSAND_EQUAL,
        BANG,
        BANG_EQUAL,
        BRA_LEFT,
        BRA_RIGHT,
        CARET,
        CARET_EQUAL,
        COLON,
        COLON_BIS,
        COMMA,
        DOT,
        END_OF_INPUT,
        EQUAL,
        EQUAL_BIS,
        IDENTIFIER,
        LIT_DECIMAL,
        LIT_INTEGER,
        KET_LEFT,
        KET_LEFT_BIS,
        KET_LEFT_BIS_EQUAL,
        KET_LEFT_EQUAL,
        KET_RIGHT,
        KET_RIGHT_BIS,
        KET_RIGHT_BIS_EQUAL,
        KET_RIGHT_EQUAL,
        MINUS,
        MINUS_EQUAL,
        MINUS_KET_RIGHT,
        PAR_LEFT,
        PAR_RIGHT,
        PERCENT,
        PERCENT_EQUAL,
        PIPE,
        PIPE_BIS,
        PIPE_EQUAL,
        PLUS,
        PLUS_EQUAL,
        QUESTION,
        SEMI_COLON,
        SLASH,
        SLASH_EQUAL,
        SLASH_STAR,
        SQR_LEFT,
        SQR_RIGHT,
        STAR,
        STAR_EQUAL,
        STAR_SLASH,
        START_OF_INPUT,
        TILDE,
        UNKNOWN
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Token);
};

} // namespace dmit::lex
