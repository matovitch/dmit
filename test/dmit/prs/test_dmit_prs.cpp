#include "dmit/prs/state.hpp"
#include "dmit/prs/error.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/lex/state.hpp"
#include "dmit/lex/token.hpp"

#include "dmit/fmt/prs/error.hpp"
#include "dmit/fmt/prs/tree.hpp"

#include "doctest/doctest_fwd.h"

#include <iostream>
#include <vector>

class Parser
{

public:

    const dmit::prs::State& operator()(const std::string& toParse)
    {
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

using NodeKind = dmit::prs::state::tree::node::Kind;

std::vector<NodeKind> makeNodeKinds(const std::string& toParse)
{
    Parser parser;

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

bool validParse(const std::string& toParse)
{
    return Parser{}(toParse)._errorSet.empty();
}

TEST_CASE("std::vector<NodeKind> makeNodeKinds(const std::string& toParse)")
{
    CHECK(makeNodeKinds("a + 3") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                   NodeKind::INTEGER,
                                                   NodeKind::SUM}));

    CHECK(makeNodeKinds("a = 3") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                   NodeKind::INTEGER,
                                                   NodeKind::ASSIGNMENT}));

    CHECK(makeNodeKinds("x = y") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                   NodeKind::IDENTIFIER,
                                                   NodeKind::ASSIGNMENT}));

    CHECK(makeNodeKinds("2 * 3.14") == makeNodeKinds({NodeKind::INTEGER,
                                                      NodeKind::DECIMAL,
                                                      NodeKind::PRODUCT}));

    CHECK(makeNodeKinds("(((2)))") == makeNodeKinds({NodeKind::INTEGER}));

    CHECK(makeNodeKinds("-5.2") == makeNodeKinds({NodeKind::DECIMAL,
                                                  NodeKind::OPPOSE}));

    CHECK(makeNodeKinds("(a + b) * (a - b)") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                               NodeKind::IDENTIFIER,
                                                               NodeKind::SUM,
                                                               NodeKind::IDENTIFIER,
                                                               NodeKind::IDENTIFIER,
                                                               NodeKind::OPPOSE,
                                                               NodeKind::SUM,
                                                               NodeKind::PRODUCT}));

    CHECK(makeNodeKinds("let pi = 3.14") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                           NodeKind::DECIMAL,
                                                           NodeKind::DECLAR_LET}));

    CHECK(makeNodeKinds("let pi : f64 = 3.14") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                                 NodeKind::IDENTIFIER,
                                                                 NodeKind::DECIMAL,
                                                                 NodeKind::DECLAR_LET}));

    CHECK(makeNodeKinds("var x") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                   NodeKind::DECLAR_VAR}));

    CHECK(makeNodeKinds("f(a, b + c)") == makeNodeKinds({NodeKind::IDENTIFIER,
                                                         NodeKind::IDENTIFIER,
                                                         NodeKind::IDENTIFIER,
                                                         NodeKind::IDENTIFIER,
                                                         NodeKind::SUM,
                                                         NodeKind::LIST_ARG,
                                                         NodeKind::LIST_DISP}));


}

TEST_CASE("bool validParse(const std::string& toParse)")
{
    CHECK(validParse("2.4e-8"));

    CHECK(validParse("a = -22"));

    CHECK(validParse("a = -z + 2.5/(36 + 35 * 8)"));

    CHECK(!validParse("((1)"));
}