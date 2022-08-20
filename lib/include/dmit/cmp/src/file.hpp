#pragma once

#include "dmit/src/file.hpp"

extern "C"
{
    #include "cmp/cmp.h"
}

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const src::File&);

} // namespace dmit::cmp
