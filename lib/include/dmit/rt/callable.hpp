#pragma once

#include <cstdint>

namespace dmit::rt
{

struct Callable
{
    virtual void operator()(const uint8_t* const) = 0;
};

} // namespace dmit::rt
