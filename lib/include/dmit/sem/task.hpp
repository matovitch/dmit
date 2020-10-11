#pragma once

#include "dmit/sem/message.hpp"
#include "dmit/sem/work.hpp"

#include "dmit/com/option_reference.hpp"
#include "dmit/com/assert.hpp"

#include "topo/graph.hpp"

#include "pool/pool.hpp"

namespace dmit::sem
{

template <class Type>
class TTask;

namespace task
{

struct Abstract
{
    virtual message::Abstract& message() = 0;

    virtual work::Abstract& work() = 0;

    virtual void run() = 0;

    virtual ~Abstract() {}

    template <class Type>
    TTask<Type>& as()
    {
        return reinterpret_cast<TTask<Type>&>(*this);
    }
};

} // namespace task

template <class Type>
class TTask : public task::Abstract
{

public:

    void assignWork(TWork<Type>& work)
    {
        _work = work;
    }

    work::Abstract& work() override
    {
        DMIT_COM_ASSERT(_work);
        return _work.value().get();
    }

    message::Abstract& message() override
    {
        DMIT_COM_ASSERT(_work);
        return work().message();
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

template <class Type>
using TPool = pool::TMake<TTask<Type>, 0x10>;

template <class Type, std::size_t SIZE>
struct TWrapper
{
    using NodeItType = typename topo::graph::TMake<task::Abstract*, SIZE>::NodeListIt;

    TWrapper(NodeItType value) : _value{value} {}

    TTask<Type>& operator()()
    {
        return _value->_value->template as<Type>();
    }

    NodeItType _value;
};

} // namespace task
} // namespace dmit::sem
