#pragma once

#include <cstdint>

namespace dmit::wsm
{

static const uint32_t K_LEB128_MAX_SIZE = 10;
static const uint32_t K_LEB128_OBJ_SIZE =  5;

struct Leb128
{
    Leb128(uint32_t);
    Leb128(uint64_t);
    Leb128( int64_t);
    Leb128( int32_t);

    uint8_t _asBytes[K_LEB128_MAX_SIZE] = {0};
    uint8_t _size = 0;
};

struct Leb128Obj
{
    Leb128Obj(uint32_t);
    Leb128Obj(uint64_t);
    Leb128Obj( int64_t);
    Leb128Obj( int32_t);

    uint8_t _asBytes[K_LEB128_OBJ_SIZE] = {0x80, 0x80, 0x80, 0x80, 0x00};
    uint8_t _size = 0;
};

} // namespace dmit::wsm
