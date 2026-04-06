#pragma once

#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"

#include "dmit/fmt/formatable.hpp"

#include "schmit/scheduler.hpp"

#include "robin/table.hpp"
#include "robin/map.hpp"

#include <optional>
#include <variant>
#include <memory>
#include <string>

namespace dmit::sem
{

#ifdef DMIT_SEM_CONTEXT_DEBUG
    using SchmitScheduler = schmit::TScheduler<std::string>;
#else
    using SchmitScheduler = schmit::TScheduler<int8_t>;
#endif

using SchmitTaskGraphPoolSet = typename SchmitScheduler::TaskGraphPoolSet;
using SchmitDependency       = typename SchmitScheduler::Dependency;
using SchmitTaskNode         = typename SchmitScheduler::TaskNode;
using SchmitPoolTask         = typename SchmitScheduler::PoolTask;

using SchmitCoroutineStackPool = typename SchmitScheduler::CoroutineStackPool;

struct Context : fmt::Formatable
{
    using DebugType = SchmitScheduler::Debug;

    Context();

    void notifyEvent(const com::UniqueId&, const ast::node::VIndex&);

    ast::node::VIndex vIndex(ast::node::VIndexOrLock& vIndexOrLock);

    std::optional<ast::node::VIndex> getFact(const com::UniqueId&);

    SchmitTaskNode getOrMakeEvent (const com::UniqueId&);

    void run();

    template <class Function>
    void makeTask(Function&& function,
                  const ast::node::VIndexOrLock& astNodeVIndexOrLock,
                  const com::UniqueId& comUniqueId,
                  std::unique_ptr<DebugType> debug)
    {
        if (auto factOpt = getFact(comUniqueId))
        {
            if (std::holds_alternative<SchmitTaskNode>(astNodeVIndexOrLock))
            {
                _unlockSet.emplace(std::get<SchmitTaskNode>(astNodeVIndexOrLock));
            }
            function(factOpt.value());
            return;
        }

        if (std::holds_alternative<ast::node::VIndex>(astNodeVIndexOrLock))
        {
            function(std::get<ast::node::VIndex>(astNodeVIndexOrLock));
            return;
        }

        auto task = _scheduler.makeTask
        (
            [this, astNodeVIndexOrLock, function, comUniqueId]
            {
                if (auto factOpt = getFact(comUniqueId))
                {
                    function(factOpt.value());
                }
                _unlockSet.emplace(std::get<SchmitTaskNode>(astNodeVIndexOrLock));
            },
            std::move(debug)
        );

        auto lock  = std::get<SchmitTaskNode>(astNodeVIndexOrLock);
        auto event = getOrMakeEvent (comUniqueId   );

        _scheduler.attach(task, event);
        _scheduler.attach(event, lock);
    }

    SchmitTaskNode makeLock() { return _scheduler.makeLock(); }

    SchmitScheduler _scheduler;

    robin::map::TMake<com::UniqueId,
                      SchmitTaskNode,
                      com::unique_id::Hasher,
                      com::unique_id::Comparator, 4, 3> _eventMap;

    robin::table::TMake<SchmitTaskNode,
                        SchmitTaskNode::Hasher,
                        SchmitTaskNode::Comparator, 4, 3> _unlockSet;

    robin::map::TMake<com::UniqueId,
                      ast::node::VIndex,
                      com::unique_id::Hasher,
                      com::unique_id::Comparator, 4, 3> _factMap;
};

} // namespace dmit::sem

#ifdef DMIT_SEM_CONTEXT_DEBUG
    #define DMIT_SEM_CONTEXT_STR(str) std::make_unique<std::string>(str)
#else
    #define DMIT_SEM_CONTEXT_STR(str) std::unique_ptr<int8_t>{nullptr}
#endif