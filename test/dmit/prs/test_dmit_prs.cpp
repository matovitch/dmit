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
    CHECK(makeNodeKinds("a + 3") == makeNodeKinds({NodeKind::VARIABLE,
                                                   NodeKind::NUMBER,
                                                   NodeKind::SUM}));

    CHECK(makeNodeKinds("a = 3") == makeNodeKinds({NodeKind::VARIABLE,
                                                   NodeKind::NUMBER,
                                                   NodeKind::ASSIGNMENT}));

    CHECK(makeNodeKinds("x = y") == makeNodeKinds({NodeKind::VARIABLE,
                                                   NodeKind::VARIABLE,
                                                   NodeKind::ASSIGNMENT}));

    CHECK(makeNodeKinds("2 * 3.14") == makeNodeKinds({NodeKind::NUMBER,
                                                      NodeKind::NUMBER,
                                                      NodeKind::PRODUCT}));

    CHECK(makeNodeKinds("(((2)))") == makeNodeKinds({NodeKind::NUMBER}));

    CHECK(makeNodeKinds("-5.2") == makeNodeKinds({NodeKind::NUMBER,
                                                  NodeKind::OPPOSE}));

    CHECK(makeNodeKinds("(a + b) * (a - b)") == makeNodeKinds({NodeKind::VARIABLE,
                                                               NodeKind::VARIABLE,
                                                               NodeKind::SUM,
                                                               NodeKind::VARIABLE,
                                                               NodeKind::VARIABLE,
                                                               NodeKind::OPPOSE,
                                                               NodeKind::SUM,
                                                               NodeKind::PRODUCT}));
}

TEST_CASE("bool validParse(const std::string& toParse)")
{
    CHECK(validParse("2.4e-8"));

    CHECK(validParse("a = -22"));

    CHECK(validParse("a = -z + 2.5/(36 + 35 * 8)"));

    CHECK(!validParse("((1)"));
}