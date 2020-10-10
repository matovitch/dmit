#pragma once

#include "dmit/sem/message.hpp"
#include "dmit/sem/work.hpp"

#include "dmit/com/option_reference.hpp"
#include "dmit/com/assert.hpp"

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
class TPool
{

public:

    TTask<Type>& make()
    {
        _pool.emplace_back(std::make_unique<TTask<Type>>());
        return *(_pool.back());
    }

private:

    std::vector<std::unique_ptr<TTask<Type>>> _pool; // TODO replace with topo's pool
};

} // namespace task
} // namespace dmit::sem
