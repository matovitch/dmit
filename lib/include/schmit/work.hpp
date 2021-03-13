#pragma once

#include "pool/intrusive.hpp"

#include <functional>
#include <cstdint>
#include <utility>

namespace schmit
{

template <std::size_t SIZE>
class TWork
{

public:

    using Pool = pool::intrusive::TMake<TWork<SIZE>, SIZE>;

    template <class Function>
    TWork(Pool& pool, Function&& function) :
        _pool{pool},
        _function{std::move(function)}
    {}

    void run()
    {
        _function();
        _pool.recycle(*this);
    }

private:

    Pool&                 _pool;
    std::function<void()> _function;
};

} // namespace schmit
