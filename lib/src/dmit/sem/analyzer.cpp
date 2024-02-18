#include "dmit/sem/analyzer.hpp"

#include "dmit/ast/node.hpp"
#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"
#include "dmit/sem/check.hpp"
#include "dmit/sem/bind.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"
#include "dmit/com/blit.hpp"

#include <atomic>

namespace dmit::sem
{

std::atomic<int> Analyzer::_interfaceAtomCount;

void analyze(com::parallel_for::ThreadPool& threadPool, com::TParallelFor<Analyzer>& parallelAnalyzer)
{
    Analyzer::init();
    threadPool.notify_and_wait(parallelAnalyzer);
}

void analyze(ast::Bundle& bundle, Context& context, InterfaceMap& interfaceMap)
{
    context._scheduler.makeTask(
        [&]()
        {
            bind(bundle, context, interfaceMap);
        },
        context._coroutinePoolLarge,
        DMIT_SEM_CONTEXT_STR("bind")
    );

    context._scheduler.makeTask(
        [&]()
        {
            check(bundle, context, interfaceMap);
        },
        context._coroutinePoolLarge,
        DMIT_SEM_CONTEXT_STR("check")
    );

    context.run();
}

} // namespace dmit::sem
