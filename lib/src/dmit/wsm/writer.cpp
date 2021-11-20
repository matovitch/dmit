#include "dmit/wsm/writer.hpp"

#include "dmit/wsm/leb128.hpp"

#include "dmit/com/endian.hpp"

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

Scribe::Scribe() : _data{nullptr} {}

Scribe::Scribe(uint8_t* const data) : _data(data) {}

void Scribe::skip(const uint32_t toSkip)
{
    _data += toSkip;
}

void Scribe::write(const uint8_t *const data, uint32_t size)
{
    std::memcpy(static_cast<      void*>(_data),
                static_cast<const void*>( data), size);

    _data += size;
}

void Scribe::write(const uint8_t byte)
{
    *_data++ = byte;
}

void Scribe::write(const Leb128& leb128)
{
    write(leb128._asBytes, leb128._size);
}

uint32_t Scribe::diff(const Scribe scribe) const
{
    return _data - scribe._data;
}

void Scribe::writeF32(const flt32_t value)
{
    const uint8_t* const asBytes = reinterpret_cast<const uint8_t*>(&value);

    if (com::Endianness{} == com::Endianness::LITTLE)
    {
        for (uint32_t i = 0; i < sizeof(flt32_t); i++)
        {
            write(asBytes[i]);
        }
    }
    else if (com::Endianness{} == com::Endianness::BIG)
    {
        for (uint32_t i = sizeof(flt32_t); i > 0; i--)
        {
            write(asBytes[i - 1]);
        }
    }
}

void Scribe::writeF64(const flt64_t value)
{
    const uint8_t* const asBytes = reinterpret_cast<const uint8_t*>(&value);

    if (com::Endianness{} == com::Endianness::LITTLE)
    {
        for (uint32_t i = 0; i < sizeof(flt64_t); i++)
        {
            write(asBytes[i]);
        }
    }
    else if (com::Endianness{} == com::Endianness::BIG)
    {
        for (uint32_t i = sizeof(flt64_t); i > 0; i--)
        {
            write(asBytes[i - 1]);
        }
    }
}

Scribe Scribe::fork() const
{
    return *this;
}

} // namespace dmit::wsm/writer