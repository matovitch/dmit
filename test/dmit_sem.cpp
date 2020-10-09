#include "test.hpp"

#include "dmit/sem/analyze.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/prs/state.hpp"

#include "dmit/lex/state.hpp"

#include "dmit/src/partition.hpp"

TEST_CASE("sem")
{
    dmit::ast::state::Builder aster;
    dmit::prs::state::Builder parser;
    dmit::lex::state::Builder lexer;

    const auto& toParse = fileAsString("test/data/ast/increment.in");

    const auto toParseAsBytes = reinterpret_cast<const uint8_t*>(toParse.data());

    const auto& lex = lexer(toParseAsBytes,
                            toParse.size());

    const auto& prs = parser(lex._tokens);

    auto& ast = aster(prs._tree);

    dmit::src::Partition partition{toParseAsBytes, lex._offsets};

    dmit::sem::analyze(partition, ast);
}
