#pragma once

#include "topo/graph/edge.hpp"

#include "topo/details/list/iterator.hpp"
#include "topo/details/list/list.hpp"

#include <cstdint>

namespace topo
{

template <class>
class TGraph;

namespace graph
{

template <class Traits>
class TNode
{
    template <class>
    friend class topo::TGraph;

    template <class>
    friend class TEdge;

public:

    using Type           = typename Traits::Type;
    using EdgeListItList = typename Traits::EdgeListItList;
    using EdgeListItPool = typename Traits::EdgeListItPool;

    template <class... Args>
    TNode(EdgeListItPool& edgeListItPool, Args&&... args) :
        _value{args...},
        _dependees{edgeListItPool},
        _dependers{edgeListItPool}
    {}

    bool isPending() const
    {
        return _dependees.empty();
    }

    Type _value;

private:

    EdgeListItList _dependees;
    EdgeListItList _dependers;
};

namespace node
{

template <class TypeTraits, std::size_t SIZE>
struct TTraits
{
    using Type = TypeTraits;

    using EdgeListIt     = topo_details::list::iterator::TMake<edge::TMake<Type, SIZE>>;
    using EdgeListItList = topo_details::list::TMake<EdgeListIt, SIZE>;

    using EdgeListItPool = typename EdgeListItList::CellPool;
};

} // namespace node

} // namespace graph

} // namespace topo
