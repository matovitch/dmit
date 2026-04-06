#pragma once

#include "schmit/details/coroutine/register.hpp"
#include "schmit/details/coroutine.hpp"

extern "C" void* schmitDetailsCoroutineContextSwitch(schmit_details::coroutine::State* srce,
                                                     schmit_details::coroutine::State* dest);

extern "C" void schmitDetailsCoroutineMmxFpuSave(schmit_details::coroutine::Register*);

namespace schmit_details
{

namespace coroutine
{

void contextSwitch(Coroutine& srce,
                   Coroutine& dest);

void mmxFpuSave(coroutine::Register&);

} // namespace coroutine
} // namespace schmit_details
