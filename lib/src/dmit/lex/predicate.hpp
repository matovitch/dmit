#pragma once

#include <cstdint>

namespace dmit
{

namespace lex
{

template <uint8_t low,
          uint8_t high>
struct TIsBetween
{
    bool operator()(const uint8_t byte) const
    {
        return byte >= low  &&
               byte <= high;
    }
};

template <uint8_t... bytes>
struct TIsIn
{
    bool operator()(const uint8_t byte) const
    {
        const auto compare = [byte](const uint8_t test) { return byte == test; };

        return (compare(bytes) || ...);
    }
};

template <uint8_t byte>
using TIs = TIsIn<byte>;

template <class... Predicates>
struct TOr
{
    bool operator()(const uint8_t byte) const
    {
        const auto compare = [byte](const auto predicate) { return predicate(byte); };

        return (compare(Predicates{}) || ...);
    }
};

} // namespace lex

} // namespace dmit
