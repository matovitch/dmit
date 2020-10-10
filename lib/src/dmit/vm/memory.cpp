#include "dmit/vm/memory.hpp"

#include <cstdint>
#include <cstring>

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

void Memory::copy(const uint8_t* const data, const uint64_t size, const uint64_t address)
{
    std::memcpy(_asBytes.data() + address, data, size);
}

} // namespace dmit::vm
