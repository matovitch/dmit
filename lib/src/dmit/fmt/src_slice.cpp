#include "dmit/fmt/src/slice.hpp"

#include "dmit/src/slice.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const src::Slice slice)
{
    std::ostringstream oss;

    oss << '"';

    for (int i = 0; i < slice.size(); i++)
    {
        oss << slice._head[i];
    }

    oss << '"';

    return oss.str();
}


} // namespace dmit::fmt
