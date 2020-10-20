#pragma once

#include "dmit/sem/scheduler.hpp"
#include "dmit/sem/task.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/src/partition.hpp"

#include "dmit/com/unique_id.hpp"

#include <unordered_map>
#include <cstdint>

namespace dmit::sem
{

struct Context
{
    static constexpr std::size_t SIZE = 0x10;

    using Scheduler   = TScheduler <SIZE>;
    using TaskWrapper = typename Scheduler::TaskWrapper;


    Context(const dmit::src::Partition& srcPartition,
            ast::State::NodePool&       astNodePool);

    const dmit::src::Partition& _srcPartition;
    ast::State::NodePool&       _astNodePool;

    TScheduler<SIZE>    _scheduler;
    Scheduler::TaskPool _taskPool;
    work::Pool          _workPool;

    std::unordered_map<com::UniqueId, TaskWrapper, com::unique_id::Hasher,
                                                   com::unique_id::Comparator> _taskMap;

    std::unordered_map<com::UniqueId, ast::node::Location, com::unique_id::Hasher,
                                                           com::unique_id::Comparator> _factMap;
    static const com::UniqueId ARGUMENT_OF;
    static const com::UniqueId DEFINE;
};

} // namespace dmit::sem
