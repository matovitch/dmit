#pragma once

#include "dmit/sem/task.hpp"

#include "topo/graph.hpp"

#include <cstdint>

namespace dmit::sem
{

template <std::size_t SIZE>
class TScheduler
{

public:

    using TaskGraph  = topo::graph::TMake<task::Abstract*, SIZE>;
    using PoolSet    = typename TaskGraph::PoolSet;
    using Dependency = typename TaskGraph::EdgeListIt;

    template <class Type>
    using TTaskWrapper = task::TWrapper<Type, SIZE>;

    TScheduler() : _taskGraph{_poolSet} {}

    template <class Type>
    TTaskWrapper<Type> makeTask(task::TPool<Type>& taskPool)
    {
        auto& task = taskPool.make();
        return TTaskWrapper<Type>{_taskGraph.makeNode(&task)};
    }

    template <class Type>
    Dependency attach(TTaskWrapper<Type> lhs,
                      TTaskWrapper<Type> rhs)
    {
        rhs().message().send();
        return _taskGraph.attach(lhs._value,
                                 rhs._value);
    }

    void run()
    {
        while (!_taskGraph.empty())
        {
            auto top = _taskGraph.top();
            top->_value->run();
            _taskGraph.pop(top);
        }
    }

private:

    PoolSet _poolSet;
    TaskGraph _taskGraph;
};

using Scheduler = dmit::sem::TScheduler<1>;

} // namespace dmit::sem
