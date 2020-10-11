#pragma once

#include "pool/pool.hpp"

#include <cstdint>

namespace dmit::sem
{

template <class Type>
class TMessage;

namespace message
{

class Abstract
{

public:

    void send();

    void retrieve();

    bool isValid() const;

    template <class Type>
    TMessage<Type>& as()
    {
        return reinterpret_cast<TMessage<Type>&>(*this);
    }

private:

    uint32_t _refCount = 0;
};

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
class TMessage : public message::Abstract
{

public:

    TMessage(message::TPool<Type>& pool) :
        _pool{pool}
    {}

    void write(const Type value)
    {
        _value = value;
    }

    Type read()
    {
        this->message::Abstract::retrieve();

        if (!this->message::Abstract::isValid())
        {
            _pool.recycle(*this);
        }

        return _value;
    }

private:

    Type _value;
    message::TPool<Type>& _pool;
};

} // namespace dmit::sem
