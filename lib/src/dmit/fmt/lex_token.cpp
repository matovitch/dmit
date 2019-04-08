#include "dmit/fmt/lex/token.hpp"

#include "dmit/lex/token.hpp"

static const char* K_TOKEN_AS_CSTR[] =
{
    "UNKNOWN",
    "INTEGER",
    "DECIMAL",
    "WHITESPACE",
    "IDENTIFIER",
    "PLUS",
    "MINUS",
    "STAR",
    "SLASH"
};

namespace dmit
{

namespace lex
{

std::ostream& operator<<(std::ostream& os, const Token token)
{
    return os << K_TOKEN_AS_CSTR[static_cast<std::size_t>(token.value)];
}

} // namespace lex

} // namespace dmit
