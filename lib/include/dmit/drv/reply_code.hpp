#pragma once

#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::drv
{

struct ReplyCode : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        OK,
        KO
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(ReplyCode);
};

} // namespace dmit::drv
