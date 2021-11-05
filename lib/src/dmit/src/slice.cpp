#include "dmit/src/slice.hpp"

#include "dmit/com/unique_id.hpp"

#include <cstdint>

namespace dmit::src
{

uint32_t Slice::size() const
{
    return static_cast<uint32_t>(_tail - _head);
}

com::UniqueId Slice::makeUniqueId() const
{
    return com::UniqueId{_head, size()};
}

} //namespace dmit::src
