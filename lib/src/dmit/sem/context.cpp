#include "dmit/sem/context.hpp"

#include "dmit/ast/node.hpp"

#include "dmit/com/logger.hpp"
#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"

#include <dmit/fmt/sem/context.hpp>

#include <optional>

namespace dmit::sem
{

Context::Context() : _scheduler{_taskGraphPoolSet, _taskPool} {}

ast::node::VIndex Context::vIndex(ast::node::VIndexOrLock& vIndexOrLock)
{
    if (std::holds_alternative<ast::node::VIndex>(vIndexOrLock))
    {
        return std::get<ast::node::VIndex>(vIndexOrLock);
    }

    DMIT_COM_ASSERT(_scheduler.isRunning());
    SchmitDependency dependency;
    _scheduler.top()().attach(std::get<SchmitTaskNode>(vIndexOrLock), dependency);

    return std::get<ast::node::VIndex>(vIndexOrLock);
}

SchmitTaskNode Context::getOrMakeEvent(const com::UniqueId& uniqueId)
{
    auto fitEvent = _eventMap.find(uniqueId);

    auto event = (fitEvent != _eventMap.end()) ? fitEvent->second
                                               : _scheduler.makeEvent();
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
    do
    {
        for (auto lock : _unlockSet)
        {
            _scheduler.detachAll(lock);
        }

        _unlockSet.clear();
        _scheduler.run();
    }
    while (!_unlockSet.empty());

    if (_scheduler.isCyclic())
    {
        DMIT_COM_LOG_ERR << "Aborting: " << fmt::asString(*this) << "\n";
        abort();
    }

    //DMIT_COM_ASSERT(!_scheduler.isCyclic());

    _eventMap.clear();
}

} // namespace dmit::sem
