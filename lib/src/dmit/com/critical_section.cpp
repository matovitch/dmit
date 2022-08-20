#include "dmit/com/critical_section.hpp"

#include <optional>
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
    std::optional<std::chrono::time_point<std::chrono::steady_clock>> timePointOpt;

    while (_flag.exchange(true, std::memory_order_relaxed))
    {
        if ((0xff & (++i)) != 0)
        {
            continue;
        }

        const auto now = std::chrono::steady_clock::now();

        if (!timePointOpt)
        {
            timePointOpt = now + K_DELAY;
        }

        if (now > timePointOpt.value())
        {
            goto SLEEP_LOOP;
        }
    }

    goto FENCE;

    SLEEP_LOOP:
    {
        while (_flag.exchange(true, std::memory_order_relaxed))
        {
            std::this_thread::sleep_for(K_DELAY);
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
    #if defined(SCHMIT_USE_TSAN)
        __tsan_release(&_flag);
    #endif

    std::atomic_thread_fence(std::memory_order_release);
    _flag.store(false, std::memory_order_relaxed);
}

} // namespace dmit::com
