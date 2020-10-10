#pragma once

#include "pool/details/allocator.hpp"

template <class Allocator>
class TPool
{

public:

    using Type = typename Allocator::Type;

    template <class... Args>
    Type& make(Args&&... args)
    {
        return *(new (static_cast<void *>(_allocator.allocate())) Type(std::forward<Args>(args)...));
    }

    void recycle(Type& value)
    {
        _allocator.recycle(&value);
    }

private:

    Allocator _allocator;
};

namespace pool
{

template <class Type, std::size_t SIZE>
using TMake = TPool<pool_details::allocator::TMake<Type, SIZE>>;

} // namespace pool
