#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/fmt/prs/state.hpp"
#include "dmit/fmt/prs/tree.hpp"

#include "dmit/lex/state.hpp"

struct Parser
{
    const dmit::prs::State& operator()(const char* const filePath)
    {
        const auto& toParse = fileAsString(filePath);

        _parser .clearState();
        _lexer  .clearState();

        const auto& tokens = _lexer(reinterpret_cast<const uint8_t*>(toParse.data()),
                                                                     toParse.size())._tokens;
        return _parser(tokens);
    }

    dmit::prs::state::Builder _parser;
    dmit::lex::state::Builder _lexer;
};

#define VALID   "test/data/prs/valid/"
#define INVALID "test/data/prs/invalid/"

TEST_SUITE("inout")
{

TEST_CASE("prs")
{
    Parser parser;

    CHECK(dmit::fmt::asString(parser(VALID "function_empty.in")) == fileAsString(VALID "function_empty.out" ));
    CHECK(dmit::fmt::asString(parser(VALID "function_add.in"  )) == fileAsString(VALID "function_add.out"   ));
    CHECK(dmit::fmt::asString(parser(VALID "function_incr.in" )) == fileAsString(VALID "function_incr.out"  ));
    CHECK(dmit::fmt::asString(parser(VALID "expression.in"    )) == fileAsString(VALID "expression.out"     ));
    CHECK(dmit::fmt::asString(parser(VALID "scope.in"         )) == fileAsString(VALID "scope.out"          ));

    CHECK(dmit::fmt::asString(parser(INVALID "missing_semicolon.in" )) == fileAsString(INVALID "missing_semicolon.out" ));
    CHECK(dmit::fmt::asString(parser(INVALID "expression.in"        )) == fileAsString(INVALID "expression.out"        ));
}

} // TEST_SUITE("inout")
