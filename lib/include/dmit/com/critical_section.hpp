#pragma once

#include <atomic>
#include <chrono>

namespace dmit::com
{

using namespace std::chrono_literals;

class CriticalSection
{

public:

    CriticalSection(std::atomic<bool>& flag);

    ~CriticalSection();

private:

    std::atomic<bool>& _flag;

    static constexpr auto K_DELAY = 100us;
};

} // namespace dmit::com

#define DMIT_COM_CRITICAL_SECTION(flag) dmit::com::CriticalSection criticalSection{flag}
