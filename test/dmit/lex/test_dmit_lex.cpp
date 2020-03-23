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

TEST_CASE("dmit::lex")
{
    Lexer lexer;

    CHECK(dmit::fmt::asString(lexer("test/data/lex_0.in")) == fileAsString("test/data/lex_0.out"));
    CHECK(dmit::fmt::asString(lexer("test/data/lex_1.in")) == fileAsString("test/data/lex_1.out"));
    CHECK(dmit::fmt::asString(lexer("test/data/lex_2.in")) == fileAsString("test/data/lex_2.out"));
    CHECK(dmit::fmt::asString(lexer("test/data/lex_3.in")) == fileAsString("test/data/lex_3.out"));
    CHECK(dmit::fmt::asString(lexer("test/data/lex_4.in")) == fileAsString("test/data/lex_4.out"));
    CHECK(dmit::fmt::asString(lexer("test/data/lex_5.in")) == fileAsString("test/data/lex_5.out"));
}
