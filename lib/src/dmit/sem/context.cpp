#include "dmit/sem/context.hpp"

#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"

#include <optional>

namespace dmit::sem
{

Context::Context() : _scheduler{_taskGraphPoolSet} {}

SchmitTaskNode Context::getOrMakeLock(ast::node::Index astNodeIndex)
{
    auto fitLock = _lockMap.find(astNodeIndex);

    auto lock = (fitLock != _lockMap.end()) ? fitLock->second
                                            : _scheduler.makeTask(_poolTask,
                                                                  _coroutinePoolSmall);
    if (fitLock == _lockMap.end())
    {
        _scheduler.attach(lock, lock);
        _lockMap.emplace(astNodeIndex, lock);
    }

    return lock;
}

void Context::notifyEvent(const com::UniqueId& id)
{
    auto fit = _eventMap.find(id);

    if (fit != _eventMap.end())
    {
        _scheduler.detach(fit->second);
    }

    //_eventMap.erase(fit);
}

void Context::registerEvent(const com::UniqueId& id,
                            std::optional<SchmitDependency> dependencyOpt)
{
    if (dependencyOpt)
    {
        _eventMap.emplace(id, dependencyOpt.value());
    }
}

void Context::run()
{
    _scheduler.run();

    for (auto key : _unlockSet)
    {
        auto fit = _lockMap.find(key);
        _scheduler.detachAll(fit->second);
    }

    //_unlockSet.clear();
    _scheduler.run();

    DMIT_COM_ASSERT(!_scheduler.isCyclic());
}

} // namespace dmit::sem
