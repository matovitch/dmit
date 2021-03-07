#include "test.hpp"

#include "dmit/ast/source_register.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/pool.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/fmt/ast/state.hpp"

#include "dmit/lex/state.hpp"

#include <cstring>

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

        auto& ast = _aster(prs._tree);

        // Build the source

        auto& source = ast._nodePool.get(ast._source);

        _sourceRegister.add(source);

        source._srcPath = std::vector<uint8_t>{reinterpret_cast<const uint8_t*>(filePath),
                                               reinterpret_cast<const uint8_t*>(filePath) + sizeof(filePath)};

        source._srcContent.resize(toParse.size());

        std::memcpy(source._srcContent.data(), toParse.data(), toParse.size());

        source._srcOffsets = dmit::src::line_index::makeOffsets(source._srcContent);

        source._lexOffsets = lex._offsets;

        // return the ast
        return ast;
    }

    dmit::ast::state::Builder _aster;
    dmit::prs::state::Builder _parser;
    dmit::lex::state::Builder _lexer;

    dmit::ast::SourceRegister _sourceRegister;
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
