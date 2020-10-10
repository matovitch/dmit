#pragma once

#include "topo/node.hpp"
#include "topo/edge.hpp"

#include "list/list.hpp"

namespace topo
{

template <class>
class TGraph;

namespace graph
{

template <class Traits>
class TPoolSet
{
    template <class>
    friend class topo::TGraph;

    typename Traits::Nodes   _nodes;
    typename Traits::Edges   _edges;
    typename Traits::EdgeIts _edgeIts;
};

namespace pool_set
{

template <class Type, std::size_t SIZE>
struct TTraits
{
    using Node = node::TMake<Type, SIZE>;
    using Edge = edge::TMake<Type, SIZE>;

    using Nodes   = typename list::TMake<Node   , SIZE>::CellPool;
    using Edges   = typename list::TMake<Edge   , SIZE>::CellPool;
    using EdgeIt  = typename list::TMake<Edge   , SIZE>::iterator;
    using EdgeIts = typename list::TMake<EdgeIt , SIZE>::CellPool;
};

template <class Type, std::size_t SIZE>
using TMake = TPoolSet<TTraits<Type, SIZE>>;

} // namespace pool_set

} // namespace graph

} // namespace topo
