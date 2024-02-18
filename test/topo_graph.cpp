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

    CHECK(!cycle.empty());

    const char* ptr = "CAB";

    for (const auto& nodeIt : cycle)
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
    std::vector<int> partitions;
    int hyperNodeStackSize = 0;

    do
    {
        if (hyperNodeStackSize == 0)
        {
            partitions.push_back(nodes.size());
        }

        while (charGraph.isCyclic())
        {
            charGraph.solveCycle();
        }

        const auto top = charGraph.top();
                         charGraph.pop(top);

        if (top->isHyperOpen())
        {
            hyperNodeStackSize++;
            continue;
        }

        if (top->isHyperClose())
        {
            hyperNodeStackSize--;
            continue;
        }

        nodes.push_back(top->_value);
    }
    while (!charGraph.empty() || charGraph.isCyclic());

    // 3. Check the result

    const char* ptrChar = "BDECAGF";

    for (auto node : nodes)
    {
        CHECK(node == *ptrChar++);
    }

    const int arrInt[] = {0, 5, 6};
    const int* ptrInt = arrInt;

    for (auto partition : partitions)
    {
        CHECK(partition == *ptrInt++);
    }
}

TEST_CASE("topo::graph_7")
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

    charGraph.attach(aAsNode, bAsNode);
    charGraph.attach(bAsNode, cAsNode);
    charGraph.attach(cAsNode, aAsNode);

    charGraph.attach(eAsNode, bAsNode);

    charGraph.attach(dAsNode, eAsNode);
    charGraph.attach(eAsNode, fAsNode);
    charGraph.attach(fAsNode, dAsNode);

    // 2. Compute the strongly connected components

    std::vector<char> nodes;
    std::vector<int> partitions;
    int hyperNodeStackSize = 0;

    do
    {
        if (hyperNodeStackSize == 0)
        {
            partitions.push_back(nodes.size());
        }

        while (charGraph.isCyclic())
        {
            charGraph.solveCycle();
        }

        const auto top = charGraph.top();
                         charGraph.pop(top);

        if (top->isHyperOpen())
        {
            hyperNodeStackSize++;
            continue;
        }

        if (top->isHyperClose())
        {
            hyperNodeStackSize--;
            continue;
        }

        nodes.push_back(top->_value);
    }
    while (!charGraph.empty() || charGraph.isCyclic());

    // 3. Check the result

    const char* ptrChar = "BCAFDE";

    for (auto node : nodes)
    {
        CHECK(node == *ptrChar++);
    }

    const int arrInt[] = {0, 3};
    const int* ptrInt = arrInt;

    for (auto partition : partitions)
    {
        CHECK(partition == *ptrInt++);
    }
}
