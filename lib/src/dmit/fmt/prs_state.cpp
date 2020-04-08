#include "dmit/fmt/prs/state.hpp"

#include "dmit/fmt/prs/error.hpp"
#include "dmit/fmt/prs/tree.hpp"

namespace dmit::fmt
{

std::string asString(const prs::State& state)
{
    std::ostringstream oss;

    oss << "{\"tree\" :"  << state._tree
        << ",\"errors\":" << state._errors << "}";

    return oss.str();

}

} // namespace dmit::fmt
