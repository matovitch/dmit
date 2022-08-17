#pragma once

#include <cstdint>

namespace dmit::com
{

struct Endianness
{
    Endianness();

    Endianness(uint8_t asInt);

    enum : uint8_t
    {
        LITTLE,
        BIG
    };

    uint8_t _asInt;
};

bool operator==(const Endianness,
                const Endianness);

bool operator!=(const Endianness,
                const Endianness);

} // namespace dmit::com
