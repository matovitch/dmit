#pragma once

#include "dmit/wsm/type.hpp"

#include "dmit/com/enum.hpp"

#include <cstdint>
#include <variant>

namespace dmit::wsm::instruction
{

struct Width : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        _32,
        _64,
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Width);
};

struct Sign : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        U,
        S,
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Sign);
};

template <com::TEnumIntegerType<type::Number>>
struct TConst;

template <> struct TConst<type::Number::I32> { int32_t _value; };
template <> struct TConst<type::Number::F32> { float   _value; };
template <> struct TConst<type::Number::F64> { double  _value; };
template <> struct TConst<type::Number::I64> { int64_t _value; };

struct Iunop : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        CLZ,
        CTZ,
        POPCNT
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Iunop);
};

struct Ibinop : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        ADD,
        SUB,
        MUL,
        DIV_U,
        DIV_S,
        REM_U,
        REM_S,
        AND,
        OR,
        XOR,
        SHL,
        SHR_U,
        SHR_S,
        ROTL,
        ROTR
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Ibinop);
};

struct Funop : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        ABS,
        NEG,
        SQRT,
        CEIL,
        FLOOR,
        TRUNC,
        NEAREST
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Funop);
};

struct Fbinop : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        ADD,
        SUB,
        MUL,
        DIV,
        MIN,
        MAX,
        COPYSIGN
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Fbinop);
};

struct Itestop : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        EQZ
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Itestop);
};

struct Irelop : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        EQ,
        NE,
        LT_U,
        LT_S,
        GT_U,
        GT_S,
        LE_U,
        LE_S,
        GE_U,
        GE_S
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Irelop);
};

struct Frelop : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        EQ,
        NE,
        LT,
        GT,
        LE,
        GE
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Frelop);
};

struct Extend : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        _8_S,
        _16_S,
        _32_S,
        I_32_U,
        I_32_S,
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Extend);
};

struct WrapI64    {};
struct DemoteF64  {};
struct PromoteF32 {};

struct Trunc : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        F32_U,
        F32_S,
        F64_U,
        F64_S,
        SAT_F32_U,
        SAT_F32_S,
        SAT_F64_U,
        SAT_F64_S
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Trunc);
};

struct Convert : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        I32_U,
        I32_S,
        I64_U,
        I64_S
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Convert);
};

struct Reinterpret : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        I32,
        I64,
        F32,
        F64
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Reinterpret);
};

template <com::TEnumIntegerType<type::Number>>
struct TNumeric;

template <>
struct TNumeric<type::Number::I32>
{
    std::variant<TConst<type::Number::I32>, Iunop, Ibinop, Itestop, Irelop, Extend, Reinterpret, Trunc, WrapI64> _value;
};

template <>
struct TNumeric<type::Number::I64>
{
    std::variant<TConst<type::Number::I64>, Iunop, Ibinop, Itestop, Irelop, Extend, Reinterpret, Trunc> _value;
};

template <>
struct TNumeric<type::Number::F32>
{
    std::variant<TConst<type::Number::F32>, Funop, Fbinop, Frelop, Reinterpret, Convert, DemoteF64> _value;
};

template <>
struct TNumeric<type::Number::F64>
{
    std::variant<TConst<type::Number::F64>, Funop, Fbinop, Frelop, Reinterpret, Convert, PromoteF32> _value;
};

namespace ref
{

template <com::TEnumIntegerType<type::Reference>>
struct Null {};

struct IsNull{};

struct Func
{
    uint32_t _funcidx;
};

} // namespace ref

using Reference = std::variant<ref::Null<type::Reference::EXTERNREF>,
                               ref::Null<type::Reference::FUNCREF>,
                               ref::IsNull,
                               ref::Func>;

struct Drop {};

struct Select
{
    type::Value _valtype;
};

using Parametric = std::variant<Drop, Select>;

struct LocalGet  { uint32_t _localidx; };
struct LocalSet  { uint32_t _localidx; };
struct GlobalGet { uint32_t _globalidx; };
struct GlobalSet { uint32_t _globalidx; };

using Variable = std::variant<LocalGet, LocalSet, GlobalSet, GlobalGet>;



} // namespace dmit::wsm::instruction
