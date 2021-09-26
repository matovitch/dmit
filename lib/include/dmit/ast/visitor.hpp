#pragma once

#include "dmit/ast/node.hpp"

#include "dmit/com/enum.hpp"

namespace dmit::ast
{

template <class Derived>
struct TVisitor
{
    TVisitor(ast::State::NodePool& nodePool) :
        _nodePool{nodePool}
    {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void operator()(ast::node::TIndex<KIND>& nodeIndex)
    {
        static_cast<Derived*>(this)->operator()(nodeIndex);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void operator()(ast::node::TRange<KIND>& nodeRange)
    {
        static_cast<Derived*>(this)->template loopPreamble<KIND>(nodeRange);

        for (uint32_t i = 0; i < nodeRange._size; i++)
        {
            static_cast<Derived*>(this)->template loopIterationPreamble<KIND>(nodeRange[i]);
            static_cast<Derived*>(this)->operator()(nodeRange[i]);
            static_cast<Derived*>(this)->template loopIterationConclusion<KIND>(nodeRange[i]);
        }

        static_cast<Derived*>(this)->template loopConclusion<KIND>(nodeRange);
    }

    template <class... Types>
    void operator()(std::variant<Types...>& variant)
    {
        std::visit(*this, variant);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(std::optional<node::TIndex<KIND>>& nodeIndexOpt)
    {
        if (!nodeIndexOpt)
        {
            static_cast<Derived*>(this)->template emptyOption<KIND>();
            return;
        }

        static_cast<Derived*>(this)->operator()(nodeIndexOpt.value());
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    TNode<KIND>& get(node::TIndex<KIND>& nodeIndex)
    {
        return _nodePool.get(nodeIndex);
    }

    ast::State::NodePool& _nodePool;
};

} // namespace dmit::ast
