#pragma once

#include "dmit/src/slice.hpp"

#include <cstdint>
#include <vector>

namespace dmit::src
{

class Partition
{

public:

    Partition(const uint8_t* const source,
              const std::vector<uint32_t>& offsets);

    Slice getSlice(const uint32_t index) const;

private:

    const uint8_t* const _source;
    const std::vector<uint32_t>& _offsets;
};

} // namespace dmit::src
