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

    using NodeListIt = typename Traits::NodeListIt;
    using EdgeListIt = typename Traits::EdgeListIt;
    using PoolSet    = typename Traits::PoolSet;

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
        if (!isCyclic())
        {
            return std::nullopt;
        }

        std::vector<NodeListIt> cycle;

        NodeListIt nodeIt = _blockeds.begin();

        do
        {
            cycle.push_back(nodeIt);

            nodeIt = (*(nodeIt->_dependers.begin()))->_dependerNode;
        }
        while (nodeIt != cycle.front());

        return cycle;
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

private:

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

    using EdgeItPool = typename list::TMake<EdgeListIt, SIZE>::CellPool;

    using PoolSet = pool_set::TMake<Type, SIZE>;
};

template <class Type, std::size_t SIZE>
using TMake = TGraph<TTraits<Type, SIZE>>;

} // namespace graph

} // namespace topo
