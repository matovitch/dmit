#pragma once

#include "schmit/details/coroutine.hpp"

#include "topo/graph.hpp"

#include "pool/intrusive.hpp"

#include <functional>
#include <optional>
#include <utility>
#include <memory>

namespace schmit
{

template <std::size_t, class>
class TTask;

template <std::size_t, class>
class TScheduler;

namespace task
{

template <std::size_t SIZE, class DebugType>
struct TNode
{
    struct Hasher
    {
        std::size_t operator()(const TNode& node) const
        {
            auto hash = reinterpret_cast<uint64_t>(&(*(node._value)));
            hash ^= (hash >> 33);
            hash *= 0xff51afd7ed558ccd;
            hash ^= (hash >> 33);
            hash *= 0xc4ceb9fe1a85ec53;
            hash ^= (hash >> 33);
            return hash;
        }
    };

    struct Comparator
    {
        bool operator()(const TNode& lhs, const TNode& rhs)
        {
            return lhs._value == rhs._value;
        }
    };

    using NodeItType = typename topo::graph::TMake<TTask<SIZE, DebugType>*, SIZE>::NodeListIt;

    TNode(NodeItType value) : _value{value} {}

    TTask<SIZE, DebugType>& operator()()
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

template <std::size_t SIZE, class DebugType>
struct TEntryPoint
{
    static void run()
    {
        _next.value()().run();
    }

    constexpr static void* address()
    {
        return reinterpret_cast<void*>(&TEntryPoint<SIZE, DebugType>::run);
    }

    static thread_local std::optional<TNode<SIZE, DebugType>> _next;
};

template <std::size_t SIZE, class DebugType>
thread_local std::optional<TNode<SIZE, DebugType>> TEntryPoint<SIZE, DebugType>::_next;

} // namespace task

template <std::size_t SIZE, class DebugType>
class TTask
{
    friend TScheduler<SIZE, DebugType>;

    using Dependency = typename topo::graph::TMake<TTask<SIZE, DebugType>*, SIZE>::EdgeListIt;

    using Pool = pool::intrusive::TMake<TTask<SIZE, DebugType>, SIZE>;

public:

    template <std::size_t STACK_SIZE, std::size_t POOL_SIZE, class Function>
    TTask(Pool& pool,
          schmit::TScheduler<SIZE, DebugType>& scheduler,
          pool::intrusive::TMake<schmit_details::TCoroutine<STACK_SIZE, POOL_SIZE>, POOL_SIZE>& coroutinePool,
          Function&& function,
          task::TNode<SIZE, DebugType> node,
          std::unique_ptr<DebugType> debug) :
        _scheduler{scheduler},
        _coroutine{coroutinePool.make(task::TEntryPoint<SIZE, DebugType>::address())},
        _function{std::move(function)},
        _node{node},
        _pool{pool}
    {
        if (debug)
        {
            _debug = *debug;
        }
    }

    void attach(task::TNode<SIZE, DebugType>& task, Dependency& dependency)
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

    TScheduler<SIZE, DebugType>&         _scheduler;
    schmit_details::coroutine::Abstract& _coroutine;
    bool                                 _isRunning = false;

    std::function<void()>        _function;
    task::TNode<SIZE, DebugType> _node;
    Pool&                        _pool;

public:

    DebugType _debug;
};

} // namespace schmit
