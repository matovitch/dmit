#pragma once

#include "dmit/sem/task.hpp"

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
        auto& task = taskPool.make(0);
        return TTaskWrapper<Type>{_taskGraph.makeNode(&task)};
    }

    template <class Type>
    TTaskWrapper<Type> makeTaskAsFence(TTaskPool<Type>& taskPool, const uint64_t fenceTtl)
    {
        auto& task = taskPool.make(_fenceTime + fenceTtl);
        auto taskWrapper = TTaskWrapper<Type>{_taskGraph.makeNode(&task)};

        task.registerLock(_taskGraph.attach(taskWrapper._value,
                                            taskWrapper._value));
        _fences.push_back(&task);
        
        return taskWrapper;
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

            while (_taskGraph.empty() && !_fences.empty())
            {
                unlockFences();
            }
        }
    }

    void unlockFences()
    {
        int limit = _fences.size() - 2;

        while (limit != -1)
        {
            if (_fences[limit + 1] == _fences[limit])
            {
                limit--;
            }
            else
            {
                break;
            }
        }

        limit++;

        for (int i = 0; i < limit; i++)
        {
            if (_fences[i]->fenceTime() > _fences[limit]->fenceTime())
            {
                continue;
            }

            limit = (_fences[i]->fenceTime() < _fences[limit]->fenceTime()) ? _fences.size() - 1
                                                                            : limit - 1;
            auto tmp = _fences[i];
            _fences[i] = _fences[limit];
            _fences[limit] = tmp;
        }

        if (_fenceTime < _fences[limit]->fenceTime())
        {
            _fenceTime = _fences[limit]->fenceTime();
        }

        for (int j = _fences.size() - 1; j >= limit; j--)
        {
            if (_fences[j]->hasLock())
            {
                _taskGraph.detach(_fences[j]->lock());
            }

            _fences.pop_back();
        }
    }

private:

    PoolSet _poolSet;
    TaskGraph _taskGraph;
    std::vector<task::TAbstract<SIZE>*> _fences;
    uint64_t _fenceTime = 0;
};

using Scheduler = dmit::sem::TScheduler<1>;

} // namespace dmit::sem
