#include "dmit/sql/canary_buffer.hpp"

#include <cstdint>

namespace dmit::sql
{

CanaryBuffer::CanaryBuffer(const int32_t size) :
    _size{((size >> 4) << 4) + 0x10},
    _data{new uint8_t[_size]}
{
    reset(_size);
}

void CanaryBuffer::reset(const int32_t size)
{
    for (int32_t i = 0; i < size; i+= 0x10)
    {
        _data[i + 0x0] = 0xe7;
        _data[i + 0x1] = 0x1f;
        _data[i + 0x2] = 0xbf;
        _data[i + 0x3] = 0xa8;
        _data[i + 0x4] = 0x73;
        _data[i + 0x5] = 0x3d;
        _data[i + 0x6] = 0x42;
        _data[i + 0x7] = 0x7a;
        _data[i + 0x8] = 0xb3;
        _data[i + 0x9] = 0x26;
        _data[i + 0xa] = 0x06;
        _data[i + 0xb] = 0xc0;
        _data[i + 0xc] = 0x58;
        _data[i + 0xd] = 0xd1;
        _data[i + 0xe] = 0x96;
        _data[i + 0xf] = 0xaa;
    }
}

int32_t CanaryBuffer::size() const
{
    int32_t i = _size - 0x10;

    while (i >= 0)
    {
        if ( _data[i + 0x0] != 0xe7 ||
             _data[i + 0x1] != 0x1f ||
             _data[i + 0x2] != 0xbf ||
             _data[i + 0x3] != 0xa8 ||
             _data[i + 0x4] != 0x73 ||
             _data[i + 0x5] != 0x3d ||
             _data[i + 0x6] != 0x42 ||
             _data[i + 0x7] != 0x7a ||
             _data[i + 0x8] != 0xb3 ||
             _data[i + 0x9] != 0x26 ||
             _data[i + 0xa] != 0x06 ||
             _data[i + 0xb] != 0xc0 ||
             _data[i + 0xc] != 0x58 ||
             _data[i + 0xd] != 0xd1 ||
             _data[i + 0xe] != 0x96 ||
             _data[i + 0xf] != 0xaa )
        {
            break;
        }

        i -= 0x10;
    }

    i += 0x10;

    return i;
}

CanaryBuffer::~CanaryBuffer()
{
    delete[] _data;
}

} // namespace dmit::sql
