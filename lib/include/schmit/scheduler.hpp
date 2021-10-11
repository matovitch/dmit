#pragma once

#include "schmit/details/coroutine/context_switch.hpp"
#include "schmit/task.hpp"

#include "topo/graph.hpp"

#include <cstdint>

namespace schmit
{

template <std::size_t SIZE>
class TScheduler
{
    using TaskGraph = topo::graph::TMake<task::TAbstract<SIZE>*, SIZE>;

public:

    using TaskGraphPoolSet = typename TaskGraph::PoolSet;
    using Dependency       = typename TaskGraph::EdgeListIt;
    using TaskNode         = task::TNode<SIZE>;

    template <std::size_t STACK_SIZE>
    using TCoroutinePool = typename schmit_details::TCoroutine<STACK_SIZE, SIZE>::Pool;

    using PoolTask = typename TTask<SIZE>::Pool;
    using PoolWork = typename TWork<SIZE>::Pool;

    TScheduler(TaskGraphPoolSet& taskGraphPoolSet) :
        _coroutine {_coroutinePool.make(nullptr)},
        _taskGraph {taskGraphPoolSet}
    {}

    template <class CoroutinePool>
    TaskNode makeTask(PoolTask& taskPool, CoroutinePool& coroutinePool)
    {
        auto& task = taskPool.make(*this, coroutinePool);
        TaskNode taskNode{_taskGraph.makeNode(&task)};
        task.setNode(taskNode);

        return taskNode;
    }

    void run()
    {
        _isRunning = true;

        while (!_taskGraph.empty())
        {
            schmit_details::coroutine::contextSwitch(_coroutine,
                                                     nextCoroutine());
        }

        _isRunning = false;
    }

    Dependency attach(TaskNode lhs,
                      TaskNode rhs)
    {
        return _taskGraph.attach(lhs._value,
                                 rhs._value);
    }

    void detachAll(TaskNode taskNode)
    {
        _taskGraph.detachAll(taskNode._value);
    }

    schmit_details::coroutine::Abstract& nextCoroutine()
    {
        if (_taskGraph.empty())
        {
            return _coroutine;
        }

        auto top = _taskGraph.top();

        if (!top->_value->isRunning())
        {
            task::TEntryPoint<SIZE>::_next = top;
        }

        return top->_value->_coroutine;
    }

    void releaseThread(TaskNode taskNode)
    {
        if (!_isRunning || (taskNode.isRunning() && taskNode.isPending()))
        {
            return;
        }

        schmit_details::coroutine::contextSwitch(taskNode()._coroutine,
                                                 nextCoroutine());
    }

    void pop(TaskNode taskNode)
    {
        // Note: The pop() bellow release the memory from 'task' to the allocator
        // but this is safe as it is destroyed only when the slot gets reallocated
        // (this is NOT thread-safe, as is everything here)
        _taskGraph.pop(taskNode._value);

        releaseThread(taskNode);
    }

    bool isCyclic() const
    {
        return _taskGraph.isCyclic();
    }

    void detach(Dependency dependency)
    {
        _taskGraph.detach(dependency);
    }

private:

    bool _isRunning = false;

    schmit_details::coroutine::Abstract& _coroutine;
    TaskGraph _taskGraph;

    static schmit_details::TCoroutine<0x1000, 1>::Pool _coroutinePool;
};

template <std::size_t SIZE>
schmit_details::TCoroutine<0x1000, 1>::Pool TScheduler<SIZE>::_coroutinePool;

} // namespace schmit
