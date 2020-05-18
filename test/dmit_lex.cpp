#include "test.hpp"

#include "dmit/lex/state.hpp"

#include "dmit/fmt/lex/state.hpp"

class Lexer
{

public:

    const dmit::lex::State& operator()(const char* const filePath)
    {
        const auto& toLex = fileAsString(filePath);

        _lexer.clearState();

        return _lexer(reinterpret_cast<const uint8_t*>(toLex.data()),
                                                       toLex.size());
    }

private:

    dmit::lex::state::Builder _lexer;
};

TEST_SUITE("inout")
{

TEST_CASE("lex")
{
    Lexer lexer;

    CHECK(dmit::fmt::asString(lexer("test/data/lex/comment.in"    )) == fileAsString("test/data/lex/comment.out"    ));
    CHECK(dmit::fmt::asString(lexer("test/data/lex/identifier.in" )) == fileAsString("test/data/lex/identifier.out" ));
    CHECK(dmit::fmt::asString(lexer("test/data/lex/keyword.in"    )) == fileAsString("test/data/lex/keyword.out"    ));
    CHECK(dmit::fmt::asString(lexer("test/data/lex/number.in"     )) == fileAsString("test/data/lex/number.out"     ));
    CHECK(dmit::fmt::asString(lexer("test/data/lex/operator.in"   )) == fileAsString("test/data/lex/operator.out"   ));
    CHECK(dmit::fmt::asString(lexer("test/data/lex/whitespace.in" )) == fileAsString("test/data/lex/whitespace.out" ));
}

} // TEST_SUITE("inout")
