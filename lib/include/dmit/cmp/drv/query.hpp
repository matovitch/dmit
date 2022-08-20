#pragma once

#include "dmit/drv/query.hpp"

extern "C"
{
    #include "cmp/cmp.h"
}

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const drv::Query);

} // namespace dmit::cmp
