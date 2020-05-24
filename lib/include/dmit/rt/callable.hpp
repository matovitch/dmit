#pragma once

#include <cstdint>

namespace dmit::rt
{

struct Callable
{
    virtual void call(const uint8_t* const) = 0;

    virtual ~Callable() {}
};

} // namespace dmit::rt
