#include "dmit/fmt/src/line_index.hpp"

#include "dmit/src/line_index.hpp"

#include <sstream>
#include <string>

namespace dmit
{

namespace fmt
{

std::string asString(const src::LineIndex& lineIndex)
{
    std::ostringstream oss;

    oss << "{\"offsets\":" << DMIT_FMT_CONTAINER_AS_STRING(lineIndex.offsets()) << "}";

    return oss.str();
}

} // namespace fmt

} // namespace dmit
