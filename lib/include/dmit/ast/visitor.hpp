#pragma once

#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/src/slice.hpp"

#include "dmit/com/tree_visitor.hpp"
#include "dmit/com/tree_node.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/enum.hpp"

#include <utility>

namespace dmit::ast
{

template <class Derived, class StackIn, class StackOut>
using TBaseVisitor = typename com::tree::TTMetaVisitor<node::Kind,
                                                       TNode,
                                                       State::NodePool>::template TVisitor<Derived,
                                                                                           StackIn,
                                                                                           StackOut>;
struct StackDummy{};

template <class Derived, class StackIn  = StackDummy,
                         class StackOut = StackDummy>
struct TVisitor : TBaseVisitor<Derived, StackIn, StackOut>
{
    template <class... Args>
    TVisitor(State::NodePool& nodePool, Args&&... args) :
        TBaseVisitor<Derived, StackIn, StackOut>{nodePool, std::forward<Args>(args)...}
    {}

    template <>
    TVisitor(State::NodePool& nodePool) :
        TBaseVisitor<Derived, StackIn, StackOut>{nodePool}
    {}

    lex::Token getToken(const node::TIndex<node::Kind::LEXEME> lexemeIdx)
    {
        return lexeme::getToken(lexemeIdx, TBaseVisitor<Derived, StackIn, StackOut>::_nodePool);
    }

    src::Slice getSlice(const node::TIndex<node::Kind::LEXEME> lexemeIdx)
    {
        return lexeme::getSlice(lexemeIdx, TBaseVisitor<Derived, StackIn, StackOut>::_nodePool);
    }

    src::Slice getSlice(const node::TIndex<node::Kind::IDENTIFIER> idenfifierIdx)
    {
        return getSlice(TBaseVisitor<Derived, StackIn, StackOut>::get(idenfifierIdx)._lexeme);
    }

    bool isInterface(const node::VIndex vIndex)
    {
        return com::tree::v_index::isInterface<node::Kind>(vIndex);
    }
};

} // namespace dmit::ast

#define DMIT_AST_VISITOR_SIMPLE() DMIT_COM_TREE_VISITOR_SIMPLE(dmit::ast::node, Kind);
