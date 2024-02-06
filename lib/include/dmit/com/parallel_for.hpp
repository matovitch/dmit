#pragma once

#include "dmit/com/storage.hpp"

#include <condition_variable>
#include <type_traits>
#include <cstdint>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

namespace dmit::com
{

namespace parallel_for
{

namespace thread_pool
{

struct Job
{
    virtual void run(uint32_t threadId, int32_t index) = 0;

    virtual int32_t size() const = 0;
};

} // namespace thread_pool

struct ThreadPool
{
    ThreadPool(const ThreadPool&) = delete;

    ThreadPool(uint32_t numThreads) : _threads{numThreads}
    {
        int32_t i = 0;

        for (auto& thread : _threads)
        {
            new (&thread) std::thread{&ThreadPool::processFunction, this, i++};
        }
    }

    uint32_t size() const
    {
        return _threads._size;
    }

    void processFunction(int id)
    {
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(_mutex_todo);
                _condition_variable_todo.wait(lock, [this] { return _todo > 0; });
            }

            if (_exit)
            {
                break;
            }

            while (true)
            {
                int32_t todo = --_todo;

                if (todo < 0)
                {
                    break;
                }

                _job->run(id, todo);

                int32_t done = --_done;

                if (done == 0)
                {
                    _condition_variable_done.notify_one();
                }
            }
        }
    }

    void notify_and_wait(thread_pool::Job& job)
    {
        // Notifiy
        _job = &job;

        const auto jobSize = job.size();

        _done = jobSize;
        _todo = jobSize;

        _condition_variable_todo.notify_all();

        // Wait
        {
            std::unique_lock<std::mutex> lock(_mutex_done);
            _condition_variable_done.wait(lock, [this] { return !_done; });
        }
    }

    ~ThreadPool()
    {
        _exit = true;

        _todo = 1;
        _condition_variable_todo.notify_all();

        for (auto& thread : _threads)
        {
            thread.join();
        }
    }

    TStorage<std::thread> _threads;
    thread_pool::Job* _job;
    bool _exit = false;

    std::mutex _mutex_todo;
    std::mutex _mutex_done;
    std::condition_variable _condition_variable_todo;
    std::condition_variable _condition_variable_done;
    std::atomic<int32_t> _todo = 0;
    std::atomic<int32_t> _done = 0;
};

template <class ThreadContextType, class OutputType>
struct TJob
{
    using ThreadContext = ThreadContextType;
    using Output = OutputType;

    virtual void run(ThreadContext& threadContext, int32_t index, Output* output) = 0;

    virtual int32_t size() const = 0;
};

template <class ThreadContext>
struct TThreadContexts
{
    template <class... Args>
    TThreadContexts(ThreadPool& threadPool, Args&&... args) :
        _size{static_cast<uint64_t>(threadPool.size())},
        _threadContexts{new ThreadContext[_size]{std::forward<Args>(args)...}}
    {}

    ThreadContext& operator[](const uint64_t index)
    {
        return _threadContexts[index];
    }

    ~TThreadContexts()
    {
        delete[] _threadContexts;
    }

    uint64_t        _size;
    ThreadContext* _threadContexts;
};

} // namespace parallel_for


template <class JobType>
struct TParallelFor : parallel_for::thread_pool::Job
{
    using ThreadContext = typename JobType::ThreadContext;
    using Output        = typename JobType::Output;

    template <class... Args>
    TParallelFor(parallel_for::TThreadContexts<ThreadContext>& threadContexts, Args&&... args) :
        _threadContexts{&threadContexts},
        _job{std::forward<Args>(args)...},
        _outputs{static_cast<uint64_t>(_job.size())}
    {}

    void run(uint32_t threadId, int32_t index) override
    {
        const int32_t reversed_index = _job.size() - index - 1;

        _job.run((*_threadContexts)[threadId], reversed_index, _outputs.data() + reversed_index);
    }

    int32_t size() const override
    {
        return _job.size();
    }

    parallel_for::TThreadContexts<ThreadContext>* _threadContexts;
    JobType          _job;
    TStorage<Output> _outputs;
};

} // namespace dmit::com