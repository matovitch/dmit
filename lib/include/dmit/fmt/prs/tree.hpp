#pragma once

#include "dmit/prs/tree.hpp"

#include <string>

namespace dmit
{

namespace fmt
{

std::string asString(const prs::state::tree::node::Kind);

std::string asString(const prs::state::tree::Node&);

std::string asString(const prs::state::Tree&);

} // namespace fmt
} // namespace dmit
