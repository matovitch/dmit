#include "dmit/src/slice.hpp"

#include <cstdint>

namespace dmit::src
{

uint32_t Slice::size() const
{
    return static_cast<uint32_t>(_tail - _head);
}

} //namespace dmit::src
