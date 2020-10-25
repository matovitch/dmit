#pragma once

#include "dmit/drv/action.hpp"

#include "cmp/cmp.h"

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const drv::Action);

} // namespace dmit::cmp
