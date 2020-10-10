#pragma once

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

} // namespace message


template <class Type>
class TMessage : public message::Abstract
{

public:

    void write(const Type value)
    {
        _value = value;
    }

    Type read()
    {
        message::Abstract::retrieve();
        return _value;
    }

private:

    Type _value;
};

} // namespace dmit::sem
