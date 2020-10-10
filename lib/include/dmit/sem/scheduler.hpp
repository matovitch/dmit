#pragma once

#include "dmit/sem/task.hpp"

#include "topo/graph/graph.hpp"

#include <cstdint>

namespace dmit::sem
{

template <std::size_t SIZE>
class TScheduler
{
    using TaskGraph  = topo::graph::TMake<task::Abstract*, SIZE>;

public:

    using PoolSet    = typename TaskGraph::PoolSet;
    using Task       = typename TaskGraph::NodeListIt;
    using Dependency = typename TaskGraph::EdgeListIt;

    TScheduler(PoolSet& poolSet) : _taskGraph{poolSet} {}

    template <class Type>
    Task makeTask(task::TPool<Type>& taskPool)
    {
        auto& task = taskPool.make();
        return _taskGraph.makeNode(&task);
    }

    Dependency attach(Task lhs,
                      Task rhs)
    {
        rhs->_value->message().send();
        return _taskGraph.attach(lhs, rhs);
    }

    void run()
    {
        while (!_taskGraph.empty())
        {
            _taskGraph.top()->_value->run();
            _taskGraph.pop(_taskGraph.top());
        }
    }

private:

    TaskGraph _taskGraph;
};

} // namespace dmit::sem
