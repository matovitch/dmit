#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/com/blit.hpp"

#include <variant>

namespace dmit::ast
{

template <class Type>
struct TBlitter
{
    TBlitter(State::NodePool& nodePool, Type& value) :
        _nodePool{nodePool},
        _value{value}
    {}

    template <com::TEnumIntegerType<node::Kind> NODE_KIND>
    node::Index operator()(node::TIndex<NODE_KIND>)
    {
        node::TIndex<NODE_KIND> nodeIndex;

        _nodePool.make(nodeIndex);

        com::blit(nodeIndex, _value);

        return nodeIndex;
    }

    template <class... Types>
    node::Index operator()(std::variant<Types...>& variant)
    {
        return std::visit(*this, variant);
    }

    State::NodePool & _nodePool;
    Type            & _value;
};

namespace blitter
{

template<class Type>
TBlitter<Type> make(State::NodePool& nodePool, Type& value)
{
    return TBlitter<Type>{nodePool, value};
}

} // namespace blitter

} // namespace dmit::ast
