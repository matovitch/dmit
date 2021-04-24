#pragma once

#include "topo/edge.hpp"

#include "list/iterator.hpp"
#include "list/list.hpp"

#include <cstddef>

namespace topo
{

template <class>
class TGraph;

namespace graph
{

namespace node
{

struct HyperOpen  {};
struct HyperClose {};

} // namespace node

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

    TNode(EdgeListItPool& edgeListItPool, node::HyperOpen) :
        _dummy{true},
        _dependees{edgeListItPool},
        _dependers{edgeListItPool},
        _isHyperOpen {true}
    {}

    TNode(EdgeListItPool& edgeListItPool, node::HyperClose) :
        _dummy{true},
        _dependees{edgeListItPool},
        _dependers{edgeListItPool},
        _isHyperClose {true}
    {}

    bool isPending() const
    {
        return _dependees.empty();
    }

    bool isHyperOpen  () const { return _isHyperOpen  ; }
    bool isHyperClose () const { return _isHyperClose ; }

    union
    {
        Type _value;
        bool _dummy : 1;
    };

private:

    EdgeListItList _dependees;
    EdgeListItList _dependers;

    bool _withinCycle  : 1 = false;
    bool _isHyperOpen  : 1 = false;
    bool _isHyperClose : 1 = false;
};

namespace node
{

template <class TypeTraits, std::size_t SIZE>
struct TTraits
{
    using Type = TypeTraits;

    using EdgeListIt     = list::iterator::TMake<edge::TMake<Type, SIZE>>;
    using EdgeListItList = list::TMake<EdgeListIt, SIZE>;

    using EdgeListItPool = typename EdgeListItList::CellPool;
};

} // namespace node
} // namespace graph
} // namespace topo
