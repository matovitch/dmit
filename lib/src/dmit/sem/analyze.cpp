#include "dmit/sem/analyze.hpp"

#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/src/slice.hpp"

#include "dmit/com/murmur.hpp"
#include "dmit/com/enum.hpp"

#include <iostream> // TO_REMOVE

namespace dmit::sem
{

namespace
{

template <com::TEnumIntegerType<ast::node::Kind> KIND>
src::Slice getSlice(const ast::node::TIndex<KIND>& node,
                    Context& context)
{
    return context._srcPartition.getSlice(
               context._astNodePool.get(
                   context._astNodePool.get(
                       node
                   )._lexeme
               )._index
           );
};

struct AnalyzerFunction
{
    AnalyzerFunction(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx,
                     Context& context) :
        _functionIdx{functionIdx},
        _context{context}
    {}

    void operator()()
    {
        auto& function = _context._astNodePool.get(_functionIdx);

        const auto& functionName = getSlice(function._name, _context);

        dmit::com::murmur::hash(functionName._head, functionName.size(), function._id);
    }

    const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> _functionIdx;
    Context& _context;
};

} // namespace

void analyze(ast::State& ast,
             Context& context)
{
    auto& functions = ast._program._functions;

    for (uint32_t i = 0; i < functions._size; i++)
    {
        auto task = context._scheduler.makeTask(context._taskVoidPool);

        auto& work = context._workVoidPool.make(AnalyzerFunction{functions[i], context}, context._mesgVoid);

        task().assignWork(work);
    }

    context._scheduler.run();
}

} // namespace dmit::sem
