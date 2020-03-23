#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/fmt/prs/state.hpp"
#include "dmit/fmt/prs/tree.hpp"

#include "dmit/lex/state.hpp"

class Parser
{

public:

    const dmit::prs::State& operator()(const char* const filePath)
    {
        const auto& toParse = fileAsString(filePath);

        _parser .clearState();
        _lexer  .clearState();

        const auto& tokens = _lexer(reinterpret_cast<const uint8_t*>(toParse.data()),
                                                                     toParse.size())._tokens;
        return _parser(tokens);
    }

private:

    dmit::prs::state::Builder _parser;
    dmit::lex::state::Builder _lexer;
};

TEST_CASE("dmit::prs")
{
    Parser parser;

    CHECK(dmit::fmt::asString(parser("test/data/prs_0.in")) == fileAsString("test/data/prs_0.out"));
    CHECK(dmit::fmt::asString(parser("test/data/prs_1.in")) == fileAsString("test/data/prs_1.out"));
}

using NodeKind = dmit::prs::state::tree::node::Kind;

TEST_CASE("dmit::prs::Reader")
{
    Parser parser;

    const auto& tree = parser("test/data/prs_0.in")._tree;

    dmit::prs::Reader reader{tree};

    auto program = reader.makeHead();

    CHECK(reader.look(program)._kind == NodeKind::PROGRAM);

    auto functionOpt = reader.makeHead(program, 0); CHECK(functionOpt);

    CHECK(reader.look(functionOpt.value())._kind == NodeKind::DECLAR_FUN);

    auto headOpt = reader.makeHead(functionOpt.value(), 0); CHECK(headOpt);
    CHECK(reader.look(headOpt.value())._kind == NodeKind::SCOPE);
    headOpt = reader.makeHead(functionOpt.value(), 1); CHECK(headOpt);
    CHECK(reader.look(headOpt.value())._kind == NodeKind::IDENTIFIER);
    headOpt = reader.makeHead(functionOpt.value(), 2); CHECK(headOpt);
    CHECK(reader.look(headOpt.value())._kind == NodeKind::IDENTIFIER);
    headOpt = reader.makeHead(functionOpt.value(), 3); CHECK(headOpt);
    CHECK(reader.look(headOpt.value())._kind == NodeKind::IDENTIFIER);
    headOpt = reader.makeHead(functionOpt.value(), 4); CHECK(headOpt);
    CHECK(reader.look(headOpt.value())._kind == NodeKind::IDENTIFIER);
    headOpt = reader.makeHead(functionOpt.value(), 5); CHECK(headOpt);
    CHECK(reader.look(headOpt.value())._kind == NodeKind::IDENTIFIER);
    headOpt = reader.makeHead(functionOpt.value(), 6); CHECK(headOpt);
    CHECK(reader.look(headOpt.value())._kind == NodeKind::IDENTIFIER);
    headOpt = reader.makeHead(functionOpt.value(), 7); CHECK(!headOpt);
}