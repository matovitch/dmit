#pragma once

#include "dmit/ast/bundle.hpp"

namespace dmit::sem
{

struct InterfaceMap
{
    void registerBundle(ast::Bundle& bundle);
};

} // namespace dmit::sem
