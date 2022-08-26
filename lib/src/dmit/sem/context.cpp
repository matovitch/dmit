#include "dmit/sem/context.hpp"

#include "dmit/ast/node.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"

#include <optional>

namespace dmit::sem
{

Context::Context() : _scheduler{_taskGraphPoolSet} {}

SchmitTaskNode Context::getOrMakeLock(const ast::node::VIndex& astNodeVIndex)
{
    auto fitLock = _lockMap.find(astNodeVIndex);

    auto lock = (fitLock != _lockMap.end()) ? fitLock->second
                                            : _scheduler.makeTask(_poolTask,
                                                                  _coroutinePoolSmall);
    if (fitLock == _lockMap.end())
    {
        _scheduler.attach(lock, lock);
        _lockMap.emplace(astNodeVIndex, lock);
    }

    return lock;
}

SchmitTaskNode Context::getOrMakeEvent(const com::UniqueId& uniqueId)
{
    auto fitEvent = _eventMap.find(uniqueId);

    auto event = (fitEvent != _eventMap.end()) ? fitEvent->second
                                               : _scheduler.makeTask(_poolTask,
                                                                     _coroutinePoolSmall);
    if (fitEvent == _eventMap.end())
    {
        _eventMap.emplace(uniqueId, event);
    }

    return event;
}

void Context::notifyEvent(const com::UniqueId& id, const ast::node::VIndex& vIndex)
{
    auto fit = _eventMap.find(id);

    if (fit != _eventMap.end())
    {
        _scheduler.forcePending(fit->second);
        _eventMap.erase(fit);
    }

    _factMap.emplace(id, vIndex);
}

std::optional<ast::node::VIndex> Context::getFact(const com::UniqueId& id)
{
    auto fit = _factMap.find(id);

    return fit != _factMap.end() ? std::optional<ast::node::VIndex>{fit->second}
                                 : std::nullopt;
}

void Context::run()
{
    _scheduler.run();

    for (auto key : _unlockSet)
    {
        auto fit = _lockMap.find(key);

        if (fit != _lockMap.end())
        {
            _scheduler.detachAll(fit->second);
        }
    }

    _unlockSet.clear();
    _scheduler.run();

    DMIT_COM_ASSERT(!_scheduler.isCyclic());

    _eventMap .clear();
    _lockMap  .clear();
}

} // namespace dmit::sem
