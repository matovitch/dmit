#pragma once

#include "pool/pool.hpp"

#include <cstdint>

namespace pool
{

template <class Traits>
class TIntrusive
{
    using Type = typename Traits::Type;
    using Pool = typename Traits::Pool;

public:

    template <class... Args>
    Type& make(Args&&... args)
    {
        return _pool.make(*this, args...);
    }

    void recycle(Type& value)
    {
        return _pool.recycle(value);
    }

private:

    Pool _pool;
};

namespace intrusive
{

template <class TraitsType, std::size_t SIZE>
struct TTraits
{
    using Type = TraitsType;
    using Pool = pool::TMake<Type, SIZE>;
};

template <class Type, std::size_t SIZE>
using TMake = TIntrusive<TTraits<Type, SIZE>>;

} // namespace intrusive
} // namespace pool
