#pragma once

#include "dmit/fmt/formatable.hpp"

#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::ast
{

struct FunctionStatus : com::TEnum<uint8_t>, fmt::Formatable
{
    enum : uint8_t
    {
        EXPORTED,
        LOCAL
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(FunctionStatus);
};

} // namespace dmit::ast
