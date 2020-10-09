#pragma once

#include <cstdint>

namespace dmit::src
{

struct Slice
{
    uint32_t size() const;

    const uint8_t* const _head;
    const uint8_t* const _tail;
};

} // namespace dmit::src
