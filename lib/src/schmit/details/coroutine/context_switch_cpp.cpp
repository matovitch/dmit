#include "schmit/details/coroutine/context_switch.hpp"
#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine.hpp"

namespace schmit_details
{

namespace coroutine
{

void contextSwitch(schmit_details::Coroutine& srce,
                   schmit_details::Coroutine& dest)
{
    #if defined(USE_ASAN)
        dest.asanStart();
    #endif

    schmitDetailsCoroutineContextSwitch(&srce._state, &dest._state);

    #if defined(USE_ASAN)
        srce.asanFinish();
    #endif
}

void mmxFpuSave(schmit_details::coroutine::Register& mmxFpuState)
{
    schmitDetailsCoroutineMmxFpuSave(&mmxFpuState);
}

} // namespace coroutine

} // namespace schmit_details
