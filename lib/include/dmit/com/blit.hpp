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

} // namespace dmit::com
