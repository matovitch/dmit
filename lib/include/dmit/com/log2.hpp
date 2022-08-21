#pragma once

#include <cstdint>

namespace dmit::com
{

constexpr uint32_t log2(uint32_t val)
{
    return val ? 1 + log2(val >> 1) : 0;
}

} // namespace dmit::com
