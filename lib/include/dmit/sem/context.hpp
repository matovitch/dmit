#pragma once

#include "dmit/ast/v_index.hpp"

#include "dmit/com/unique_id.hpp"

#include "schmit/scheduler.hpp"

#include "robin/table.hpp"
#include "robin/map.hpp"

#include <optional>

namespace dmit::sem
{

using SchmitScheduler        = schmit::TScheduler<1>;
using SchmitTaskGraphPoolSet = typename SchmitScheduler::TaskGraphPoolSet;
using SchmitDependency       = typename SchmitScheduler::Dependency;
using SchmitTaskNode         = typename SchmitScheduler::TaskNode;
using SchmitPoolTask         = typename SchmitScheduler::PoolTask;
using SchmitPoolWork         = typename SchmitScheduler::PoolWork;

template <std::size_t STACK_SIZE>
using SchmitCoroutinePool = typename SchmitScheduler::TCoroutinePool<STACK_SIZE>;

struct Context
{
    Context();

    SchmitTaskNode getOrMakeLock  (const ast::node::VIndex & );
    SchmitTaskNode getOrMakeEvent (const com::UniqueId     & );

    void notifyEvent(const com::UniqueId&, const ast::node::VIndex&);

    std::optional<ast::node::VIndex> getFact(const com::UniqueId&);

    void run();

    template <class Function, class CoroutinePool>
    SchmitTaskNode makeTaskFromWork(Function&& function,
                                    CoroutinePool& coroutinePool)
    {
        auto task = _scheduler.makeTask(_poolTask, coroutinePool);

        task().assignWork(_poolWork.make(std::forward<Function>(function)));

        return task;
    }

    template <class Function, class CoroutinePool>
    void makeTask(Function&& function,
                  CoroutinePool& coroutinePool,
                  const ast::node::VIndex& astNodeVIndex,
                  const com::UniqueId& comUniqueId)
    {
        if (auto factOpt = getFact(comUniqueId))
        {
            function(factOpt.value());
            _unlockSet.emplace(astNodeVIndex);
            return;
        }

        auto task = makeTaskFromWork
        (
            [this, astNodeVIndex, function, comUniqueId]
            {
                if (auto factOpt = getFact(comUniqueId))
                {
                    function(factOpt.value());
                    _unlockSet.emplace(astNodeVIndex);
                }
            },
            coroutinePool
        );

        auto lock  = getOrMakeLock  (astNodeVIndex );
        auto event = getOrMakeEvent (comUniqueId   );

        _scheduler.attach(task, event);
        _scheduler.attach(event, lock);
    }

    SchmitTaskGraphPoolSet _taskGraphPoolSet;
    SchmitScheduler        _scheduler;

    SchmitPoolTask _poolTask;
    SchmitPoolWork _poolWork;

    SchmitCoroutinePool<0x2222 /*stack size*/> _coroutinePoolSmall;
    SchmitCoroutinePool<0x5555 /*stack size*/> _coroutinePoolMedium;
    SchmitCoroutinePool<0xffff /*stack size*/> _coroutinePoolLarge;

    robin::map::TMake<com::UniqueId,
                      SchmitTaskNode,
                      com::unique_id::Hasher,
                      com::unique_id::Comparator, 4, 3> _eventMap;

    robin::map::TMake<ast::node::VIndex,
                      SchmitTaskNode,
                      ast::node::v_index::Hasher,
                      ast::node::v_index::Comparator, 4, 3> _lockMap;

    robin::table::TMake<ast::node::VIndex,
                        ast::node::v_index::Hasher,
                        ast::node::v_index::Comparator, 4, 3> _unlockSet;

    robin::map::TMake<com::UniqueId,
                      ast::node::VIndex,
                      com::unique_id::Hasher,
                      com::unique_id::Comparator, 4, 3> _factMap;
};

} // namespace dmit::sem
