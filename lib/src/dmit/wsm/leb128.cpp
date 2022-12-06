#include "dmit/wsm/leb128.hpp"
#include <cstdint>
#include <new>

// Taken from https://llvm.org/doxygen/LEB128_8h_source.html

namespace dmit::wsm
{

namespace
{

template <uint32_t SIZE>
void init(int64_t asI64, uint8_t bytes[SIZE], uint8_t& size)
{
    bool more;

    do
    {
        uint8_t byte = asI64 & 0x7f;

        asI64 >>= 7; // NOTE: this assumes that this signed shift is an arithmetic right shift.

        more = !(((asI64 ==  0) && ((byte & 0x40) == 0)) ||
                 ((asI64 == -1) && ((byte & 0x40) != 0)));

        if (more)
        {
            byte |= 0x80; // Mark this byte to show that more bytes will follow.
        }

        bytes[size++] |= byte;
    }
    while (more);
}

template <uint32_t SIZE>
void init(uint64_t asU64, uint8_t bytes[SIZE], uint8_t& size)
{
    do
    {
        uint8_t byte = asU64 & 0x7f;

        asU64 >>= 7;

        if (asU64 != 0)
        {
            byte |= 0x80; // Mark this byte to show that more bytes will follow.
        }

        bytes[size++] |= byte;
    }
    while (asU64 != 0);
}

} // namespace

Leb128Obj::Leb128Obj(uint64_t asU64)
{
    init<K_LEB128_OBJ_SIZE>(asU64, _asBytes, _size);
}

Leb128Obj::Leb128Obj(int64_t asI64)
{
    init<K_LEB128_OBJ_SIZE>(asI64, _asBytes, _size);
}

Leb128::Leb128(uint64_t asU64) { init<K_LEB128_MAX_SIZE>(asU64, _asBytes, _size); }
Leb128::Leb128( int64_t asI64) { init<K_LEB128_MAX_SIZE>(asI64, _asBytes, _size); }

Leb128::Leb128(uint32_t asU32) : Leb128{static_cast<uint64_t>(asU32)} {}
Leb128::Leb128( int32_t asI32) : Leb128{static_cast< int64_t>(asI32)} {}
Leb128Obj::Leb128Obj(uint32_t asU32) : Leb128Obj{static_cast<uint64_t>(asU32)} {}
Leb128Obj::Leb128Obj( int32_t asI32) : Leb128Obj{static_cast< int64_t>(asI32)} {}

} // namespace dmit::wsm
