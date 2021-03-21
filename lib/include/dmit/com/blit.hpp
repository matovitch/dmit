#pragma once

#include <cstring>

#include <utility>

namespace dmit::com
{

template <class TypeSrc,
          class TypeDst>
void blit(TypeSrc&& src, TypeDst& dst)
{
    new (&dst) TypeDst{std::forward<TypeSrc>(src)};
}

template <class TypeDst>
void blitDefault(TypeDst& dst)
{
    new (&dst) TypeDst{};
}

} // namespace dmit::com
