#include "dmit/fmt/sem/interface_map.hpp"

#include "dmit/fmt/ast_visitor.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const sem::InterfaceMap& interfaceMap)
{
    std::ostringstream oss;

    oss << '[';

    AstVisitor astVisitor{const_cast<sem::InterfaceMap&>(interfaceMap), oss};

    for (const auto& idViewPair : interfaceMap._asSimpleMap)
    {
        astVisitor.base()(idViewPair.second);
        oss << ',';
    }

    oss.seekp(interfaceMap._asSimpleMap.empty() ? 0 : -1, std::ios_base::end);

    oss << ']';

    return oss.str();
}

} // namespace dmit::fmt
