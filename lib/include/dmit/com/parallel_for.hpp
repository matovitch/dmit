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
    virtual ~Job() = default;

    virtual void run(uint32_t threadId, int32_t index) = 0;

    virtual int32_t size() const = 0;
};

} // namespace thread_pool

struct ThreadPool
{
    ThreadPool            (const ThreadPool&) = delete;
    ThreadPool& operator= (const ThreadPool&) = delete;

    explicit ThreadPool(uint32_t numThreads)
        : _threads(numThreads)
    {
        int32_t i = 0;

        for (auto& thread : _threads)
        {
            new (&thread) std::thread{&ThreadPool::processFunction, this, i++};
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex_todo);
            // Signal threads to exit.
            // Use memory_order_release to ensure that all previous writes by this thread
            // (e.g., to other shared variables, though not critical here for _exit itself)
            // become visible to other threads that acquire this atomic variable.
            _exit.store(true, std::memory_order_release);
        }

        // Notify all waiting threads to wake up and check the exit condition.
        _condition_variable_todo.notify_all();

        // Join all threads.
        for (auto& thread : _threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    void processFunction(uint32_t thread_id)
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(_mutex_todo);
            // Wait until there's work (_todo > 0) OR it's time to exit (_exit is true).
            // The lambda predicate protects against spurious wakeups.
            _condition_variable_todo.wait(lock, [this] {
                // Use memory_order_acquire for _exit.load() to synchronize with the
                // release store in the destructor. This ensures that if we see _exit == true,
                // we also see any writes that happened before _exit was set to true.
                // For _todo, relaxed is fine as its consistency is primarily managed
                // by the mutex and other atomic operations on it.
                return _todo.load(std::memory_order_relaxed) > 0 || _exit.load(std::memory_order_acquire);
            });

            // After waking up, re-check _exit under the lock.
            // If _exit is true, this thread should terminate.
            if (_exit.load(std::memory_order_acquire)) // Use acquire for consistency
            {
                // Lock is automatically released when unique_lock goes out of scope.
                break;
            }

            // If not exiting, it means _todo.load() > 0 (or became > 0 while waiting).
            // Release the lock before processing the job item. This allows other threads
            // to acquire the lock (e.g., to wait or for the main thread to submit new jobs)
            // and improves concurrency.
            lock.unlock();

            // Inner loop to process tasks from the current job.
            // This loop continues as long as there are tasks (_todo > 0 for this job).
            while (true)
            {
                // Atomically decrement _todo and get the task index.
                // fetch_sub returns the value *before* subtraction.
                // So, if _todo was N, it returns N, and _todo becomes N-1.
                // The task index would be N-1.
                int32_t task_index = _todo.fetch_sub(1, std::memory_order_relaxed) - 1;

                if (task_index < 0)
                {
                    // No task was obtained (e.g., _todo was 0 or became 0 due to another thread).
                    // We "over-decremented" _todo, so add 1 back to correct it.
                    _todo.fetch_add(1, std::memory_order_relaxed);
                    break; // Exit this inner loop, go back to wait on condition variable.
                }

                // If we are here, task_index is a valid index (>= 0).
                // _job must be valid if _todo > 0 was set by notify_and_wait.
                _job->run(thread_id, task_index);

                // Atomically decrement _done.
                // If _done reaches 0, it means all tasks for the current job are completed.
                // Use memory_order_acq_rel for _done.fetch_sub:
                // - 'release' part: ensures that writes related to task completion (in _job->run)
                //   are visible to the main thread when it wakes up after seeing _done == 0.
                // - 'acquire' part: (less critical here but good practice for RMW on sync vars)
                //   ensures this thread sees prior modifications to _done by other workers.
                // The result of fetch_sub is the value *before* subtraction.
                // So, if _done was 1 (last task), fetch_sub returns 1, _done becomes 0.
                if (_done.fetch_sub(1, std::memory_order_acq_rel) - 1 == 0)
                {
                    // This thread finished the very last task of the job.
                    // Notify the main thread waiting in notify_and_wait.
                    _condition_variable_done.notify_one();
                }
            }
        }
    }

    uint32_t size() const
    {
        return _threads._size;
    }

    void notify_and_wait(thread_pool::Job& job)
    {
        const auto jobSize = job.size();

        if (jobSize <= 0)
        {
            return;
        }

        // Set the job and initialize counters.
        // These operations need to be visible to worker threads before they start processing.
        this->_job = &job; // Assign job pointer.
        _done.store(jobSize, std::memory_order_relaxed); // Initialize done counter. Relaxed is fine as
                                                         // the main thread uses a CV with its own mutex.

        { // Lock scope for modifying _todo, which is related to _condition_variable_todo.
            std::unique_lock<std::mutex> lock(_mutex_todo);
            _todo.store(jobSize, std::memory_order_relaxed); // Set todo counter under lock.
                                                             // Relaxed is fine as the mutex and CV
                                                             // provide synchronization for workers.
        }
        // The writes to _job, _done, and _todo are now complete.
        // Notify worker threads that a new job is available.
        _condition_variable_todo.notify_all();

        // Wait for all tasks of this job to be completed.
        {
            std::unique_lock<std::mutex> lock(_mutex_done);
            // Predicate: wait if _done is not zero (i.e., tasks are still remaining).
            // Use memory_order_acquire on _done.load() to synchronize with the
            // release part of _done.fetch_sub(1, std::memory_order_acq_rel) in workers.
            // This ensures that if we see _done == 0, we also see all writes
            // from worker threads that completed tasks.
            _condition_variable_done.wait(lock, [this] {
                return _done.load(std::memory_order_acquire) == 0;
            });
        }
    }

private:

    TStorage<std::thread> _threads;
    thread_pool::Job* _job; // Pointer to the current job being processed.

    // Synchronization primitives
    std::mutex _mutex_todo; // Mutex for _condition_variable_todo and related shared data (_todo, _exit for signaling)
    std::mutex _mutex_done; // Mutex for _condition_variable_done and related shared data (_done for signaling)
    std::condition_variable _condition_variable_todo; // CV for workers to wait for tasks or exit signal
    std::condition_variable _condition_variable_done; // CV for main thread to wait for job completion

    // Atomic variables
    std::atomic<bool> _exit{false};          // Flag to signal worker threads to terminate. THIS IS THE KEY FIX.
    std::atomic<int32_t> _todo{0};           // Number of tasks remaining to be picked up for the current job.
    std::atomic<int32_t> _done{0};           // Number of tasks remaining to be completed for the current job.
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