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

    void registerFence(TaskWrapper task)
    {
        task().insertLock(_taskGraph.attach(task._value,
                                            task._value));
        _fences.push_back(&(task()));
    }

    void unlock(TaskWrapper task)
    {
        if (task().hasLock())
        {
            _taskGraph.detach(task().lock());
            task().removeLock();
        }
    }

    Dependency attach(TaskWrapper lhs,
                      TaskWrapper rhs)
    {
        unlock(lhs);

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

            unlockFences();
        }

        DMIT_COM_ASSERT(!_taskGraph.isCyclic());
    }

    void unlockFences()
    {
        for (auto& fence : _fences)
        {
            if (fence->hasLock())
            {
                _taskGraph.detach(fence->lock());
            }
        }

        _fences.clear();
    }

private:

    PoolSet _poolSet;
    TaskGraph _taskGraph;
    std::vector<Task*> _fences;
};

using Scheduler = dmit::sem::TScheduler<1>;

} // namespace dmit::sem
