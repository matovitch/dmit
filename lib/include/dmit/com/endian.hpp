#pragma once

#include <cstdint>

namespace dmit::com
{

struct Endianness
{
    Endianness()
    {
        uint16_t i = 1;
        _asInt = (reinterpret_cast<uint8_t*>(&i)[0] == 0);
    }

    Endianness(uint8_t asInt) : _asInt{asInt} {}

    enum : uint8_t
    {
        LITTLE,
        BIG
    };

    uint8_t _asInt;
};

bool operator==(const Endianness lhs,
                const Endianness rhs)
{
    return lhs._asInt == rhs._asInt;
}

bool operator!=(const Endianness lhs,
                const Endianness rhs)
{
    return lhs._asInt != rhs._asInt;
}

} // namespace dmit::com
