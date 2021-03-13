#pragma once

#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine.hpp"

extern "C" void* schmitDetailsCoroutineContextSwitch(schmit_details::coroutine::Abstract* srce,
                                                     schmit_details::coroutine::Abstract* dest);

extern "C" void schmitDetailsCoroutineMmxFpuSave(schmit_details::coroutine::Register*);

namespace schmit_details
{

namespace coroutine
{

void* contextSwitch(coroutine::Abstract& srce,
                    coroutine::Abstract& dest);

void mmxFpuSave(coroutine::Register&);

} // namespace coroutine
} // namespace schmit_details
