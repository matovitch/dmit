#pragma once

#include "dmit/prs/reader.hpp"
#include "dmit/prs/stack.hpp"

namespace dmit
{

namespace prs
{

struct State;

namespace open
{

template <class...>
struct TPipeline;

template <>
struct TPipeline<>
{
    void operator()(const Reader&, Stack&, State&) const {}
};

template <class Open, class... Opens>
struct TPipeline<Open, Opens...>
{
    void operator()(const Reader& reader, Stack& stack, State& state) const
    {
        Open{}(reader, stack, state);

        return TPipeline<Opens...>{}(reader, stack, state);
    }
};

} // namespace open

namespace close
{

template <class...>
struct TPipeline;

template <>
struct TPipeline<>
{
    void operator()(const std::optional<Reader>&, const Stack&, State&) const {}
};

template <class Close, class... Closes>
struct TPipeline<Close, Closes...>
{
    void operator()(const std::optional<Reader>& readerOpt, const Stack& stack, State& state) const
    {
        Close{}(readerOpt, stack, state);

        return TPipeline<Closes...>{}(readerOpt, stack, state);
    }
};

} // namespace close
} // namespace prs
} // namespace dmit