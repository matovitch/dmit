#include "dmit/ast/state.hpp"
#include "dmit/ast/pool.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/fmt/ast/state.hpp"

#include "dmit/lex/state.hpp"

#include <variant>

struct Aster
{

    const dmit::ast::State& operator()(const char* const filePath)
    {
        const auto& toParse = fileAsString(filePath);

        _parser .clearState();
        _lexer  .clearState();

        const auto& tokens = _lexer(reinterpret_cast<const uint8_t*>(toParse.data()),
                                                                     toParse.size())._tokens;
        const auto& parseTree = _parser(tokens)._tree;

        return _aster(parseTree);
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
    CHECK(dmit::fmt::asString(aster("test/data/ast/increment.in" )) == fileAsString("test/data/ast/increment.out" ));
}

} // TEST_SUITE("inout")
