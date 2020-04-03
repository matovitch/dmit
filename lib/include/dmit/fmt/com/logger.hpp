#pragma once

#include <string>

namespace dmit
{

namespace com::logger
{

class Sub;

} // namespace com::logger

namespace fmt
{

std::string asString(const com::logger::Sub&);

} // namespace fmt
} // namespace dmit
