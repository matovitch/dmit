#pragma once

#include "dmit/sem/task.hpp"

#include "dmit/com/assert.hpp"

#include "topo/graph.hpp"

#include <cstdint>
#include <vector>

namespace dmit::sem
{

template <std::size_t SIZE>
class TScheduler
{

public:

    using TaskWrapper = task::TWrapper <SIZE>;
    using TaskPool    = task::TPool    <SIZE>;
    using Task        = TTask          <SIZE>;

    using TaskGraph  = topo::graph::TMake<Task*, SIZE>;

    using PoolSet    = typename TaskGraph::PoolSet;
    using Dependency = typename TaskGraph::EdgeListIt;

    TScheduler() : _taskGraph{_poolSet} {}

    TaskWrapper makeTask(TaskPool& taskPool)
    {
        auto& task = taskPool.make();
        return TaskWrapper{_taskGraph.makeNode(&task)};
    }

    Dependency attach(TaskWrapper lhs,
                      TaskWrapper rhs)
    {
        return _taskGraph.attach(lhs._value,
                                 rhs._value);
    }

    void detachAll(TaskWrapper task)
    {
        _taskGraph.detachAll(task._value);
    }

    void run()
    {
        while (!_taskGraph.empty())
        {
            auto top = _taskGraph.top();
            top->_value->run();
            _taskGraph.pop(top);
        }

        DMIT_COM_ASSERT(!_taskGraph.isCyclic());
    }

private:

    PoolSet _poolSet;
    TaskGraph _taskGraph;
};

} // namespace dmit::sem
