#include "dmit/fmt/lex/token.hpp"

#include "dmit/lex/token.hpp"

#include <sstream>
#include <string>

static const char* const K_TOKEN_AS_CSTR[] =
{
    "WHITESPACE",
    "COMMENT",
    "IF",
    "ELSE",
    "LET",
    "FUNC",
    "WHILE",
    "EXPORT",
    "IMPORT",
    "MODULE",
    "RETURN",
    "AMPERSAND",
    "AMPERSAND_BIS",
    "AMPERSAND_EQUAL",
    "BANG",
    "BANG_EQUAL",
    "BRA_LEFT",
    "BRA_RIGHT",
    "CARET",
    "CARET_EQUAL",
    "COLON",
    "COLON_BIS",
    "COMMA",
    "DECIMAL",
    "DOT",
    "END_OF_INPUT",
    "EQUAL",
    "EQUAL_BIS",
    "IDENTIFIER",
    "INTEGER",
    "KET_LEFT",
    "KET_LEFT_BIS",
    "KET_LEFT_BIS_EQUAL",
    "KET_LEFT_EQUAL",
    "KET_RIGHT",
    "KET_RIGHT_BIS",
    "KET_RIGHT_BIS_EQUAL",
    "KET_RIGHT_EQUAL",
    "MINUS",
    "MINUS_EQUAL",
    "MINUS_KET_RIGHT",
    "PAR_LEFT",
    "PAR_RIGHT",
    "PERCENT",
    "PERCENT_EQUAL",
    "PIPE",
    "PIPE_BIS",
    "PIPE_EQUAL",
    "PLUS",
    "PLUS_EQUAL",
    "QUESTION",
    "SEMI_COLON",
    "SLASH",
    "SLASH_EQUAL",
    "SQR_LEFT",
    "SQR_RIGHT",
    "STAR",
    "STAR_EQUAL",
    "START_OF_INPUT",
    "TILDE",
    "UNKNOWN"
};

namespace dmit::fmt
{

std::string asString(const lex::Token token)
{
    std::ostringstream oss;

    oss << "\"" << K_TOKEN_AS_CSTR[token._asInt] << "\"";

    return oss.str();
}

} // namespace dmit::fmt
