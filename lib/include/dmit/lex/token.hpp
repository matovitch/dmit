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
        START_OF_INPUT,
        UNKNOWN,
        INTEGER,
        DECIMAL,
        WHITESPACE,
        IDENTIFIER,
        PLUS,
        MINUS,
        STAR,
        SLASH,
        PERCENT,
        BANG,
        QUESTION,
        TILDE,
        BRA_LEFT,
        KET_LEFT,
        PAR_LEFT,
        SQR_LEFT,
        BRA_RIGHT,
        KET_RIGHT,
        PAR_RIGHT,
        SQR_RIGHT,
        DOT,
        COMMA,
        COLON,
        SEMI_COLON,
        EQUAL,
        CARET,
        PIPE,
        AMPERSAND,
        MINUS_KET_RIGHT,
        KET_RIGHT_EQUAL,
        KET_LEFT_EQUAL,
        KET_LEFT_BIS,
        KET_RIGHT_BIS,
        BANG_EQUAL,
        EQUAL_BIS,
        PIPE_BIS,
        AMPERSAND_BIS,
        COLON_BIS,
        PLUS_EQUAL,
        MINUS_EQUAL,
        STAR_EQUAL,
        SLASH_EQUAL,
        PERCENT_EQUAL,
        KET_LEFT_BIS_EQUAL,
        KET_RIGHT_BIS_EQUAL,
        AMPERSAND_EQUAL,
        CARET_EQUAL,
        PIPE_EQUAL,
        IF,
        ELSE,
        LET,
        FUNC,
        WHILE,
        RETURN,
        END_OF_INPUT
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Token);
};

} // namespace dmit::lex
