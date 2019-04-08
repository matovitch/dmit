#include "dmit/lex/result.hpp"
#include "dmit/lex/token.hpp"

#include "dmit/fmt/lex/token.hpp"

#include "doctest/doctest_fwd.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

using Token       = dmit::lex::Token;
using TokenVector = std::vector<Token>;

TokenVector lex(const std::string& toLex)
{
    dmit::lex::Result result{reinterpret_cast<const uint8_t*>(toLex.data()),
                                                              toLex.size()};

    return result.tokens();
}

TEST_CASE("std::vector<dmit::lex::Token> lex(const std::string&)")
{
    CHECK(lex(   "1" ) == TokenVector{{Token::INTEGER}});
    CHECK(lex( "042" ) == TokenVector{{Token::INTEGER}});

    CHECK(lex(  "1."       ) == TokenVector{{Token::DECIMAL}});
    CHECK(lex(   ".1"      ) == TokenVector{{Token::DECIMAL}});
    CHECK(lex(  "0.0"      ) == TokenVector{{Token::DECIMAL}});
    CHECK(lex( "23.1415"   ) == TokenVector{{Token::DECIMAL}});
    CHECK(lex(  "1.14e0"   ) == TokenVector{{Token::DECIMAL}});
    CHECK(lex(   ".11e+71" ) == TokenVector{{Token::DECIMAL}});

    CHECK(lex(" "          ) == TokenVector{{Token::WHITESPACE}});
    CHECK(lex("\t"         ) == TokenVector{{Token::WHITESPACE}});
    CHECK(lex("\r"         ) == TokenVector{{Token::WHITESPACE}});
    CHECK(lex("\n"         ) == TokenVector{{Token::WHITESPACE}});
    CHECK(lex("  \n \r \t" ) == TokenVector{{Token::WHITESPACE}});

    CHECK(lex("_"       ) == TokenVector{{Token::IDENTIFIER}});
    CHECK(lex("a"       ) == TokenVector{{Token::IDENTIFIER}});
    CHECK(lex("Z"       ) == TokenVector{{Token::IDENTIFIER}});
    CHECK(lex("foo"     ) == TokenVector{{Token::IDENTIFIER}});
    CHECK(lex("_bAr"    ) == TokenVector{{Token::IDENTIFIER}});
    CHECK(lex("_bAr"    ) == TokenVector{{Token::IDENTIFIER}});
    CHECK(lex("_42"     ) == TokenVector{{Token::IDENTIFIER}});
    CHECK(lex("__bAz5_8") == TokenVector{{Token::IDENTIFIER}});

    CHECK(lex("_Test_42 12 10.79e-17") == TokenVector{{Token::IDENTIFIER,
                                                       Token::WHITESPACE,
                                                       Token::INTEGER,
                                                       Token::WHITESPACE,
                                                       Token::DECIMAL}});

    CHECK(lex("+") == TokenVector{{Token::PLUS  }});
    CHECK(lex("-") == TokenVector{{Token::MINUS }});
    CHECK(lex("*") == TokenVector{{Token::STAR  }});
    CHECK(lex("/") == TokenVector{{Token::SLASH }});
}
