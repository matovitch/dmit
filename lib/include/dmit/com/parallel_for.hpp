#pragma once

#include "dmit/com/concurrent_queue.hpp"
#include "dmit/com/log2.hpp"

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
    TParallelFor(Args&&... args) :
        _numThreads{std::thread::hardware_concurrency()},
        _concurrentQueue{Body{std::forward<Args>(args)...}.size()}
    {
        for (uint64_t i = 0; i < numThreadsUpPow2(); i++)
        {
            _bodies.emplace_back(std::make_unique<Body>(std::forward<Args>(args)...));
        }
    }

    void run()
    {
        operator()(_numThreads, numThreadsUpPow2() >> 1, 0);
    }

    std::vector<Type> makeVector()
    {
        run();

        std::vector<Type> bundles;

        bundles.reserve(size());

        for (int i = 0; i < size(); ++i)
        {
            bundles.emplace_back(result(i));
        }

        return bundles;
    }

    ~TParallelFor()
    {
        _concurrentQueue.clean();
    }

private:

    const Type& result(uint64_t index) const
    {
        return _concurrentQueue[index];
    }

    std::size_t size() const
    {
        return _concurrentQueue.size();
    }

    uint32_t numThreadsUpPow2() const
    {
        return 1 << log2(_numThreads);
    }

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

    uint32_t _numThreads;

    ConcurrentCounter      _concurrentCounter;
    TConcurrentQueue<Type> _concurrentQueue;

    std::vector<std::unique_ptr<Body>> _bodies; // Bury'em deep it ain't pretty
};

} // namespace dmit::com
