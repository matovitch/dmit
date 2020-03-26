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

TEST_SUITE("inout")
{

TEST_CASE("prs")
{
    Parser parser;

    CHECK(dmit::fmt::asString(parser("test/data/prs_0.in")) == fileAsString("test/data/prs_0.out"));
    CHECK(dmit::fmt::asString(parser("test/data/prs_1.in")) == fileAsString("test/data/prs_1.out"));
}

} // TEST_SUITE("inout")

using NodeKind = dmit::prs::state::tree::node::Kind;

TEST_CASE("dmit::prs::Reader")
{
    Parser parser;

    const auto& tree = parser("test/data/prs_0.in")._tree;

    dmit::prs::Reader reader{tree};

    CHECK(reader.look()._kind == NodeKind::DECLAR_FUN);

    auto readerOpt_1 = reader.makeSubReader();

    reader.advance();
    CHECK(!reader.isValid());

    CHECK(readerOpt_1);
    auto readerOpt_2 = readerOpt_1.value().makeSubReader();

    CHECK(readerOpt_1.value().look()._kind == NodeKind::SCOPE);
    readerOpt_1.value().advance();
    CHECK(readerOpt_1.value().look()._kind == NodeKind::IDENTIFIER);
    readerOpt_1.value().advance();
    CHECK(readerOpt_1.value().look()._kind == NodeKind::ARG_LIST);
    readerOpt_1.value().advance();
    CHECK(readerOpt_1.value().look()._kind == NodeKind::IDENTIFIER);
    readerOpt_1.value().advance();
    CHECK(!readerOpt_1.value().isValid());

    CHECK(readerOpt_2);
    CHECK(readerOpt_2.value().look()._kind == NodeKind::STATEM_RETURN);
    readerOpt_2.value().advance();
    CHECK(!readerOpt_2.value().isValid());
}
