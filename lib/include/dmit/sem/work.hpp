#pragma once

#include "dmit/sem/message.hpp"

#include <functional>

namespace dmit::sem
{

template <class Type>
class TWork;

namespace work
{

template <class Type>
class TPool
{

public:

    template <class Function>
    TWork<Type>& make(Function&& function,
                      TMessage<Type>& message)
    {
        return _pool.make(function, *this, message);
    }

    void recycle(TWork<Type>& work)
    {
        _pool.recycle(work);
    }

private:

    pool::TMake<TWork<Type>, 0x10> _pool;
};

} // namespace work

template <class Type>
class TWork
{

public:
    
    template <class Function>
    TWork(Function&& function,
          work::TPool<Type>& pool,
          TMessage<Type>& message) :
        _function{std::move(function)},
        _pool{pool},
        _message{message}
    {}

    void run()
    {
        _message.write(_function);
        _pool.recycle(*this);
    }

private:

    const std::function<Type()> _function;
    work::TPool<Type>&          _pool;

public:

    TMessage<Type>& _message;
};

} // namespace dmit::sem
