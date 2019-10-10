#pragma once

#include "dmit/com/enum.hpp"

#include "dmit/fmt/formatable.hpp"

#include <cstdint>

namespace dmit
{

namespace lex
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
        BRA_LEFT,
        KET_LEFT,
        PAR_LEFT,
        BRA_RIGHT,
        KET_RIGHT,
        PAR_RIGHT,
        DOT,
        COMMA,
        COLON,
        SEMI_COLON,
        EQUAL,
        ARROW,
        IF,
        ELSE,
        LET,
        VAR,
        FUNC,
        WHILE,
        RETURN,
        END_OF_INPUT
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Token);
};

} // namespace lex

} // namespace dmit
