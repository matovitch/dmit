#pragma once

#include "dmit/drv/reply.hpp"

extern "C"
{
    #include "cmp/cmp.h"
}

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const drv::Reply);

} // namespace dmit::cmp
