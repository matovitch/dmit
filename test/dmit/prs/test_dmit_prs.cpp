#include "dmit/prs/state.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/lex/state.hpp"
#include "dmit/lex/token.hpp"

#include "dmit/fmt/prs/tree.hpp"

#include "doctest/doctest_fwd.h"
#include "doctest/utils.h"

#include <iostream>
#include <utility>
#include <vector>

enum class ParserType
{
    EXPRESSION,
    ASSIGNMENT,
    DECLARATION,
    STATEMENT,
    FUNCTION,
    PROGRAM
};

template <ParserType PARSER_TYPE>
class TParser
{

public:

    const dmit::prs::State& operator()(const std::string& toParse)
    {
        _parser .clearState();
        _lexer  .clearState();

        const auto& tokens = _lexer(reinterpret_cast<const uint8_t*>(toParse.data()),
                                                                     toParse.size())._tokens;

        if constexpr (PARSER_TYPE == ParserType::EXPRESSION)
        {
          return _parser(tokens, _parser._parserExpression);
        }
        if constexpr (PARSER_TYPE == ParserType::ASSIGNMENT)
        {
          return _parser(tokens, _parser._parserAssignment);
        }
        if constexpr (PARSER_TYPE == ParserType::DECLARATION)
        {
          return _parser(tokens, _parser._parserDeclaration);
        }
        if constexpr (PARSER_TYPE == ParserType::STATEMENT)
        {
          return _parser(tokens, _parser._parserStatement);
        }
        if constexpr (PARSER_TYPE == ParserType::FUNCTION)
        {
          return _parser(tokens, _parser._parserFunction);
        }

        return _parser(tokens);
    }

private:

    dmit::prs::state::Builder _parser;
    dmit::lex::state::Builder _lexer;
};

using NodeKind = dmit::prs::state::tree::node::Kind;

template <ParserType PARSER_TYPE>
std::vector<NodeKind> tMakeNodeKinds(const std::string& toParse)
{
    TParser<PARSER_TYPE> parser;

    const auto& tree = parser(toParse)._tree;

    std::vector<NodeKind> nodeKinds;

    for (const auto& node : tree.nodes())
    {
        nodeKinds.push_back(node._kind);
    }

    return nodeKinds;
}

std::vector<NodeKind> makeNodeKinds(std::initializer_list<NodeKind> nodeKinds)
{
    return std::vector<NodeKind>{nodeKinds};
}

template <ParserType PARSER_TYPE>
bool tValidParse(const std::string& toParse)
{
    return TParser<PARSER_TYPE>{}(toParse)._errorSet.empty();
}

TEST_CASE("dmit::prs::expression")
{
    auto makeExpression = [](const std::string& toParse)
                          {
                              return tMakeNodeKinds<ParserType::EXPRESSION>(toParse);
                          };

    CHECK(makeExpression("+a") == makeNodeKinds({NodeKind::IDENTIFIER}));

    CHECK(makeExpression("-a") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                NodeKind::OPPOSE}));


    CHECK(makeExpression("a + 3") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                   NodeKind::INTEGER,
                                                   NodeKind::SUM}));

    CHECK(makeExpression("2 * 3.14") == makeNodeKinds({NodeKind::INTEGER,
                                                      NodeKind::DECIMAL,
                                                      NodeKind::PRODUCT}));

    CHECK(makeExpression("(((2)))") == makeNodeKinds({NodeKind::INTEGER}));

    CHECK(makeExpression("-5.2") == makeNodeKinds({NodeKind::DECIMAL,
                                                  NodeKind::OPPOSE}));

    CHECK(makeExpression("(a + b) * (a - b);") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                                NodeKind::IDENTIFIER,
                                                                NodeKind::SUM,
                                                                NodeKind::IDENTIFIER,
                                                                NodeKind::IDENTIFIER,
                                                                NodeKind::SUM,
                                                                NodeKind::PRODUCT}));

    CHECK(makeExpression("i >= 3") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                    NodeKind::INTEGER,
                                                    NodeKind::COMPARISON}));

    CHECK(makeExpression("f()") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                 NodeKind::FUN_CALL}));

    CHECK(makeExpression("f(3)") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                  NodeKind::INTEGER,
                                                  NodeKind::FUN_CALL}));

    CHECK(makeExpression("f(a, b)") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                     NodeKind::IDENTIFIER,
                                                     NodeKind::IDENTIFIER,
                                                     NodeKind::FUN_CALL}));

    CHECK(makeExpression("f(g())") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                    NodeKind::IDENTIFIER,
                                                    NodeKind::FUN_CALL,
                                                    NodeKind::FUN_CALL}));
}

