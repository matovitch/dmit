#pragma once

#include "dmit/sem/message.hpp"
#include "dmit/sem/work.hpp"

#include "dmit/com/option_reference.hpp"
#include "dmit/com/assert.hpp"

#include "topo/graph.hpp"

#include "pool/pool.hpp"

#include <optional>

namespace dmit::sem
{

template <class Type, std::size_t SIZE>
class TTask;

namespace task
{

template <std::size_t SIZE>
class TAbstract
{
    using Lock = typename topo::graph::TMake<task::TAbstract<SIZE>*, SIZE>::EdgeListIt;

public:

    virtual void run() = 0;

    virtual ~TAbstract() {}

    template <class Type>
    TTask<Type, SIZE>& as()
    {
        return reinterpret_cast<TTask<Type, SIZE>&>(*this);
    }

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

private:

    std::optional<Lock> _lockOpt;
};

} // namespace task

template <class Type, std::size_t SIZE>
class TTask : public task::TAbstract<SIZE>
{

public:

    void assignWork(TWork<Type>& work)
    {
        _work = work;
    }

    TWork<Type>& work()
    {
        DMIT_COM_ASSERT(_work);
        return _work.value().get();
    }

    void run() override
    {
        if (_work)
        {
            work().run();
        }
    }

private:

    com::OptionReference<TWork<Type>> _work;
};

namespace task
{

template <class Type, std::size_t SIZE>
using TPool = pool::TMake<TTask<Type, SIZE>, 0x10>;

template <class Type, std::size_t SIZE>
struct TWrapper
{
    using NodeItType = typename topo::graph::TMake<task::TAbstract<SIZE>*, SIZE>::NodeListIt;

    TWrapper(NodeItType value) : _value{value} {}

    TTask<Type, SIZE>& operator()()
    {
        return _value->_value->template as<Type>();
    }

    NodeItType _value;
};

} // namespace task
} // namespace dmit::sem
