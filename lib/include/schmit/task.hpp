#pragma once

#include "schmit/details/coroutine.hpp"

#include "topo/graph.hpp"

#include "pool/intrusive.hpp"

#include <functional>
#include <optional>
#include <utility>

namespace schmit
{

template <class>
class TTask;

template <class>
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

    using NodeItType = typename topo::graph::TMake<TTask<DebugType>*, SIZE>::NodeListIt;

    TNode(NodeItType value) : _value{value} {}

    TTask<DebugType>& operator()()
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

template <class DebugType>
class TTask
{
    friend TScheduler<DebugType>;

    using Dependency = typename topo::graph::TMake<TTask<DebugType>*, 1>::EdgeListIt;
    using Pool       = pool::intrusive::TMake<TTask<DebugType>, 1>;

public:

    template <class Function>
    TTask(Pool& pool,
          schmit::TScheduler<DebugType>& scheduler,
          Function&& function,
          task::TNode<1, DebugType> node,
          std::unique_ptr<DebugType> debug) :
        _scheduler{scheduler},
        _coroutine{scheduler._coroutineStackPool, task::TEntryPoint<1, DebugType>::address()},
        _function{std::move(function)},
        _node{node},
        _pool{pool}
    {
        if (debug)
        {
            _debug = *debug;
        }
    }

    void attach(task::TNode<1, DebugType>& task, Dependency& dependency)
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
        #if defined(USE_ASAN)
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

    TScheduler<DebugType>&      _scheduler;
    schmit_details::Coroutine   _coroutine;
    bool                        _isRunning = false;

    std::function<void()>     _function;
    task::TNode<1, DebugType> _node;
    Pool&                     _pool;

public:

    DebugType _debug;
};

} // namespace schmit
