#pragma once

#include "dmit/fmt/formatable.hpp"

#include <cstdint>

namespace dmit::src
{

struct Slice : fmt::Formatable
{
    uint32_t size() const;

    const uint8_t* _head = nullptr;
    const uint8_t* _tail = nullptr;
};

} // namespace dmit::src
