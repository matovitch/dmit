#pragma once

#include "dmit/com/tree_node.hpp"

#include "dmit/com/assert.hpp"
#include "dmit/com/enum.hpp"

#include <utility>

namespace dmit::com::tree
{

template <class Kind, template <TEnumIntegerType<Kind>> class TNode, class Pool>
struct TTMetaVisitor
{
    struct StackDummy{};

    template <class Derived, class StackIn  = StackDummy,
                             class StackOut = StackDummy>
    struct TVisitor
    {
        template <class... Args>
        TVisitor(Pool& nodePool, Args&&... args) :
            _nodePool{nodePool},
            _stackIn{std::forward<Args>(args)...},
            _stackPtrIn  {&_stackIn  },
            _stackPtrOut {&_stackOut }
        {}

        template <>
        TVisitor(Pool& nodePool) :
            _nodePool{nodePool},
            _stackIn{},
            _stackPtrIn  {&_stackIn  },
            _stackPtrOut {&_stackOut }
        {}

        template <TEnumIntegerType<Kind> KIND>
        void operator()(typename TMetaNode<Kind>::template TIndex<KIND> nodeIndex)
        {
            StackOut stackOut; // Create the output stack
            StackIn  stackIn ; // create the input  stack

            stackIn = *_stackPtrIn; // Copy the parent input stack

            StackOut * stackPtrOutCopy = _stackPtrOut ; // Save parent output stack pointer
            StackIn  * stackPtrInCopy  = _stackPtrIn  ; // Save parent input  stack pointer

            _stackPtrOut = &stackOut ; // Update output stack pointer
            _stackPtrIn  = &stackIn  ; // Update input  stack pointer

            static_cast<Derived*>(this)->operator()(nodeIndex);

            _stackPtrOut = stackPtrOutCopy ; // Restore parent output stack pointer
            _stackPtrIn  = stackPtrInCopy  ; // Restore parent input  stack pointer

            *_stackPtrOut = stackOut; // Copy the child ouput stack
        }

        template <TEnumIntegerType<Kind> KIND>
        void operator()(typename TMetaNode<Kind>::template TRange<KIND>& nodeRange)
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

        template <class Type>
        void operator()(std::optional<Type>& opt)
        {
            if (!opt)
            {
                static_cast<Derived*>(this)->template emptyOption<Type>();
                return;
            }

            (*this)(opt.value());
        }

        template <TEnumIntegerType<Kind> KIND>
        TNode<KIND>& get(const typename TMetaNode<Kind>::template TIndex<KIND> nodeIndex)
        {
            return _nodePool.get(nodeIndex);
        }

        TVisitor<Derived, StackIn, StackOut>& base()
        {
            return (*this);
        }

        Pool& _nodePool;

        StackIn    _stackIn;
        StackOut   _stackOut;
        StackIn  * _stackPtrIn;
        StackOut * _stackPtrOut;
    };
};

} // namespace dmit::com::tree

#define DMIT_COM_TREE_VISITOR_SIMPLE(prefix, Kind)              \
    template <dmit::com::TEnumIntegerType<prefix::Kind> KIND>   \
    void loopConclusion(prefix::TRange<KIND>&) {}               \
                                                                \
    template <dmit::com::TEnumIntegerType<prefix::Kind> KIND>   \
    void loopPreamble(prefix::TRange<KIND>&) {}                 \
                                                                \
    template <dmit::com::TEnumIntegerType<prefix::Kind> KIND>   \
    void loopIterationConclusion(prefix::TIndex<KIND>) {}       \
                                                                \
    template <dmit::com::TEnumIntegerType<prefix::Kind> KIND>   \
    void loopIterationPreamble(prefix::TIndex<KIND>) {}         \
                                                                \
    template <class Type>                                       \
    void emptyOption() {}                                       \
                                                                \
    template <dmit::com::TEnumIntegerType<prefix::Kind> KIND>   \
    void operator()(prefix::TIndex<KIND>)                       \
    {                                                           \
        DMIT_COM_ASSERT(!"Not implemented");                    \
    }                                                           \
