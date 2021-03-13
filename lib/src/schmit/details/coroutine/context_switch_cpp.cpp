#include "schmit/details/coroutine/context_switch.hpp"
#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine.hpp"

#include <cstdint>

namespace schmit_details
{

namespace coroutine
{

void* contextSwitch(schmit_details::coroutine::Abstract& srce,
                    schmit_details::coroutine::Abstract& dest)
{
    return schmitDetailsCoroutineContextSwitch(&srce, &dest);
}

void mmxFpuSave(schmit_details::coroutine::Register& mmxFpuState)
{
    schmitDetailsCoroutineMmxFpuSave(&mmxFpuState);
}

} // namespace coroutine

} // namespace schmit_details
