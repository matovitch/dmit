#pragma once

#include "dmit/sem/message.hpp"

#include <functional>

namespace dmit::sem
{

template <class Type>
class TWork
{

public:
    
    template <class Function>
    TWork(Function&& function,
          TMessage<Type>& message) :
        _function{std::move(function)},
        _message{message}
    {}

    void run()
    {
        _message.write(_function());
    }

private:

    const std::function<Type()> _function;

public:

    TMessage<Type>& _message;
};

} // namespace dmit::sem
