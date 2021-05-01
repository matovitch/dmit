#pragma once

#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::drv
{

struct Query : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        ADD_FILE,
        STOP,
        DATABASE_GET,
        DATABASE_CLEAN,
        MAKE,
        INVALID
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Query);
};

} // namespace dmit::drv
