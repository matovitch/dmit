#pragma once

#include <cstdint>

namespace dmit
{

namespace lex
{

struct Token
{
    enum : uint8_t
    {
        UNKNOWN,
        INT,
        FLOAT
    };

    Token(const uint8_t theValue);

    const uint8_t value;
};

bool operator==(const Token,
                const Token);
bool operator!=(const Token,
                const Token);

} // namespace lex

} // namespace dmit
