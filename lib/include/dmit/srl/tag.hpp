#pragma once

#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::srl
{

struct Tag : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        FILE,
        INVALID
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Tag);
};

} // namespace dmit::srl
