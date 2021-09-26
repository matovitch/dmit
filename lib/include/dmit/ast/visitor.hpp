#pragma once

#include "dmit/ast/node.hpp"
#include "dmit/ast/pool.hpp"

#include "dmit/com/enum.hpp"

#include <utility>

namespace dmit::ast
{

struct StackDummy{};

template <class Derived, class Stack = StackDummy>
struct TVisitor
{
    template <class... Args>
    TVisitor(ast::State::NodePool& nodePool, Args&&... args) :
        _nodePool{nodePool},
        _stack{std::forward<Args>(args)...},
        _stackPtr{&_stack}
    {}

    template <>
    TVisitor(ast::State::NodePool& nodePool) :
        _nodePool{nodePool},
        _stack{},
        _stackPtr{&_stack}
    {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void operator()(ast::node::TIndex<KIND> nodeIndex)
    {
        Stack stack = *_stackPtr;        // Copy the parent stack

        Stack* stackPtrCopy = _stackPtr; // Save parent stack pointer

        _stackPtr = &stack;              // Update stack pointer

        static_cast<Derived*>(this)->operator()(nodeIndex);

        _stackPtr = stackPtrCopy; // Restore parent stack pointer
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void operator()(ast::node::TRange<KIND>& nodeRange)
    {
        static_cast<Derived*>(this)->template loopPreamble<KIND>(nodeRange);

        for (uint32_t i = 0; i < nodeRange._size; i++)
        {
            static_cast<Derived*>(this)->template loopIterationPreamble<KIND>(nodeRange[i]);
            (*this)(nodeRange[i]);
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

        (*this)(nodeIndexOpt.value());
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    TNode<KIND>& get(node::TIndex<KIND>& nodeIndex)
    {
        return _nodePool.get(nodeIndex);
    }

    ast::State::NodePool& _nodePool;

    Stack  _stack;
    Stack* _stackPtr;
};

} // namespace dmit::ast
