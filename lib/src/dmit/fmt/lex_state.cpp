#include "dmit/fmt/lex/state.hpp"

#include "dmit/fmt/formatable.hpp"
#include "dmit/fmt/lex/token.hpp"

#include "dmit/lex/state.hpp"

#include <sstream>
#include <string>

namespace dmit
{

namespace fmt
{

std::string asString(const lex::State& state)
{
    std::ostringstream oss;

    oss << "{\"tokens\" :" << DMIT_FMT_CONTAINER_AS_STRING(state._tokens)
        << ",\"offsets\":" << DMIT_FMT_CONTAINER_AS_STRING(state._offsets) << "}";

    return oss.str();
}

} // namespace fmt
} // namespace dmit
