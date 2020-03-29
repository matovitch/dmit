#include "dmit/ast/pool.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/lex/state.hpp"

#include "doctest/doctest_fwd.h"
#include "doctest/utils.h"

struct ParserNew
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

void makeFunction(const dmit::prs::state::Tree& tree,
                  dmit::prs::Reader& reader,
                  dmit::ast::TNode<dmit::ast::node::Kind::FUNCTION>& function,
                  dmit::ast::node::TPool<2>& nodePool)
{
    CHECK(reader.look()._kind == NodeKind::SCOPE);
    nodePool.make(function._body);
    reader.advance();
    CHECK(reader.look()._kind == NodeKind::RETURN_TYPE);
    nodePool.make(function._returnType);
    reader.advance();
    CHECK(reader.look()._kind == NodeKind::ARG_LIST);
    nodePool.make(function._arguments);
    reader.advance();
    CHECK(reader.look()._kind == NodeKind::IDENTIFIER);
    nodePool.make(function._name);

    nodePool.get(function._name)._index = tree.range(reader.look())._start;
}

TEST_CASE("dmit::ast::dummy")
{
    Parser parser;

    const auto& tree = parser("test/data/prs_0.in")._tree;

    dmit::prs::Reader reader{tree};

    dmit::ast::node::TPool<2> nodePool;

    dmit::ast::TNode<dmit::ast::node::Kind::PROGRAM> program;

    CHECK(reader.isValid());
    CHECK(reader.look()._kind == NodeKind::DECLAR_FUN);

    nodePool.make(reader.size(), program._functions);

    uint32_t i = 0;

    while (reader.isValid())
    {
        auto function = nodePool.get(program._functions[i]);
        auto functionReader = reader.makeSubReader();

        CHECK(functionReader);

        makeFunction(tree, functionReader.value(), function, nodePool);

        reader.advance();
        i++;
    }
}
