#pragma once

#include <string>

namespace dmit
{

namespace com
{

namespace logger
{

class Sub;

} // namespace logger
} // namespace com

namespace fmt
{

std::string asString(const com::logger::Sub&);

} // namespace fmt
} // namespace dmit
