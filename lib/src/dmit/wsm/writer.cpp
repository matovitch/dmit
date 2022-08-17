#include "dmit/wsm/writer.hpp"

#include "dmit/wsm/leb128.hpp"

#include <cstdint>
#include <cstring>

namespace dmit::wsm::writer
{

void Bematist::skip(const uint32_t) {}

void Bematist::write(const uint8_t *const data, uint32_t size)
{
    _size += size;
}

void Bematist::write(const uint8_t)
{
    _size++;
}

void Bematist::write(const Leb128& leb128)
{
    _size += leb128._size;
}

void Bematist::writeF32(const flt32_t) { _size += sizeof(flt32_t); }
void Bematist::writeF64(const flt64_t) { _size += sizeof(flt64_t); }


uint32_t Bematist::diff(const Bematist) const
{
    return 0;
}

Bematist Bematist::fork() const
{
    return *this;
}

} // namespace dmit::wsm/writer
