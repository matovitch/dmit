#include "dmit/fmt/prs/reader.hpp"

#include "dmit/fmt/lex/token.hpp"

#include <sstream>
#include <string>

namespace dmit::fmt
{

std::string asString(const lex::Reader& reader)
{
    std::ostringstream oss;

    oss << "{ \"token\":\"" << reader.look() << ",\"size\":" << reader.offset() << "}";

    return oss.str();
}

} // namespace dmit::fmt
