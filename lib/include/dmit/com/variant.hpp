#pragma once

#include <variant>

namespace dmit
{

namespace com
{

namespace variant
{

template <class... Ts>
struct TFlatMerger;

template <class... Ts, class... Us>
struct TFlatMerger<std::variant<Ts...>, std::variant<Us...>>
{
    using Type = std::variant<Ts..., Us...>;
};

template <class... Ts, class...  Us>
struct TFlatMerger<std::variant<Ts...>, Us...>
{
    using Type = std::variant<Ts..., Us...>;
};

template <class... Ts>
using TFlatMerge = typename TFlatMerger<Ts...>::Type;

} // namespace variant
} // namespace com
} // namespace dmit
