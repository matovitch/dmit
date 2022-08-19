#pragma once

#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/bundle.hpp"

#include "dmit/com/parallel_for.hpp"

#include <cstdint>
#include <atomic>
#include <vector>

namespace dmit::sem
{

int8_t analyze(ast::Bundle&, Context&, InterfaceMap&);

struct Analyzer
{
    using ReturnType = int8_t;

    Analyzer(sem::InterfaceMap        & interfaceMap,
             std::vector<ast::Bundle> & bundles) :
        _interfaceMap{interfaceMap},
        _bundles{bundles}
    {}

    static void init()
    {
        _interfaceAtomCount.store(0, std::memory_order_relaxed);
    }

    int8_t run(const uint64_t index)
    {
        if (index)
        {
            while (_interfaceAtomCount.load(std::memory_order_acquire) < index - 1);

            return sem::analyze(_bundles[index - 1], _context, _interfaceMap);
        }

        for (auto& bundle : _bundles)
        {
            _interfaceMap.registerBundle(bundle, _context);

            int interfaceCount = _interfaceAtomCount.load(std::memory_order_relaxed);

            _interfaceAtomCount.store(interfaceCount + 1, std::memory_order_release);
        }

        return 0;
    }

    uint32_t size() const
    {
        return _bundles.size() + 1;
    }

    static std::atomic<int> _interfaceAtomCount;

    Context _context;

    sem::InterfaceMap        & _interfaceMap;
    std::vector<ast::Bundle> & _bundles;
};

void analyze(com::TParallelFor<Analyzer>& parallelAnalyzer);

} // namespace dmit::sem
