#pragma once

#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::drv
{

struct Query : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        CREATE_OR_UPDATE_FILE,
        STOP_SERVER,
        INVALID
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Query);
};

} // namespace dmit::drv