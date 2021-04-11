#include "dmit/com/mutex.hpp"

#include <atomic>

namespace dmit::com
{

void Mutex::lock()
{
    while (flag.exchange(true, std::memory_order_relaxed));
    std::atomic_thread_fence(std::memory_order_acquire);
}

void Mutex::unlock()
{
    std::atomic_thread_fence(std::memory_order_release);
    flag.store(false, std::memory_order_relaxed);
}

} // namespace dmit::com
