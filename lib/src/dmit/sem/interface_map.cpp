#include "dmit/sem/interface_map.hpp"

#include "dmit/ast/copy_shallow.hpp"
#include "dmit/ast/visitor.hpp"
#include "dmit/ast/bundle.hpp"
#include "dmit/ast/lexeme.hpp"
#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/murmur.hpp"

#include "schmit/scheduler.hpp"

#include <optional>
#include <vector>

namespace dmit::sem
{

namespace
{

using Scheduler                 = schmit::TScheduler<1>;
using SchedulerTaskGraphPoolSet = typename Scheduler::TaskGraphPoolSet;
using PoolTask                  = typename Scheduler::PoolTask;
using PoolWork                  = typename Scheduler::PoolWork;
using PoolCoroutine             = typename Scheduler::TCoroutinePool<0xffff /*stack size*/>;
using Dependency                = typename Scheduler::Dependency;
using TaskNode                  = typename Scheduler::TaskNode;

struct Conductor
{
    Conductor(Scheduler& scheduler) : _scheduler{scheduler} {}

    TaskNode getOrMakeLock(ast::node::Index astNodeIndex)
    {
        auto fitLock = _lockMap.find(astNodeIndex);

        auto lock = (fitLock != _lockMap.end()) ? fitLock->second
                                                : _scheduler.makeTask(_poolTask,
                                                                      _poolCoroutine);
        if (fitLock == _lockMap.end())
        {
            _scheduler.attach(lock, lock);
            _lockMap.emplace(astNodeIndex, lock);
        }

        return lock;
    }

    template <class Function>
    std::optional<Dependency> makeLockedTask(ast::node::Index astNodeIndex, Function&& function)
    {
        if (function())
        {
            return std::nullopt;
        }

        auto& work = _poolWork.make
        (
            [this, astNodeIndex, function]
            {
                function();
                _unlockSet.emplace(astNodeIndex);
            }
        );

        auto task = _scheduler.makeTask(_poolTask, _poolCoroutine);
        task().assignWork(work);

        auto lock = getOrMakeLock(astNodeIndex);

        return _scheduler.attach(task, lock);
    }

    void notifyEvent(const com::UniqueId& id)
    {
        auto fit = _eventMap.find(id);

        if (fit != _eventMap.end())
        {
            _scheduler.detach(fit->second);
        }
    }

    void registerEvent(const com::UniqueId& id, std::optional<Dependency> dependencyOpt)
    {
        if (dependencyOpt)
        {
            _eventMap.emplace(id, dependencyOpt.value());
        }
    }

    void cleanLocks()
    {
        for (auto key : _unlockSet)
        {
            auto fit = _lockMap.find(key);
            _scheduler.detachAll(fit->second);
        }

        _scheduler.run();
    }

    Scheduler&    _scheduler;

    PoolCoroutine  _poolCoroutine;
    PoolWork       _poolWork;
    PoolTask       _poolTask;

    InterfaceMap::TMap<Dependency> _eventMap;

    robin::map::TMake<ast::node::Index,
                      TaskNode,
                      ast::node::index::Hasher,
                      ast::node::index::Comparator, 4, 3> _lockMap;

    robin::table::TMake<ast::node::Index,
                        ast::node::index::Hasher,
                        ast::node::index::Comparator, 4, 3> _unlockSet;
};


struct Stack
{
    com::UniqueId _prefix;
};

struct InterfaceMaker : ast::TVisitor<InterfaceMaker, Stack>
{
    InterfaceMaker(ast::State::NodePool      & astNodePool,
                   InterfaceMap::SymbolTable & symbolTable,
                   Scheduler& scheduler) :
        TVisitor<InterfaceMaker, Stack>{astNodePool},
        _symbolTable{symbolTable},
        _conductor{scheduler}
    {}

    DMIT_AST_VISITOR_SIMPLE();

