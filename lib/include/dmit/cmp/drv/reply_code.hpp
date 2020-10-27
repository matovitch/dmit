#pragma once

#include "dmit/drv/reply_code.hpp"

#include "cmp/cmp.h"

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const drv::ReplyCode);

} // namespace dmit::cmp
