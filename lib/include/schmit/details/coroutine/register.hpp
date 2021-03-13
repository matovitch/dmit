#pragma once

#include <cstdint>

namespace schmit_details
{

namespace coroutine
{

using Register = uint64_t;

namespace RegisterMap
{

enum
{
    R12,
    R13,
    R14,
    R15,
    RDX_RETURN_ADDRESS,
    RCX_STACK_POINTER,
    RBX,
    RBP,
    MMX_FPU_STATE,
    SIZE
};

} // namespace RegisterMap
} // namespace coroutine
} // namespace schmit_details