    void operator()(ast::node::TIndex<ast::node::Kind::TYPE_CLAIM> typeClaimIdx)
    {
        auto typeIdx = get(typeClaimIdx)._type;

        auto&& slice = ast::lexeme::getSlice(get(get(typeIdx)._name)._lexeme, _nodePool);

        const com::UniqueId sliceId{slice._head, slice.size()};

        auto id = com::murmur::combine(sliceId,_stackPtrIn->_prefix);

        auto dependencyOpt = _conductor.makeLockedTask
        (
            typeIdx,
            [this, id, typeIdx]()
            {
                auto status = (_symbolTable.find(id) != _symbolTable.end());

                if (status)
                {
                    com::blit(id, get(typeIdx)._id);
                }

                return status;
            }
        );

        _conductor.registerEvent(id, dependencyOpt);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEF_CLASS> defClassIdx)
    {
        auto& defClass = get(defClassIdx);

        base()(defClass._members);

        auto&& slice = ast::lexeme::getSlice(get(defClass._name)._lexeme, _nodePool);

        com::murmur::combine(
            com::UniqueId{slice._head, slice.size()},
            _stackPtrIn->_prefix
        );

        _symbolTable.emplace(_stackPtrIn->_prefix, defClassIdx);

        _conductor.notifyEvent(_stackPtrIn->_prefix);
    }


    void operator()(ast::node::TIndex<ast::node::Kind::DEF_FUNCTION> functionIdx)
    {
        base()(get(functionIdx)._arguments);
    }

    void operator()(ast::node::TIndex<ast::node::Kind::DEFINITION> definitionIdx)
    {
        auto& definition = get(definitionIdx);

        if (definition._status == ast::DefinitionStatus::EXPORTED)
        {
            base()(definition._value);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::MODULE> moduleIdx)
    {
        auto& module = get(moduleIdx);

        _stackPtrIn->_prefix = module._id;

        base()(module._definitions);

        for (uint32_t i = 0; i < module._imports._size; i++)
        {
            _stackPtrIn->_prefix = get(module._imports[i])._id;
            base()(module._definitions);
        }
    }

    void operator()(ast::node::TIndex<ast::node::Kind::VIEW> viewIdx)
    {
        base()(get(viewIdx)._modules);
    }

    void cleanLocks()
    {
        _conductor.cleanLocks();
    }

    InterfaceMap::SymbolTable& _symbolTable;

    Conductor _conductor;
};

} // namespace

InterfaceMap::InterfaceMap(const std::vector<ast::Bundle>& bundles, ast::State::NodePool& astNodePool) :
    _astNodePool{astNodePool}
{
    for (const auto& bundle : bundles)
    {
        if (!bundle._views._size)
        {
            continue;
        }

        auto& views = _viewsPool.make();

        _astNodePool.make(views, bundle._views._size);

        for (uint32_t i = 0; i < bundle._views._size; i++)
        {
            _asSimpleMap.emplace(bundle._nodePool.get(bundle._views[i])._id, views[i]);
        }
    }
}

void InterfaceMap::registerBundle(ast::Bundle& bundle)
{
    // Create the scheduler
    SchedulerTaskGraphPoolSet schedulerTaskGraphPoolSet;
    Scheduler scheduler{schedulerTaskGraphPoolSet};

    // Create the task
    PoolWork      poolWork;
    PoolTask      poolTask;
    PoolCoroutine poolCoroutine;

    auto task = scheduler.makeTask(poolTask, poolCoroutine);

    // Create and assign the work
    InterfaceMaker interfaceMaker{bundle._nodePool, _symbolTable, scheduler};

    auto& work = poolWork.make
    (
        [&interfaceMaker, &bundle]()
        {
            interfaceMaker.base()(bundle._views);
        }
    );

    task().assignWork(work);

    // Run the scheduler
    scheduler.run();

    // Clean the locks and check if the scheduler ended on a cycle
    interfaceMaker.cleanLocks();
    DMIT_COM_ASSERT(!scheduler.isCyclic());

    // Copy the views
    for (uint32_t i = 0; i < bundle._views._size; i++)
    {
        const auto viewId = bundle._nodePool.get(bundle._views[i])._id;

        ast::copyShallow(bundle._views[i],
                         bundle._nodePool,
                         _asSimpleMap.at(viewId),
                         _astNodePool);
    }
}

} // namespace dmit::sem
