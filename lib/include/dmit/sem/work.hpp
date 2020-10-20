#pragma once

#include "pool/pool.hpp"

#include <functional>

namespace dmit::sem
{

namespace work
{

class Pool;

} // namespace work

class Work
{

public:

    template <class Function>
    Work(Function&& function,
         work::Pool& pool) :
        _function{std::move(function)},
        _pool{pool}
    {}

    void run();

private:

    const std::function<void()> _function;
    work::Pool& _pool;
};

namespace work
{

class Pool
{

public:

    template <class Function>
    Work& make(Function&& function)
    {
        return _pool.make(function, *this);
    }

    void recycle(Work& work);

private:

    pool::TMake<Work, 0x10> _pool;
};

} // namespace work
} // namespace dmit::sem
