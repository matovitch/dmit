#pragma once

#include "dmit/com/concurrent_counter.hpp"
#include "dmit/com/storage.hpp"

#include <cstdint>
#include <atomic>

namespace dmit::com
{

template <class Type>
class TConcurrentQueue
{

public:

    TConcurrentQueue(uint64_t size) : _storage(size) {}

    bool isValid(const ConcurentU64 concurrentU64)
    {
        return concurrentU64._value < _storage._size;
    }

    Type& operator[](const ConcurentU64 concurrentU64)
    {
        return (_storage.data())[concurrentU64._value];
    }

    const Type& operator[](const uint64_t index) const
    {
        return (_storage.data())[index];
    }

    void clean()
    {
        for (uint64_t i = 0; i < _storage._size; i++)
        {
            (_storage.data())[i].~Type();
        }
    }

private:

    TStorage<Type> _storage;
};

} // namespace dmit::com
