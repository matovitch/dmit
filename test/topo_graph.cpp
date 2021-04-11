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

    const char* ptr = "CAB";

    for (const auto& nodeIt : cycle.value())
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


/*TEST_CASE("topo::graph_5")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');

    charGraph.attach(aAsNode, bAsNode);
    charGraph.attach(bAsNode, cAsNode);
    charGraph.attach(cAsNode, aAsNode);

    CHECK(charGraph.isCyclic());

    const auto& cycle = charGraph.makeCycle();

    CHECK(cycle);

    const char* ptr = "CBA";

    for (const auto& nodeIt : cycle.value())
    {
        CHECK(nodeIt->_value == *ptr++);
    }

    charGraph.solveCycle();

    CHECK(!charGraph.isCyclic());

    ptr = "BAC";

    while (!charGraph.empty())
    {
        CHECK(charGraph.top()->_value == *ptr++);
        charGraph.pop(charGraph.top());
    }

    CHECK(!charGraph.isCyclic());
}*/

/*struct CharNode
{
    CharNode(char value) : _value{value} {}

    CharNode(char value, bool isHyperNode) :
        _value{value},
        _isHyperNode{isHyperNode}
    {}

    char _value;
    bool _isHyperNode = false;
};

std::ostream& operator<<(std::ostream& os, const CharNode& charNode)
{
    return os << charNode._value;
}

using CharNodeGraph   = topo::graph::TMake<CharNode, 4>;
using CharNodePoolSet = typename CharNodeGraph::PoolSet;*/

TEST_CASE("topo::graph_6")
{
    // 1. Build the graph

    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');
    auto&& dAsNode = charGraph.makeNode('D');
    auto&& eAsNode = charGraph.makeNode('E');
    auto&& fAsNode = charGraph.makeNode('F');
    auto&& gAsNode = charGraph.makeNode('G');

    charGraph.attach(gAsNode, bAsNode);
    charGraph.attach(aAsNode, bAsNode);
    charGraph.attach(bAsNode, cAsNode);
    charGraph.attach(cAsNode, aAsNode);
    charGraph.attach(bAsNode, dAsNode);
    charGraph.attach(dAsNode, eAsNode);
    charGraph.attach(eAsNode, bAsNode);
    charGraph.attach(fAsNode, bAsNode);

    // 2. Compute the strongly connected components

    std::vector<char> nodes;
    std::vector<int> partitions{0};
    auto hyperNode = std::make_optional(aAsNode); hyperNode = std::nullopt;

    do
    {
        if (charGraph.isCyclic())
        {
            hyperNode = hyperNode ? hyperNode : charGraph.makeNode('Z');
            charGraph.solveCycle(hyperNode.value());
        }

        const auto top = charGraph.top();
                         charGraph.pop(top);

        if (!hyperNode || hyperNode.value() != top)
        {
            nodes.push_back(top->_value);
        }

        if (!hyperNode || hyperNode.value() == top)
        {
            partitions.push_back(nodes.size());
            hyperNode = std::nullopt;
        }
    }
    while (!charGraph.empty() || charGraph.isCyclic());

    // 3. Check the result

    const char* ptrChar = "EDACBFG";

    for (auto node : nodes)
    {
        CHECK(node == *ptrChar++);
    }

    const int arrInt[] = {0, 5, 6, 7};
    const int* ptrInt = arrInt;

    for (auto partition : partitions)
    {
        CHECK(partition == *ptrInt++);
    }
}
