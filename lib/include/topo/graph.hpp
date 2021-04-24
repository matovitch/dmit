#pragma once

#include "topo/pool_set.hpp"
#include "topo/node.hpp"
#include "topo/edge.hpp"

#include "list/steal.hpp"
#include "list/cell.hpp"
#include "list/list.hpp"

#include "pool/pool.hpp"

#include <optional>
#include <cstdint>
#include <vector>

namespace topo
{

template <class Traits>
class TGraph
{
    using EdgeItPool = typename Traits::EdgeItPool;
    using EdgeList   = typename Traits::EdgeList;
    using NodeList   = typename Traits::NodeList;

public:

    using EdgeListItList = typename Traits::EdgeListItList;
    using EdgeListIt     = typename Traits::EdgeListIt;
    using NodeListIt     = typename Traits::NodeListIt;
    using PoolSet        = typename Traits::PoolSet;

    TGraph(PoolSet& poolSet) :
        _edges    {poolSet._edges},
        _pendings {poolSet._nodes},
        _blockeds {poolSet._nodes},
        _edgeIts  {poolSet._edgeIts}
    {}

    template <class... Args>
    NodeListIt makeNode(Args&&... args)
    {
        _pendings.emplace_front(_edgeIts, args...);

        return _pendings.begin();
    }

    // lhs depends upon rhs
    EdgeListIt attach(NodeListIt lhs,
                      NodeListIt rhs)
    {
        if (lhs->isPending())
        {
            list::steal(_blockeds, _pendings, lhs);
        }

        _edges.emplace_front(lhs, rhs);

        auto&& edgeIt = _edges.begin();

        lhs->_dependees.emplace_front(edgeIt);
        rhs->_dependers.emplace_front(edgeIt);

        edgeIt->update();

        return edgeIt;
    }

    void detach(EdgeListIt edgeIt)
    {
        edgeIt->detach();

        if (edgeIt->_dependerNode->isPending())
        {
            list::steal(_pendings, _blockeds, edgeIt->_dependerNode);
        }

        _edges.erase(edgeIt);
    }

    bool isCyclic() const
    {
        return _pendings.empty() && !_blockeds.empty();
    }

    std::optional<std::vector<NodeListIt>> makeCycle()
    {
        // 0. If not cyclic, return the empty cycle

        if (!isCyclic())
        {
            return std::nullopt;
        }

        // 1. Build the cycle

        std::vector<NodeListIt> cycle;

        auto nodeIt = _blockeds.begin();

        while (cycle.empty() || nodeIt != cycle.front())
        {
            if (nodeIt->_withinCycle)
            {
                cycle.push_back(nodeIt);
            }

            nodeIt->_withinCycle = !(nodeIt->_withinCycle);
            nodeIt = (*(nodeIt->_dependees.begin()))->_dependeeNode;
        }

        // 2. Reset the state of the visited nodes

        nodeIt = _blockeds.begin();

        while (nodeIt->_withinCycle)
        {
            nodeIt->_withinCycle = false;
            nodeIt = (*(nodeIt->_dependees.begin()))->_dependeeNode;
        }

        return cycle;
    }

    void solveCycle()
    {
        // 0. Create the hypernodes

        auto hyperNodeOpen  = makeNode(graph::node::HyperOpen  {});
        auto hyperNodeClose = makeNode(graph::node::HyperClose {});

        // 1. Find cycle and attach its nodes to the hyperNodes

        std::optional<NodeListIt> cycleFront;

        auto prevIt = _blockeds.begin();
        auto nodeIt = _blockeds.begin();

        while (!cycleFront || nodeIt != cycleFront.value())
        {
            auto edgeIt = *(nodeIt->_dependees.begin());

            if (nodeIt->_withinCycle)
            {
                attach(hyperNodeClose, nodeIt);
                attach(nodeIt, hyperNodeOpen);
                cycleFront = cycleFront ? cycleFront : nodeIt;
                detach(edgeIt);
            }

            nodeIt->_withinCycle = !(nodeIt->_withinCycle);
            nodeIt = edgeIt->_dependeeNode;
        }

        // 2. Reset the state of the visited nodes

        nodeIt = prevIt;

        while (nodeIt->_withinCycle)
        {
            nodeIt->_withinCycle = false;
            nodeIt = (*(nodeIt->_dependees.begin()))->_dependeeNode;
        }

        // 3. Attach depender/dependee nodes to hyperNodes and detach them from cycle

        auto& cycle = hyperNodeOpen->_dependers;
        auto getNode = [](auto edgeIt) { return edgeIt->_dependerNode; };

        for (auto edge : cycle)
        {
            nodeIt = getNode(edge);

            if (!(nodeIt->isHyperClose()))
            {
                attachDetachDependees(hyperNodeOpen, nodeIt->_dependees);
            }

            if (!(nodeIt->isHyperOpen()))
            {
                attachDetachDependers(hyperNodeClose, nodeIt->_dependers);
            }
        }

        attach(hyperNodeClose, hyperNodeOpen);
    }

    bool empty() const
    {
        return _pendings.empty();
    }

    NodeListIt top()
    {
        return _pendings.begin();
    }

    void detachAll(NodeListIt nodeIt)
    {
        for (auto&& depender : nodeIt->_dependers)
        {
            detach(*(depender->_dependerEdge));
        }
    }

    void pop(NodeListIt nodeIt)
    {
        detachAll(nodeIt);

        _pendings.erase(nodeIt);
    }

    std::optional<NodeListIt> nextPending(NodeListIt nodeListIt)
    {
        if (!(nodeListIt->isPending()) || ++nodeListIt == _pendings.end())
        {
            return std::nullopt;
        }

        return nodeListIt;
    }

private:

    void attachDetachDependees(NodeListIt nodeIt, EdgeListItList edgeListItList)
    {
        for (auto edgeListIt : edgeListItList)
        {
            if (edgeListIt->_dependeeNode != nodeIt)
            {
                attach(nodeIt, edgeListIt->_dependeeNode);
                detach(edgeListIt);
            }
        }
    }

    void attachDetachDependers(NodeListIt nodeIt, EdgeListItList edgeListItList)
    {
        for (auto edgeListIt : edgeListItList)
        {
            if (edgeListIt->_dependerNode != nodeIt)
            {
                attach(edgeListIt->_dependerNode, nodeIt);
                detach(edgeListIt);
            }
        }
    }

    EdgeList _edges;

    NodeList _pendings;
    NodeList _blockeds;

    EdgeItPool& _edgeIts;
};

namespace graph
{

template <class Type, std::size_t SIZE>
struct TTraits
{
    using Node = node::TMake<Type, SIZE>;
    using Edge = edge::TMake<Type, SIZE>;

    using EdgeList = list::TMake<Edge, SIZE>;
    using NodeList = list::TMake<Node, SIZE>;

    using NodeListIt = typename NodeList::iterator;
    using EdgeListIt = typename EdgeList::iterator;

    using EdgeListItList = list::TMake<EdgeListIt, SIZE>;

    using EdgeItPool = typename list::TMake<EdgeListIt, SIZE>::CellPool;

    using PoolSet = pool_set::TMake<Type, SIZE>;
};

template <class Type, std::size_t SIZE>
using TMake = TGraph<TTraits<Type, SIZE>>;

} // namespace graph

} // namespace topo
