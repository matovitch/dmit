#include "dmit/fmt/src/location.hpp"

#include "dmit/src/location.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const src::Location location)
{
    std::ostringstream oss;

    oss << "{\"line\":"   << location.line()
        << ",\"column\":" << location.column() << "}";

    return oss.str();
}

} // namespace dmit::fmt
