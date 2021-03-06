#include "dmit/src/slice.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>
#include <vector>

namespace dmit::src
{

Slice::Slice(const std::vector<uint8_t>& source,
             const std::vector<uint32_t>& lexOffsets,
             uint32_t lexIndex)
{
    DMIT_COM_ASSERT(lexIndex > 1);

    const auto head   = source.data() + lexOffsets[0];
    const auto offset = lexOffsets.data() + lexOffsets.size() - 1 - lexIndex;

    _head = head - *(offset - 1),
    _tail = head - *(offset - 0);
}

uint32_t Slice::size() const
{
    return static_cast<uint32_t>(_tail - _head);
}

} //namespace dmit::src
