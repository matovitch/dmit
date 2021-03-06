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
    const auto& lexeme = context._astNodePool.get(context._astNodePool.get(node)._lexeme);

    const auto& source = context._astNodePool.get(lexeme._source);

    return src::Slice{source._srcContent, source._lexOffsets, lexeme._index};
};

struct FunctionAnalyzer
{
    FunctionAnalyzer(const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION>& functionIdx,
                     Context& context) :
        _functionIdx{functionIdx},
        _context{context}
    {}

    void defineName(ast::TNode<ast::node::Kind::FUN_DEFINITION>& function)
    {
        // 1. Compute definition key
        const auto& functionName = getSlice(function._name, _context);

        dmit::com::murmur::hash(functionName._head, functionName.size(), function._id);

        dmit::com::murmur::combine(_context.DEFINE, function._id);

        // 2. Check possible redefinition
        auto fitFact = _context._factMap.find(function._id);

        const auto functionLocation = _functionIdx.location();

        if (fitFact != _context._factMap.end())
        {
            // 2.1 notifyRedefinitionError(fitFact->second, functionLocation, _context);
            DMIT_COM_ASSERT(!"Function redefinition");
        }

        // 3. Notify pending task if it exists
        auto fitTask = _context._taskMap.find(function._id);

        if (fitTask != _context._taskMap.end())
        {
            _context._scheduler.unlock(fitTask->second);
        }

        // 4. Register definition key
        _context._factMap.emplace(function._id, functionLocation);
    }

    void defineArg(const ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> argumentIdx,
                   const ast::TNode<ast::node::Kind::FUN_DEFINITION>& function)
    {
        auto& argument = _context._astNodePool.get(argumentIdx);

        // 1. Compute definition key
        const auto& argumentVariable = getSlice(argument._variable, _context);

        dmit::com::murmur::hash(argumentVariable._head, argumentVariable.size(), argument._id);

        dmit::com::murmur::combine(_context.ARGUMENT_OF, argument._id);
        dmit::com::murmur::combine(function._id, argument._id);

        // 2. Check possible redefinition
        auto fitFact = _context._factMap.find(argument._id);

        const auto argumentLocation = argumentIdx.location();

        if (fitFact != _context._factMap.end())
        {
            // 2.1 notifyRedefinitionError(fitFact->second, argumentLocation, _context);
            DMIT_COM_ASSERT(!"Argument redefinition");
        }

        // 3. Register definition key
        _context._factMap.emplace(argument._id, argumentLocation);
    }

    void analyzeVariant(const ast::node::TIndex<ast::node::Kind::SCOPE_VARIANT> variantIdx,
                        const ast::TNode<ast::node::Kind::FUN_DEFINITION>& function)
    {
        //auto& variant = _context._astNodePool.get(variantIdx)._value;
        //TODO visit
    }

    void analyzeBody(const ast::TNode<ast::node::Kind::FUN_DEFINITION>& function)
    {
        auto& variants = _context._astNodePool.get(function._body)._variants;

        for (uint32_t i = 0; i < variants._size; i++)
        {
            analyzeVariant(variants[i], function);
        }
    }

    void operator()()
    {
        auto& function = _context._astNodePool.get(_functionIdx);

        // Name
        defineName(function);

        // Arguments
        auto& arguments = function._arguments;

        for (uint32_t i = 0; i < arguments._size; i++)
        {
            defineArg(arguments[i], function);
        }

        // Body
        analyzeBody(function);
    }

    const ast::node::TIndex<ast::node::Kind::FUN_DEFINITION> _functionIdx;
    Context& _context;
};

} // namespace

void analyze(ast::State& ast,
             Context& context)
{
    auto& functions = ast._nodePool.get(ast._program)._functions;

    for (uint32_t i = 0; i < functions._size; i++)
    {
        auto task = context._scheduler.makeTask(context._taskPool);

        auto& work = context._workPool.make(FunctionAnalyzer{functions[i], context});

        task().assignWork(work);
    }

    context._scheduler.run();
}

} // namespace dmit::sem
