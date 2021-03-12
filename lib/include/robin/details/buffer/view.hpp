#pragma once

#include <cstdint>

namespace robin_details::buffer
{

struct View
{
    uint8_t* const    data;
    const std::size_t size;
};

} // namespace robin_details::buffer
