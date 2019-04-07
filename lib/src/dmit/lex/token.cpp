#include "dmit/lex/token.hpp"

#include <cstdint>

namespace dmit
{

namespace lex
{

Token::Token(const uint8_t theValue) :
    value{theValue}
{}

bool operator==(const Token lhs,
                const Token rhs)
{
    return lhs.value == rhs.value;
}

bool operator!=(const Token lhs,
                const Token rhs)
{
    return lhs.value != rhs.value;
}

} // namespace lex

} // namespace dmit
