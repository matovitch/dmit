#include "dmit/sem/analyzer.hpp"

#include "dmit/sem/interface_map.hpp"
#include "dmit/sem/context.hpp"
#include "dmit/sem/check.hpp"
#include "dmit/sem/bind.hpp"

#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"
#include "dmit/com/blit.hpp"

#include <cstdint>
#include <atomic>

namespace dmit::sem
{

std::atomic<int> Analyzer::_interfaceAtomCount;

void analyze(com::TParallelFor<Analyzer>& parallelAnalyzer)
{
    Analyzer::init();
    parallelAnalyzer.run();
}

int8_t analyze(ast::Bundle& bundle, Context& context, InterfaceMap& interfaceMap)
{
    context.makeTaskFromWork(
        [&]()
        {
            bind  (bundle, context, interfaceMap);
            check (bundle, context);
        },
        context._coroutinePoolLarge
    );

    context.run();

    return 0;
}

} // namespace dmit::sem
