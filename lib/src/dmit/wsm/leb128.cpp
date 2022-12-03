#include "dmit/wsm/leb128.hpp"
#include <cstdint>
#include <new>

// Taken from https://llvm.org/doxygen/LEB128_8h_source.html

namespace dmit::wsm
{

void Leb128::push(const uint8_t byte)
{
    _asBytes[_size++] = byte;
}

Leb128::Leb128(int64_t asI64)
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

        push(byte);
    }
    while (more);
}

Leb128::Leb128(uint64_t asU64)
{
    do
    {
        uint8_t byte = asU64 & 0x7f;

        asU64 >>= 7;

        if (asU64 != 0)
        {
            byte |= 0x80; // Mark this byte to show that more bytes will follow.
        }

        push(byte);
    }
    while (asU64 != 0);
}

Leb128::Leb128(uint32_t asU32) : Leb128{static_cast<uint64_t>(asU32)} {}

Leb128Obj::Leb128Obj(uint64_t asU64)
{
    new (reinterpret_cast<Leb128*>(this)) Leb128{asU64};
}

Leb128Obj::Leb128Obj(int64_t asI64)
{
    new (reinterpret_cast<Leb128*>(this)) Leb128{asI64};
}

} // namespace dmit::wsm
