#include "dmit/fmt/ast/state.hpp"

#include "dmit/fmt/ast_visitor.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const ast::State& state)
{
    std::ostringstream oss;

    AstVisitor astVisitor{state._nodePool, oss};

    astVisitor(state._module);

    return oss.str();
}

} // namespace dmit::fmt
