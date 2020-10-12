#pragma once

#include "pool/pool.hpp"

#include <functional>
#include <cstdint>

namespace dmit::sem
{

template <class Type>
class TMessage;

namespace message
{

template <class Type>
class TPool
{

public:

    TMessage<Type>& make()
    {
        return _pool.make(*this);
    }

    void recycle(TMessage<Type>& message)
    {
        _pool.recycle(message);
    }

private:

    pool::TMake<TMessage<Type>, 0x10> _pool;
};

} // namespace message

template <class Type>
class TMessage
{

public:

    TMessage(message::TPool<Type>& pool) :
        _pool{pool}
    {}

    void send()
    {
        _refCount++;
    }

    void write(const std::function<Type()>& function)
    {
        _value = function();
    }

    Type read()
    {
        _refCount--;

        if (!_refCount)
        {
            _pool.recycle(*this);
        }

        return _value;
    }

private:

    uint32_t _refCount = 0;
    Type _value;
    message::TPool<Type>& _pool;
};

template <>
struct TMessage<void>
{
    void send() {}

    void write(const std::function<void()>& function)
    {
        function();
    }
};

} // namespace dmit::sem
