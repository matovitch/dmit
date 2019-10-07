#include "dmit/lex/state.hpp"
#include "dmit/lex/token.hpp"

#include "dmit/fmt/lex/token.hpp"

#include "doctest/doctest_fwd.h"

#include <algorithm>
#include <iostream>
#include <cstdint>
#include <utility>
#include <string>
#include <vector>

using Token = dmit::lex::Token;

class Lexer
{

public:

    const std::vector<Token>& operator()(const std::string& toLex)
    {
        _lexer  .clearState();

        return _lexer(reinterpret_cast<const uint8_t*>(toLex.data()),
                                                       toLex.size())._tokens;
    } 

private:

    dmit::lex::state::Builder _lexer;
};

std::vector<Token> makeTokens(std::initializer_list<Token> tokens)
{
    std::vector<Token> tokenVector;

    tokenVector.push_back(Token::START_OF_INPUT);

    std::copy(tokens.begin(),
              tokens.end(), std::back_inserter(tokenVector));

    tokenVector.push_back(Token::END_OF_INPUT);

    return tokenVector;
}

TEST_CASE("std::vector<Token> lex(const std::string& toLex)")
{
    Lexer lexer;

    CHECK(lexer(   "1" ) == makeTokens({Token::INTEGER}));
    CHECK(lexer( "042" ) == makeTokens({Token::INTEGER}));

    CHECK(lexer(  "1."       ) == makeTokens({Token::DECIMAL}));
    CHECK(lexer(   ".1"      ) == makeTokens({Token::DECIMAL}));
    CHECK(lexer(  "0.0"      ) == makeTokens({Token::DECIMAL}));
    CHECK(lexer( "23.1415"   ) == makeTokens({Token::DECIMAL}));
    CHECK(lexer(  "1.14e0"   ) == makeTokens({Token::DECIMAL}));
    CHECK(lexer(   ".11e+71" ) == makeTokens({Token::DECIMAL}));

    CHECK(lexer(" "          ) == makeTokens({Token::WHITESPACE}));
    CHECK(lexer("\t"         ) == makeTokens({Token::WHITESPACE}));
    CHECK(lexer("\r"         ) == makeTokens({Token::WHITESPACE}));
    CHECK(lexer("\n"         ) == makeTokens({Token::WHITESPACE}));
    CHECK(lexer("  \n \r \t" ) == makeTokens({Token::WHITESPACE}));

    CHECK(lexer("_"       ) == makeTokens({Token::IDENTIFIER}));
    CHECK(lexer("a"       ) == makeTokens({Token::IDENTIFIER}));
    CHECK(lexer("Z"       ) == makeTokens({Token::IDENTIFIER}));
    CHECK(lexer("foo"     ) == makeTokens({Token::IDENTIFIER}));
    CHECK(lexer("_bAr"    ) == makeTokens({Token::IDENTIFIER}));
    CHECK(lexer("_bAr"    ) == makeTokens({Token::IDENTIFIER}));
    CHECK(lexer("_42"     ) == makeTokens({Token::IDENTIFIER}));
    CHECK(lexer("__bAz5_8") == makeTokens({Token::IDENTIFIER}));

    CHECK(lexer("fn"     ) == makeTokens({Token::FN}));
    CHECK(lexer("if"     ) == makeTokens({Token::IF}));
    CHECK(lexer("else"   ) == makeTokens({Token::ELSE}));
    CHECK(lexer("while"  ) == makeTokens({Token::WHILE}));
    CHECK(lexer("return" ) == makeTokens({Token::RETURN}));

    CHECK(lexer("+") == makeTokens({Token::PLUS  }));
    CHECK(lexer("-") == makeTokens({Token::MINUS }));
    CHECK(lexer("*") == makeTokens({Token::STAR  }));
    CHECK(lexer("/") == makeTokens({Token::SLASH }));

    CHECK(lexer("2 * y") == makeTokens({Token::INTEGER    ,
                                        Token::WHITESPACE ,
                                        Token::STAR       ,
                                        Token::WHITESPACE ,
                                        Token::IDENTIFIER }));

    CHECK(lexer("_Test_42-12 10.79e-17") == makeTokens({Token::IDENTIFIER ,
                                                        Token::MINUS      ,
                                                        Token::INTEGER    ,
                                                        Token::WHITESPACE ,
                                                        Token::DECIMAL    }));

    CHECK(lexer("A.B") == makeTokens({Token::IDENTIFIER ,
                                      Token::DOT      ,
                                      Token::Token::IDENTIFIER}));
}
