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

TokenVector dmitLex(const std::string& toLex)
{
    dmit::lex::Result result{reinterpret_cast<const uint8_t*>(toLex.data()),
                                                              toLex.size()};

    return result.tokens();
}

TEST_CASE("std::vector<dmit::lex::Token> dmitLex(const std::string&)")
{
    CHECK(dmitLex("1 ") == TokenVector{{Token::INT,
                                        Token::UNKNOWN}});

    CHECK(dmitLex("42 ") == TokenVector{{Token::INT,
                                         Token::UNKNOWN}});

    CHECK(dmitLex("1. ") == TokenVector{{Token::FLOAT,
                                         Token::UNKNOWN}});

    CHECK(dmitLex(".1 ") == TokenVector{{Token::FLOAT,
                                         Token::UNKNOWN}});

    CHECK(dmitLex("27.1e-5 ") == TokenVector{{Token::FLOAT,
                                              Token::UNKNOWN}});

    CHECK(dmitLex("1 2.5 ") == TokenVector{{Token::INT,
                                            Token::UNKNOWN,
                                            Token::FLOAT,
                                            Token::UNKNOWN}});
}
