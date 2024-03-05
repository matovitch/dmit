#pragma once

#include "dmit/ast/state.hpp"

#include "dmit/ast/source_register.hpp"

#include "dmit/cmp/cmp.hpp"

#include <optional>

namespace dmit::cmp
{

bool write(cmp_ctx_t*, const ast::State&);

std::optional<ast::State> readAstState(cmp_ctx_t*, ast::SourceRegister&);

} // namespace dmit::cmp
