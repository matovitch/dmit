#pragma once

#include <type_traits>

namespace dmit
{

namespace com
{

namespace type_flag
{

template <class TypeFlag, class Type>
using THas = std::enable_if_t<std::is_base_of_v <TypeFlag, std::decay_t<Type>>, int>;

template <class TypeFlag, class Type>
using THasNot = std::enable_if_t<std::negation_v<std::is_base_of<TypeFlag, std::decay_t<Type>>>, int>;

} // namespace type_flag

} // namespace com

} // namespace dmit

#define DMIT_COM_TYPE_FLAG_CHECK_IS(TypeFlag, Type)     dmit::com::type_flag::THas    <TypeFlag, Type> = 1
#define DMIT_COM_TYPE_FLAG_CHECK_IS_NOT(TypeFlag, Type) dmit::com::type_flag::THasNot <TypeFlag, Type> = 1
