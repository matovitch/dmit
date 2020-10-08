#include "test.hpp"

#include "dmit/ast/state.hpp"
#include "dmit/ast/pool.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/fmt/ast/state.hpp"

#include "dmit/lex/state.hpp"

struct Aster
{

    const dmit::ast::State& operator()(const char* const filePath)
    {
        const auto& toParse = fileAsString(filePath);

        _parser .clearState();
        _lexer  .clearState();

        const auto& lex = _lexer(reinterpret_cast<const uint8_t*>(toParse.data()),
                                                                  toParse.size());
        const auto& prs = _parser(lex._tokens);

        return _aster(prs._tree);
    }

    dmit::ast::state::Builder _aster;
    dmit::prs::state::Builder _parser;
    dmit::lex::state::Builder _lexer;
};

TEST_SUITE("inout")
{

TEST_CASE("ast")
{
    Aster aster;

    CHECK(dmit::fmt::asString(aster("test/data/ast/add.in"       )) == fileAsString("test/data/ast/add.out"       ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/call.in"      )) == fileAsString("test/data/ast/call.out"      ));
    CHECK(dmit::fmt::asString(aster("test/data/ast/increment.in" )) == fileAsString("test/data/ast/increment.out" ));
}

} // TEST_SUITE("inout")
