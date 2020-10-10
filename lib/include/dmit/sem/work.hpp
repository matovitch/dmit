#pragma once

#include "dmit/sem/message.hpp"

#include <functional>

namespace dmit::sem
{

template <class Type>
class TWork;

namespace work
{

struct Abstract
{
    virtual void run() = 0;

    virtual message::Abstract& message() = 0;

    virtual ~Abstract(){}

    template <class Type>
    TWork<Type>& as()
    {
        return reinterpret_cast<TWork<Type>&>(*this);
    }
};

} // namespace work

template <class Type>
class TWork : public work::Abstract
{

public:
    
    template <class Function>
    TWork(Function&& function,
          TMessage<Type>& message) :
        _function{std::move(function)},
        _message{message}
    {}

    message::Abstract& message() override
    {
        return _message;
    }

    void run() override
    {
        _message.write(_function());
    }

private:

    const std::function<Type()> _function;
    TMessage<Type>& _message;
};

} // namespace dmit::sem
