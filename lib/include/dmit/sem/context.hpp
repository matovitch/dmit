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

    Context(const dmit::src::Partition& srcPartition,
            ast::State::NodePool&       astNodePool);

    TScheduler<SIZE>            _scheduler;
    const dmit::src::Partition& _srcPartition;
    ast::State::NodePool&       _astNodePool;

    std::unordered_map<com::UniqueId, task::TAbstract<SIZE>*, com::unique_id::Hasher,
                                                              com::unique_id::Comparator> _taskMap;
    TScheduler<SIZE>::TTaskPool <void> _taskVoidPool;
    work::TPool                 <void> _workVoidPool;
    TMessage                    <void> _mesgVoid;
};

} // namespace dmit::sem
