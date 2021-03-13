#pragma once

#include "dmit/sem/analyze.hpp"
#include "dmit/sem/context.hpp"

#include "dmit/ast/node.hpp"

#include "dmit/com/enum.hpp"

#include <variant>

namespace dmit::sem
{

struct Visitor
{
    Visitor(Context& context) : _context{context} {}

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(ast::node::TIndex<KIND> astNodeIndex)
    {
        auto task = _context._scheduler.makeTask(_context._taskPool);

        auto& work = _context._workPool.make(TAnalyzer<KIND>{_context, *this, astNodeIndex});

        task().assignWork(work);
    }

    template <com::TEnumIntegerType<ast::node::Kind> KIND>
    void operator()(ast::node::TRange<KIND> astNodeRange)
    {
        for (uint32_t i = 0; i < astNodeRange._size; i++)
        {
            (*this)(astNodeRange[i]);
        }
    }

    template <class... Types>
    void operator()(const std::variant<Types...>& variant)
    {
        std::visit(*this, variant);
    }

    Context& _context;
};


} // namespace dmit::sem
