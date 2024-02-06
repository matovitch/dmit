#pragma once

#include "dmit/sem/context.hpp"
#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/bundle.hpp"

#include "dmit/com/parallel_for.hpp"
#include "dmit/com/storage.hpp"

#include <cstdint>
#include <atomic>
#include <vector>

namespace dmit::sem
{

void analyze(ast::Bundle&, Context&, InterfaceMap&);

struct Analyzer : com::parallel_for::TJob<Context, int8_t>
{
    Analyzer(sem::InterfaceMap          & interfaceMap,
             com::TStorage<ast::Bundle> & bundles) :
        _interfaceMap{interfaceMap},
        _bundles{bundles}
    {}

    static void init()
    {
        _interfaceAtomCount.store(0, std::memory_order_relaxed);
    }

    void run(Context& context, int32_t index, int8_t*) override
    {
        if (index)
        {
            while (_interfaceAtomCount.load(std::memory_order_acquire) < index - 1);

            return sem::analyze(_bundles[index - 1], context, _interfaceMap);
        }

        for (auto& bundle : _bundles)
        {
            _interfaceMap.registerBundle(bundle, context);

            int interfaceCount = _interfaceAtomCount.load(std::memory_order_relaxed);

            _interfaceAtomCount.store(interfaceCount + 1, std::memory_order_release);
        }
    }

    int32_t size() const override
    {
        return _bundles._size + 1;
    }

    static std::atomic<int> _interfaceAtomCount;

    sem::InterfaceMap          & _interfaceMap;
    com::TStorage<ast::Bundle> & _bundles;
};

void analyze(com::parallel_for::ThreadPool& threadPool, com::TParallelFor<Analyzer>& parallelAnalyzer);

} // namespace dmit::sem
