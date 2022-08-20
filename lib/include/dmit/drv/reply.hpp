#pragma once

#include "dmit/com/enum.hpp"

extern "C"
{
    #include "cmp/cmp.h"
}

#include <cstdint>

namespace dmit::drv
{

struct Reply : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        OK,
        KO
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Reply);
};

} // namespace dmit::drv
