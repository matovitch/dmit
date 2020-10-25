#pragma once

#include "dmit/src/file.hpp"

#include "cmp/cmp.h"

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const src::File&);

} // namespace dmit::cmp
