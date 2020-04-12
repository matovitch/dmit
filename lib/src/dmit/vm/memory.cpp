#include "dmit/vm/memory.hpp"

#include <cstdint>

namespace dmit::vm
{

void Memory::grow(uint64_t size)
{
    _asBytes.resize(this->size() + size);
}

uint64_t Memory::size() const
{
    return _asBytes.size();
}

} // namesapce dmit::vm
