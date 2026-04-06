#pragma once

#include "schmit/details/coroutine/context_switch.hpp"
#include "schmit/task.hpp"

#include "topo/graph.hpp"

#include <memory>

namespace schmit
{

template <class DebugType>
class TScheduler
{
    using TaskGraph = topo::graph::TMake<TTask<DebugType>*, 1>;

public:

    using TaskGraphPoolSet = typename TaskGraph::PoolSet;
    using Dependency       = typename TaskGraph::EdgeListIt;
    using DependencyList   = typename TaskGraph::EdgeList;
    using TaskNode         = task::TNode<1, DebugType>;

    using Debug = DebugType;

    using PoolTask = typename TTask<DebugType>::Pool;

    using CoroutineStackPool = schmit_details::coroutine::stack::Pool;

    TScheduler() :
        _coroutine {_coroutineStackPool, nullptr},
        _taskGraph {_taskGraphPoolSet}
    {}

    template <class Function>
    TaskNode makeTask(Function&& function, std::unique_ptr<DebugType> debug)
    {
        TaskNode taskNode{_taskGraph.makeNode(nullptr)};
        auto& task = _taskPool.make(*this, std::forward<Function>(function), taskNode, std::move(debug));
        taskNode._value->_value = &task;

        return taskNode;
    }

    TaskNode makeLock()
    {
        TaskNode taskNode{_taskGraph.makeNode(nullptr)};

        _taskGraph.attach(taskNode._value, taskNode._value);

        return taskNode;
    }

    TaskNode makeEvent()
    {
        TaskNode taskNode{_taskGraph.makeNode(nullptr)};

        return taskNode;
    }

    TaskNode top()
    {
        return _taskGraph.top();
    }

    auto ctop() const
    {
        return _taskGraph.ctop();
    }

    auto cpot() const
    {
        return _taskGraph.cpot();
    }

    DependencyList dependencies() const
    {
        return _taskGraph._edges;
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

    void forcePending(TaskNode taskNode)
    {
        _taskGraph.forcePending(taskNode._value);
    }

    schmit_details::Coroutine& nextCoroutine()
    {
        auto top = _taskGraph.top();

        if (!top->_value)
        {
            _taskGraph.pop(top);
            return _taskGraph.empty() ? _coroutine : nextCoroutine();
        }

        if (!top->_value->isRunning())
        {
            task::TEntryPoint<1, DebugType>::_next = top;
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
                                                 _taskGraph.empty() ? _coroutine : nextCoroutine());
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

    bool isRunning() const
    {
        return _isRunning;
    }

    // Keep this before the task pool: coroutine destruction recycles stacks here,
    // so the stack pool must outlive every task-owned coroutine.
    schmit_details::coroutine::stack::Pool _coroutineStackPool;

private:

    bool _isRunning = false;

    schmit_details::Coroutine _coroutine;

    TaskGraphPoolSet _taskGraphPoolSet;
    TaskGraph        _taskGraph;
    PoolTask         _taskPool;
};


} // namespace schmit
