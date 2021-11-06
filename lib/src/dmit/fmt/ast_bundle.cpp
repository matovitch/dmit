#include "dmit/fmt/ast/state.hpp"

#include "dmit/fmt/ast_visitor.hpp"

#include "dmit/ast/bundle.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const ast::Bundle& bundle)
{
    std::ostringstream oss;

    AstVisitor astVisitor{bundle, oss};

    auto bundleViews = bundle._views; // copy to avoid const

    astVisitor.base()(bundleViews);

    return oss.str();
}

} // namespace dmit::fmt