TEST_CASE("dmit::prs::assigment")
{
    auto makeAssignment = [](const std::string& toParse)
                            {
                                return tMakeNodeKinds<ParserType::ASSIGNMENT>(toParse);
                            };

    CHECK(makeAssignment("a = 3") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                    NodeKind::INTEGER,
                                                    NodeKind::ASSIGNMENT}));

    CHECK(makeAssignment("x = y") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                   NodeKind::IDENTIFIER,
                                                   NodeKind::ASSIGNMENT}));
}

TEST_CASE("dmit::prs::statement")
{
    auto makeStatement = [](const std::string& toParse)
                         {
                             return tMakeNodeKinds<ParserType::STATEMENT>(toParse);
                         };

    CHECK(makeStatement("return 42") == makeNodeKinds({NodeKind::INTEGER,
                                                           NodeKind::STATEM_RETURN}));
}

TEST_CASE("dmit::prs::declaration")
{
    auto makeDeclaration = [](const std::string& toParse)
                           {
                               return tMakeNodeKinds<ParserType::DECLARATION>(toParse);
                           };

    CHECK(makeDeclaration("let i : i64;") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                            NodeKind::IDENTIFIER,
                                                            NodeKind::DECLAR_LET}));
}

TEST_CASE("dmit::prs::function")
{
  auto makeFunction = [](const std::string& toParse)
                      {
                          return tMakeNodeKinds<ParserType::FUNCTION>(toParse);
                      };

  CHECK(makeFunction("func f(){}") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                     NodeKind::SCOPE,
                                                     NodeKind::DECLAR_FUN}));

  CHECK(makeFunction("func f(){{}}") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                       NodeKind::SCOPE,
                                                       NodeKind::SCOPE,
                                                       NodeKind::DECLAR_FUN}));

  CHECK(makeFunction("func f(){{}{}}") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                         NodeKind::SCOPE,
                                                         NodeKind::SCOPE,
                                                         NodeKind::SCOPE,
                                                         NodeKind::DECLAR_FUN}));

  CHECK(makeFunction("func f(){let i : i64; i = i + 1;}") ==
        makeNodeKinds({NodeKind::IDENTIFIER,
                                   NodeKind::IDENTIFIER,
                                   NodeKind::IDENTIFIER,
                               NodeKind::DECLAR_LET,
                                   NodeKind::IDENTIFIER,
                                       NodeKind::IDENTIFIER,
                                       NodeKind::INTEGER,
                                   NodeKind::SUM,
                               NodeKind::ASSIGNMENT,
                           NodeKind::SCOPE,
                       NodeKind::DECLAR_FUN}));

  CHECK(makeFunction("func add(x : i64, y : i64) { return x + y; }") ==
        makeNodeKinds({NodeKind::IDENTIFIER,
                       NodeKind::IDENTIFIER,
                       NodeKind::IDENTIFIER,
                       NodeKind::IDENTIFIER,
                       NodeKind::IDENTIFIER,
                                      NodeKind::IDENTIFIER,
                                      NodeKind::IDENTIFIER,
                                  NodeKind::SUM,
                              NodeKind::STATEM_RETURN,
                          NodeKind::SCOPE,
                       NodeKind::DECLAR_FUN}));
}

TEST_CASE("dmit::prs::program")
{
    auto makeProgram = [](const std::string& toParse)
                       {
                           return tMakeNodeKinds<ParserType::PROGRAM>(toParse);
                       };

    CHECK(makeProgram("func add(x : i64, y : i64) { return x + y; }") ==
          makeNodeKinds({    NodeKind::IDENTIFIER,
                             NodeKind::IDENTIFIER,
                             NodeKind::IDENTIFIER,
                             NodeKind::IDENTIFIER,
                             NodeKind::IDENTIFIER,
                                            NodeKind::IDENTIFIER,
                                            NodeKind::IDENTIFIER,
                                        NodeKind::SUM,
                                    NodeKind::STATEM_RETURN,
                                NodeKind::SCOPE,
                             NodeKind::DECLAR_FUN,
                         NodeKind::PROGRAM}));
}

TEST_CASE("dmit::prs::validation")
{
    CHECK(tValidParse<ParserType::EXPRESSION>("2.4e-8"));

    CHECK(tValidParse<ParserType::ASSIGNMENT>("a = -22"));

    CHECK(tValidParse<ParserType::ASSIGNMENT>("a = -z + 2.5/(36 + 35 * 8)"));

    CHECK(tValidParse<ParserType::EXPRESSION>("(a < b) > (c < d)"));

    CHECK(!tValidParse<ParserType::EXPRESSION>("((1)"));
}