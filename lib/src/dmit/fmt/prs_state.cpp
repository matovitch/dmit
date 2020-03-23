#include "dmit/fmt/prs/state.hpp"

#include "dmit/fmt/prs/error.hpp"
#include "dmit/fmt/prs/tree.hpp"

namespace dmit
{

namespace fmt
{

std::string asString(const prs::State& state)
{
    std::ostringstream oss;

    oss << "{\"tree\" :"    << state._tree
        << ",\"errorSet\":" << state._errorSet << "}";

    return oss.str();

}

} // namespace fmt
} // namespace dmit
