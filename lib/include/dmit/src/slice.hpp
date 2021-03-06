#pragma once

#include <cstdint>
#include <vector>

namespace dmit::src
{

struct Slice
{
    Slice(const std::vector<uint8_t>& source,
          const std::vector<uint32_t>& lexOffsets,
          uint32_t lexIndex);

    uint32_t size() const;

    const uint8_t* _head = nullptr;
    const uint8_t* _tail = nullptr;
};

} // namespace dmit::src
