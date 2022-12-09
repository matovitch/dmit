#pragma once

#include <cstdint>

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

static const uint32_t K_LEB128_MAX_SIZE = 10;
static const uint32_t K_LEB128_OBJ_SIZE =  5;

template <bool IS_OBJECT = false>
struct Leb128;

template <>
struct Leb128</*IS_OBJECT=*/false>
{
    Leb128(uint64_t asU64) { init<K_LEB128_MAX_SIZE>(asU64, _asBytes, _size); }
    Leb128( int64_t asI64) { init<K_LEB128_MAX_SIZE>(asI64, _asBytes, _size); }
    Leb128(uint32_t asU32) : Leb128{static_cast<uint64_t>(asU32)} {}
    Leb128( int32_t asI32) : Leb128{static_cast<uint64_t>(asI32)} {}

    uint8_t _asBytes[K_LEB128_MAX_SIZE] = {0};
    uint8_t _size = 0;
};

template <>
struct Leb128</*IS_OBJECT=*/true>
{
    Leb128(uint64_t asU64) { init<K_LEB128_OBJ_SIZE>(asU64, _asBytes, _size); }
    Leb128( int64_t asI64) { init<K_LEB128_OBJ_SIZE>(asI64, _asBytes, _size); }
    Leb128(uint32_t asU32) : Leb128{static_cast<uint64_t>(asU32)} {}
    Leb128( int32_t asI32) : Leb128{static_cast<uint64_t>(asI32)} {}

    uint8_t _asBytes[K_LEB128_OBJ_SIZE] = {0x80, 0x80, 0x80, 0x80, 0x00};
    uint8_t _size = 0;
};

} // namespace dmit::wsm
