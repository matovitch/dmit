#pragma once

#include "dmit/lex/token.hpp"

#include "dmit/cmp/cmp.hpp"

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const lex::Token);

} // namespace dmit::cmp
