#include "dmit/sem/context.hpp"

#include "dmit/ast/state.hpp"

namespace dmit::sem
{

Context::Context(ast::State& ast) : _ast{ast} {}

} // namespace dmit::sem
