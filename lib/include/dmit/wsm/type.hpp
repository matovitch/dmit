#pragma once

#include "dmit/com/enum.hpp"

#include <optional>
#include <cstdint>
#include <variant>

namespace dmit::wsm::type
{

struct Number : com::TEnum<uint8_t>
{

    enum : uint8_t
    {
        I32,
        I64,
        F32,
        F64        
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Number);
};

struct Reference : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        FUNCREF,
        EXTERNREF
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Reference);
};

using Value = std::variant<Number, Reference>;

namespace value
{

struct Index
{
    uint32_t _value;
};

struct Vector
{
    Index _index;
    uint32_t _size;
};

} // namespace value

using Result = value::Vector;

struct Function
{
    Result _domain;
    Result _codomain;
};

struct Limits
{
    uint32_t _min;
    std::optional<uint32_t> _max;
};

using Memory = Limits;

struct Table
{
    Limits _limits;
    Reference _reftype;
};

struct Mut : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        _CONST,
        _VAR
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Mut);
};

struct Global
{
    Mut _mut;
    Value _valtype;
};

using External = std::variant<Function,
                              Table,
                              Memory,
                              Global>;

} // namespace dmit::wsm::type

