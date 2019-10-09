#include "dmit/fmt/lex/token.hpp"

#include "dmit/lex/token.hpp"

#include <sstream>
#include <string>

static const char* K_TOKEN_AS_CSTR[] =
{
    "START_OF_INPUT",
    "UNKNOWN",
    "INTEGER",
    "DECIMAL",
    "WHITESPACE",
    "IDENTIFIER",
    "PLUS",
    "MINUS",
    "STAR",
    "SLASH",
    "PAR_LEFT",
    "PAR_RIGHT",
    "DOT",
    "EQUAL",
    "IF",
    "ELSE",
    "LET",
    "VAR",
    "FUNC",
    "WHILE",
    "RETURN",
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
