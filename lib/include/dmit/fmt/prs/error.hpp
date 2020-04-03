#pragma once

#include "dmit/prs/error.hpp"

#include <string>

namespace dmit::fmt
{

std::string asString(const prs::state::Error&);

std::string asString(const prs::state::error::Set&);

} // namespace dmit::fmt
