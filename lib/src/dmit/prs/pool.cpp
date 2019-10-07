#include "dmit/prs/pool.hpp"

#include "dmit/prs/subscriber.hpp"
#include "dmit/prs/parser.hpp"
#include "dmit/prs/state.hpp"

#include <optional>
#include <vector>

namespace dmit
{

namespace prs
{

namespace pool
{

prs::Parser Parser::make(State& state)
{
    _parsers.emplace_back(new std::optional<prs::Parser::Fn>);
    return prs::Parser{*(_parsers.back()), state};
}

} // namespace pool

} // namespace prs

} // namespace dmit
