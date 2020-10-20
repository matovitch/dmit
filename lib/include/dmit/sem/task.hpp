#pragma once

#include "dmit/sem/work.hpp"

#include "dmit/com/option_reference.hpp"
#include "dmit/com/assert.hpp"

#include "topo/graph.hpp"

#include "pool/pool.hpp"

#include <optional>

namespace dmit::sem
{

template <std::size_t SIZE>
class TTask;

namespace task
{

template <std::size_t SIZE>
class TPool
{

public:

    TTask<SIZE>& make()
    {
        return _pool.make(*this);
    }

    void recycle(TTask<SIZE>& task)
    {
        _pool.recycle(task);
    }

private:

    pool::TMake<TTask<SIZE>, 0x10> _pool;
};


template <std::size_t SIZE>
struct TWrapper
{
    using NodeItType = typename topo::graph::TMake<TTask<SIZE>*, SIZE>::NodeListIt;

    TWrapper(NodeItType value) : _value{value} {}

    TTask<SIZE>& operator()()
    {
        return *(_value->_value);
    }

    NodeItType _value;
};

} // namespace task

template <std::size_t SIZE>
class TTask
{
    using Lock = typename topo::graph::TMake<TTask<SIZE>*, SIZE>::EdgeListIt;
    using Pool = task::TPool<SIZE>;

public:

    TTask(Pool& pool) : _pool{pool} {}

    void insertLock(const Lock lock)
    {
        _lockOpt = lock;
    }

    void removeLock()
    {
        _lockOpt = std::nullopt;
    }

    Lock lock() const
    {
        DMIT_COM_ASSERT(_lockOpt);
        return _lockOpt.value();
    }

    bool hasLock() const
    {
        return _lockOpt.operator bool();
    }

    void assignWork(Work& work)
    {
        _work = work;
    }

    void run()
    {
        if (_work)
        {
            _work.value().get().run();
        }

        _pool.recycle(*this);
    }

private:

    com::OptionReference<Work> _work;
    std::optional<Lock> _lockOpt;
    Pool& _pool;
};

} // namespace dmit::sem
