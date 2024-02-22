#include "dmit/com/critical_section.hpp"

#include <atomic>
#include <chrono>
#include <thread>

#if defined(SCHMIT_USE_TSAN)
    #include "sanitizer/tsan_interface.h"
#endif

namespace dmit::com
{

static constexpr auto K_DELAY = 100us;

CriticalSection::CriticalSection(std::atomic<bool>& flag) : _flag{flag}
{
    uint32_t i = 0;
    std::chrono::time_point<std::chrono::steady_clock>* timePointPtr = nullptr;

    while (_flag.exchange(true, std::memory_order_relaxed))
    {
        if ((0xff & (++i)) != 0)
        {
            continue;
        }

        auto now = std::chrono::steady_clock::now();

        if (!timePointPtr)
        {
            now += K_DELAY;
            timePointPtr = &now;
        }
        else if (now > *timePointPtr)
        {
            goto YIELD_LOOP;
        }
    }

    goto FENCE;

    YIELD_LOOP:
    {
        while (_flag.exchange(true, std::memory_order_relaxed))
        {
            std::this_thread::yield();
        }
    }

    FENCE:
    {
        std::atomic_thread_fence(std::memory_order_acquire);

        #if defined(SCHMIT_USE_TSAN)
            __tsan_acquire(&_flag);
        #endif
    }
}

CriticalSection::~CriticalSection()
{
    _flag.store(false, std::memory_order_release);
}

} // namespace dmit::com
