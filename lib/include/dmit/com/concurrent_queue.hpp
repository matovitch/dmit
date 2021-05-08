#pragma once

#include "dmit/com/critical_section.hpp"
#include "dmit/com/storage.hpp"

#include <cstdint>
#include <atomic>

namespace dmit::com
{

template <class Type>
class TConcurrentQueue
{

public:

    TConcurrentQueue(uint32_t size) : _storage(size) {}

    Type* next()
    {
        DMIT_COM_CRITICAL_SECTION(_flag);

        return (_offset < _storage._size) ? _storage.data() + (_offset++) : nullptr;
    }

    const uint32_t index(Type* nextPtr) const
    {
        return nextPtr - _storage.data();
    }

    void clean()
    {
        for (uint32_t i = 0; i < _storage._size; i++)
        {
            (_storage.data())[i].~Type();
        }
    }

    const Type& operator[](const uint32_t index) const
    {
        return (_storage.data())[index];
    }

private:

    std::atomic<bool> _flag = false;
    uint32_t          _offset = 0;
    TStorage<Type>    _storage;
};

} // namespace dmit::com
