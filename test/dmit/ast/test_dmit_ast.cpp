#include "dmit/ast/state.hpp"
#include "dmit/ast/pool.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/state.hpp"

#include "dmit/lex/state.hpp"

#include "doctest/doctest_fwd.h"
#include "doctest/utils.h"

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

template <class Type, class Variant>
const Type& checkAndGet(const Variant& variant)
{
    CHECK(std::holds_alternative<Type>(variant));
    return std::get<Type>(variant);
}

TEST_CASE("dmit::ast::dummy")
{
    Aster aster;

    const auto& ast = aster("test/data/prs_0.in");

    const auto& astNodePool = aster._aster.nodePool();

    CHECK(ast._functions._size == 1);

    const auto& function = astNodePool.get(ast._functions[0]);

    const auto& functionName       = astNodePool.get(function._name       );
    const auto& functionArguments  = astNodePool.get(function._arguments  );
    const auto& functionReturnType = astNodePool.get(function._returnType );
    const auto& functionBody       = astNodePool.get(function._body       );

    CHECK(functionName._index == 33);
    CHECK(functionArguments._annotaTypes._size == 2);
    CHECK(functionReturnType._option);
    CHECK(functionBody._variants._size == 1);

    const auto& functionBodyContent = astNodePool.get(functionBody._variants[0]);

    const auto& statement = checkAndGet<dmit::ast::Statement>(functionBodyContent._value);

    const auto& stmReturn = astNodePool.get(checkAndGet<dmit::ast::node::TIndex<dmit::ast::node::Kind::STATEM_RETURN>>(statement));

    const auto& binop = astNodePool.get(checkAndGet<dmit::ast::node::TIndex<dmit::ast::node::Kind::BINOP>>(stmReturn._expression));

    CHECK(std::holds_alternative<dmit::ast::node::TIndex<dmit::ast::node::Kind::LEXEME>>(binop._lhs));
    CHECK(std::holds_alternative<dmit::ast::node::TIndex<dmit::ast::node::Kind::LEXEME>>(binop._rhs));
}
