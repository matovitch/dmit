#pragma once

#include "schmit/details/coroutine.hpp"

#include "topo/graph.hpp"

#include "pool/intrusive.hpp"

#include <functional>
#include <optional>
#include <utility>

namespace schmit
{

template <std::size_t>
class TTask;

template <std::size_t>
class TScheduler;

namespace task
{

template <std::size_t SIZE>
struct TNode
{
    using NodeItType = typename topo::graph::TMake<TTask<SIZE>*, SIZE>::NodeListIt;

    TNode(NodeItType value) : _value{value} {}

    TTask<SIZE>& operator()()
    {
        return *(_value->_value);
    }

    bool isPending() const
    {
        return _value->isPending();
    }

    bool isRunning() const
    {
        return _value->_value->isRunning();
    }

    NodeItType _value;
};

template <std::size_t SIZE>
struct TEntryPoint
{
    static void run()
    {
        _next.value()().run();
    }

    constexpr static void* address()
    {
        return reinterpret_cast<void*>(&TEntryPoint<SIZE>::run);
    }

    static thread_local std::optional<TNode<SIZE>> _next;
};

template <std::size_t SIZE>
thread_local std::optional<TNode<SIZE>> TEntryPoint<SIZE>::_next;

} // namespace task

template <std::size_t SIZE>
class TTask
{
    friend TScheduler<SIZE>;

    using Dependency = typename topo::graph::TMake<TTask<SIZE>*, SIZE>::EdgeListIt;

    using Pool = pool::intrusive::TMake<TTask<SIZE>, SIZE>;

public:

    template <std::size_t STACK_SIZE, std::size_t POOL_SIZE, class Function>
    TTask(Pool& pool,
          schmit::TScheduler<SIZE> & scheduler,
          pool::intrusive::TMake<schmit_details::TCoroutine<STACK_SIZE, POOL_SIZE>, POOL_SIZE>& coroutinePool,
          Function&& function,
          task::TNode<SIZE> node) :
        _scheduler{scheduler},
        _coroutine{coroutinePool.make(task::TEntryPoint<SIZE>::address())},
        _function{std::move(function)},
        _node{node},
        _pool{pool}
    {}

    void attach(task::TNode<SIZE>& task, Dependency& dependency)
    {
        dependency = _scheduler.attach(_node, task);

        _scheduler.releaseThread(_node);
    }

    bool isRunning() const
    {
        return _isRunning;
    }

    void run()
    {
        #if defined(SCHMIT_USE_ASAN)
            _coroutine.asanFinish();
        #endif

        _isRunning = true;

        _function();

        _isRunning = false;

        _scheduler.pop(_node);
        _coroutine.recycle();
    }

    void recycle()
    {
        _pool.recycle(*this);
    }

private:

    TScheduler<SIZE>&                    _scheduler;
    schmit_details::coroutine::Abstract& _coroutine;
    bool                                 _isRunning = false;

    std::function<void()> _function;
    task::TNode<SIZE>     _node;
    Pool&                 _pool;
};

} // namespace schmit
