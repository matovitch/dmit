#pragma once

#include "dmit/com/type_flag.hpp"

#include <memory>

namespace dmit::com
{

struct Singletonable{};

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(Singletonable, Type)>
class TSingleton
{

public:

    static Type& instance()
    {
        if (!_instancePtr)
        {
            _instancePtr = std::make_unique<Type>();
        }

        return *_instancePtr;
    }

private:

    thread_local static std::unique_ptr<Type> _instancePtr;
};

template <class Type, type_flag::THas<Singletonable, Type> IS_SINGLETONABLE>
thread_local std::unique_ptr<Type> TSingleton<Type, IS_SINGLETONABLE>::_instancePtr;

} // namespace dmit::com
