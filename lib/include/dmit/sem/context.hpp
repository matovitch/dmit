#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/src/slice.hpp"

#include "dmit/com/unique_id.hpp"

#include "schmit/scheduler.hpp"

#include "robin/map.hpp"

namespace dmit::sem
{

struct Context
{
    Context(ast::State::NodePool&);

    template <com::TEnumIntegerType<ast::node::Kind> AST_NODE_KIND>
    ast::TNode<AST_NODE_KIND>& get(const ast::node::TIndex<AST_NODE_KIND> index)
    {
        return _astNodePool.get(index);
    }

    src::Slice getSlice(const ast::node::TIndex<ast::node::Kind::LEXEME> lexeme);

    // 1. dmit::ast

    static constexpr std::size_t SIZE = 0x10;

    ast::State::NodePool& _astNodePool;
    ast::node::Location   _astParentScope;

    // 2. schmit

    using Scheduler                 = schmit::TScheduler<SIZE>;
    using SchedulerTaskGraphPoolSet = typename Scheduler::TaskGraphPoolSet;
    using Task                      = typename Scheduler::TaskNode;

    using PoolTask  = typename Scheduler::PoolTask;
    using PoolWork  = typename Scheduler::PoolWork;

    SchedulerTaskGraphPoolSet _schedulerTaskGraphPoolSet;
    Scheduler                 _scheduler;

    PoolTask  _poolTask;
    PoolWork  _poolWork;

    // 3. robin

    template <class Type>
    using TMap = robin::map::TMake<com::UniqueId,
                                   Type,
                                   com::unique_id::Hasher,
                                   com::unique_id::Comparator, 4, 3>;

    using MapTask = TMap<Task>;
    using MapFact = TMap<ast::node::Location>;

    MapTask _mapTask;
    MapFact _mapFact;
};

} // namespace dmit::sem
