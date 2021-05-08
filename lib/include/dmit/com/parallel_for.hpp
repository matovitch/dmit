#pragma once

#include "dmit/com/concurrent_queue.hpp"

#include <functional>
#include <cstdint>
#include <thread>

namespace dmit::com
{

template <class Body>
class TParallelFor
{
    using Type = typename Body::ReturnType;

public:

    template <class... Args>
    TParallelFor(Args&&... args) : _concurrentQueue{Body{args...}.size()}
    {
        operator()(std::thread::hardware_concurrency(), std::forward<Args>(args)...);
    }

    const Type& result(uint32_t index) const
    {
        return _concurrentQueue[index];
    }

    ~TParallelFor()
    {
        _concurrentQueue.clean();
    }

private:

    template <class... Args>
    void operator()(uint32_t numThreads, Args&&... args)
    {
        if (numThreads > 1)
        {
            std::thread t1(&TParallelFor::operator()<Args...>, this, numThreads >> 1               , std::ref(args)...);
            std::thread t2(&TParallelFor::operator()<Args...>, this, numThreads - (numThreads >> 1), std::ref(args)...);

            t1.join();
            t2.join();

            return;
        }

        if (!numThreads)
        {
            return;
        }

        Body body{args...};

        while (auto nextPtr = _concurrentQueue.next())
        {
            new (nextPtr) Type(body.run(_concurrentQueue.index(nextPtr)));
        }
    }

    TConcurrentQueue<Type> _concurrentQueue;
};

} // namespace dmit::com
