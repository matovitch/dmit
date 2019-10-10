#pragma once

#include "dmit/com/type_flag.hpp"

namespace dmit
{

namespace com
{

struct EnumType{};

template <class EnumIntegerType>
struct TEnum : EnumType
{
    using IntegerType = EnumIntegerType;

    TEnum(IntegerType asInt) : _asInt{asInt} {}

    IntegerType _asInt;
};

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(EnumType, Type)>
bool operator!=(const Type& lhs,
                const int rhs)
{
    return lhs._asInt != rhs;
}

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(EnumType, Type)>
bool operator==(const Type& lhs,
                const int rhs)
{
    return lhs._asInt == rhs;
}

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(EnumType, Type)>
bool operator!=(const Type& lhs,
                const Type& rhs)
{
    return lhs._asInt != rhs._asInt;
}

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(EnumType, Type)>
bool operator==(const Type& lhs,
                const Type& rhs)
{
    return lhs._asInt == rhs._asInt;
}

template <class Type, DMIT_COM_TYPE_FLAG_CHECK_IS(EnumType, Type)>
using TEnumIntegerType = typename Type::TEnum::IntegerType;

#define DMIT_COM_ENUM_IMPLICIT_FROM_INT(EnumType) EnumType(const typename TEnum::IntegerType asInt) : TEnum{asInt} {}

} // namespace com

} // namespace dmit
