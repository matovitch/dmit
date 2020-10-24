#pragma once

#include "dmit/src/file.hpp"

#include "cmp/cmp.h"

namespace dmit::srl
{

bool serialize(const src::File&, cmp_ctx_t*);

} // namespace dmit::srl
