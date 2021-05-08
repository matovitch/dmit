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
    TParallelFor(Args&&... args) : _concurrentQueue{Body{std::forward<Args>(args)...}.size()}
    {
        const auto numThreads       = std::thread::hardware_concurrency();
        const auto numThreadsUpPow2 = 1 << log2(numThreads);

        for (uint64_t i = 0; i < numThreadsUpPow2; i++)
        {
            _bodies.emplace_back(std::make_unique<Body>(std::forward<Args>(args)...));
        }

        operator()(numThreads, numThreadsUpPow2 >> 1, 0);
    }

    const Type& result(uint64_t index) const
    {
        return _concurrentQueue[index];
    }

    ~TParallelFor()
    {
        _concurrentQueue.clean();
    }

private:

    void operator()(uint32_t numThreads, uint32_t weight, uint32_t id)
    {
        if (numThreads > 1)
        {
            std::thread t1(&TParallelFor::operator(), this, numThreads >> 1               , weight >> 1, id);
            std::thread t2(&TParallelFor::operator(), this, numThreads - (numThreads >> 1), weight >> 1, id + weight);

            t1.join();
            t2.join();

            return;
        }

        if (!numThreads)
        {
            return;
        }

        auto index = _concurrentCounter.increment();

        while (_concurrentQueue.isValid(index))
        {
            new (&(_concurrentQueue[index])) Type(_bodies[id]->run(index._value));
            index = _concurrentCounter.increment();
        }
    }

    ConcurrentCounter      _concurrentCounter;
    TConcurrentQueue<Type> _concurrentQueue;

    std::vector<std::unique_ptr<Body>> _bodies; // Bury'em deep it ain't pretty
};

} // namespace dmit::com
