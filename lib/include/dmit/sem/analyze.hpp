#pragma once

#include "dmit/ast/state.hpp"

#include "dmit/sem/context.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

struct Visitor;

template <com::TEnumIntegerType<ast::node::Kind> KIND>
struct TAnalyzer
{
    TAnalyzer(Context& context,
              Visitor& visitor,
              ast::node::TIndex<KIND> astNodeIndex){}

    void operator()()
    {
        DMIT_COM_ASSERT(!"Analyzer unimplemented");
    }
};

template <>
struct TAnalyzer<ast::node::Kind::FUN_DEFINITION>
{
    TAnalyzer(Context& context,
              Visitor& visitor,
              ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> astNodeIndex);

    void defineName(ast::TNode<ast::node::Kind::FUN_DEFINITION>& function);

    void operator()();

    Context& _context;
    Visitor& _visitor;
    ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> _astNodeIndex;
};

template <>
struct TAnalyzer<ast::node::Kind::TYPE_CLAIM>
{
    TAnalyzer(Context& context,
              Visitor& visitor,
              ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> astNodeIndex);

    void operator()();

    Context& _context;
    Visitor& _visitor;
    ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> _astNodeIndex;
};

void analyze(ast::State& ast);

} // namespace dmit::sem
