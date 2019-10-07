#include "dmit/fmt/lex/token.hpp"

#include "dmit/lex/token.hpp"

#include <sstream>
#include <string>

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
    "SLASH",
    "PAREN_L",
    "PAREN_R",
    "DOT",
    "EQUAL",
    "END_OF_INPUT"
};

namespace dmit
{

namespace fmt
{

std::string asString(const lex::Token token)
{
    std::ostringstream oss;

    oss << "{\"token\":\"" << K_TOKEN_AS_CSTR[token._asInt] << "\"}";

    return oss.str();
}

} // namespace fmt

} // namespace dmit
