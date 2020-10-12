#pragma once

#include "dmit/sem/task.hpp"

#include "dmit/com/assert.hpp"

#include "topo/graph.hpp"

#include <cstdint>
#include <vector>
#include <limits>

namespace dmit::sem
{

template <std::size_t SIZE>
class TScheduler
{

public:

    using TaskGraph  = topo::graph::TMake<task::TAbstract<SIZE>*, SIZE>;
    using PoolSet    = typename TaskGraph::PoolSet;
    using Dependency = typename TaskGraph::EdgeListIt;

    template <class Type>
    using TTaskPool = task::TPool<Type, SIZE>;

    template <class Type>
    using TTaskWrapper = task::TWrapper<Type, SIZE>;

    TScheduler() : _taskGraph{_poolSet} {}

    template <class Type>
    TTaskWrapper<Type> makeTask(TTaskPool<Type>& taskPool)
    {
        auto& task = taskPool.make();
        return TTaskWrapper<Type>{_taskGraph.makeNode(&task)};
    }

    template <class Type>
    void registerFence(TTaskWrapper<Type>& task)
    {
        task().insertLock(_taskGraph.attach(task._value,
                                              task._value));
        _fences.push_back(&(task()));
    }

    template <class Type>
    Dependency attach(TTaskWrapper<Type> lhs,
                      TTaskWrapper<Type> rhs)
    {
        if (lhs().hasLock())
        {
            _taskGraph.detach(lhs().lock());
            lhs().removeLock();
        }

        rhs().work()._message.send();
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
    std::vector<task::TAbstract<SIZE>*> _fences;
};

using Scheduler = dmit::sem::TScheduler<1>;

} // namespace dmit::sem
