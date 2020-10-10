#include "test.hpp"

#include "topo/pool_set.hpp"
#include "topo/graph.hpp"

using CharGraph   = topo::graph::TMake<char, 4>;
using CharPoolSet = typename CharGraph::PoolSet;


TEST_CASE("topo::graph_1")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');

    charGraph.attach(aAsNode, bAsNode);

    auto&& edge = charGraph.attach(bAsNode, cAsNode);

    charGraph.detach(edge);

    const char* ptr = "BAC";

    while (!charGraph.empty())
    {
        CHECK(charGraph.top()->_value == *ptr++);
        charGraph.pop(charGraph.top());
    }

    CHECK(!charGraph.isCyclic());
}

TEST_CASE("topo::graph_2")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');
    auto&& dAsNode = charGraph.makeNode('D');

    charGraph.attach(aAsNode, bAsNode);
    charGraph.attach(bAsNode, cAsNode);
    charGraph.attach(cAsNode, aAsNode);
    charGraph.attach(aAsNode, dAsNode);

    CHECK(charGraph.top()->_value == 'D');
    charGraph.pop(charGraph.top());

    CHECK(charGraph.empty());

    CHECK(charGraph.isCyclic());

    const auto& cycle = charGraph.makeCycle();

    CHECK(cycle);

    const char* ptr = "CBA";

    for (const auto& nodeIt : *(cycle))
    {
        CHECK(nodeIt->_value == *ptr++);
    }
}

TEST_CASE("topo::graph_3")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');
    auto&& dAsNode = charGraph.makeNode('D');

    charGraph.attach(aAsNode, bAsNode);
    charGraph.attach(bAsNode, cAsNode);
    charGraph.attach(bAsNode, dAsNode);

    charGraph.pop(cAsNode);

    const char* ptr = "DBA";

    while (!charGraph.empty())
    {
        CHECK(charGraph.top()->_value == *ptr++);
        charGraph.pop(charGraph.top());
    }

    CHECK(!charGraph.isCyclic());
}

TEST_CASE("topo::graph_4")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');

    charGraph.attach(bAsNode, bAsNode);
    charGraph.attach(aAsNode, bAsNode);

    charGraph.pop(bAsNode);

    const char* ptr = "A";

    while (!charGraph.empty())
    {
        CHECK(charGraph.top()->_value == *ptr++);
        charGraph.pop(charGraph.top());
    }

    CHECK(!charGraph.isCyclic());
}
