#include "dmit/src/partition.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>
#include <vector>

namespace dmit::src
{

Partition::Partition(const uint8_t* const source,
                     const std::vector<uint32_t>& offsets) :
    _source{source},
    _offsets{offsets}
{}

Slice Partition::getSlice(const uint32_t index) const
{
    DMIT_COM_ASSERT(index > 1);

    const auto head   = _source + _offsets[0];
    const auto offset = _offsets.data() + _offsets.size() - 1 - index;

    return Slice{head - *(offset - 1),
                 head - *(offset - 0)};
}

} // namespace dmit::src