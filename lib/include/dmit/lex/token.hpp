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
        UNKNOWN,
        INTEGER,
        DECIMAL,
        WHITESPACE,
        IDENTIFIER,
        PLUS,
        MINUS,
        STAR,
        SLASH,
        PAREN_L,
        PAREN_R,
        DOT,
        EQUAL,
        END_OF_INPUT
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Token);
};

} // namespace lex

} // namespace dmit
