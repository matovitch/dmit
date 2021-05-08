#pragma once

#include "dmit/com/critical_section.hpp"

#include <atomic>

namespace dmit::com
{

struct ConcurentU64
{
    explicit ConcurentU64(uint64_t value) : _value{value} {}

    uint64_t _value;
};

class ConcurrentCounter
{

public:

    ConcurrentCounter() : _flag{false}, _value{0} {}

    ConcurentU64 increment()
    {
        DMIT_COM_CRITICAL_SECTION(_flag);
        return ConcurentU64{(_value._value)++};
    }

private:

    std::atomic<bool> _flag;
    ConcurentU64      _value;
};


} // namespace dmit::com
