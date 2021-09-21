#pragma once

#include <utility>

namespace robin_details
{

namespace pair
{

struct NoValue {};

} // namespace pair

template <class Key,
          class Value>
struct TPair
{
    Key first;

    union
    {
        uint8_t dummy;
        Value second;
    };

    template <class Pair>
    TPair(Pair&& pair) : first  {std::move(pair.first)},
                         second {std::move(pair.second)}
    {}

    template <class ImplicitKey>
    TPair(pair::NoValue, ImplicitKey&& key) : first{key}, dummy{} {}

    template <class ImplicitKey, class ImplicitValue>
    TPair(ImplicitKey   && key,
          ImplicitValue && value) :
        first{std::forward<ImplicitKey>(key)},
        second{std::forward<ImplicitValue>(value)}
    {}

    template< class... Args1, class... Args2 >
    TPair(std::piecewise_construct_t,
          std::tuple<Args1...> firstArgs,
          std::tuple<Args2...> secondArgs) :
        first{std::forward<Args1>(firstArgs)...},
        second{std::forward<Args2>(secondArgs)...}
    {}
};

namespace pair
{

template <class Traits>
struct THasher
{
    using Pair      = typename Traits::Pair;
    using KeyHasher = typename Traits::KeyHasher;

    THasher() : _keyHasher{} {}

    std::size_t operator()(const Pair& pair) const
    {
        return _keyHasher(pair.first);
    }

    const KeyHasher _keyHasher;
};

namespace hasher
{

template <class Key,
          class Value,
          class Hasher>
struct TTraits
{
    using Pair      = TPair<Key, Value>;
    using KeyHasher = Hasher;
};

template <class Key,
          class Value,
          class Hasher>
using TMake = THasher<TTraits<Key, Value, Hasher>>;

} // namespace hasher

template <class Traits>
struct TComparator
{
    using Key        = typename Traits::Key;
    using Comparator = typename Traits::Comparator;

    TComparator() : _comparator{} {}

    template <class Value>
    bool operator()(const TPair<Key, Value>& lhs,
                    const TPair<Key, Value>& rhs) const
    {
        return _comparator(lhs.first,
                          rhs.first);
    }

    const Comparator _comparator;
};

namespace comparator
{

template <class TraitsKey,
          class TraitsComparator>
struct TTraits
{
    using Key        = TraitsKey;
    using Comparator = TraitsComparator;
};

template <class Key,
          class Comparator>
using TMake = TComparator<TTraits<Key, Comparator>>;

} // namespace comparator

} // namespace pair

} // namespace robin_details
