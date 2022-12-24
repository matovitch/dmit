#include "dmit/src/slice.hpp"

#include "dmit/com/unique_id.hpp"

#include <string_view>
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

std::string_view Slice::makeStringView() const
{
    return std::string_view{reinterpret_cast<const char*>(_head),
                            size()};
}

} //namespace dmit::src
