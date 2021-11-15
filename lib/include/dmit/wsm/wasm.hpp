#pragma once

#include "dmit/com/tree_node.hpp"
#include "dmit/com/tree_pool.hpp"
#include "dmit/com/enum.hpp"

#include <cstdint>

namespace dmit::wsm
{

namespace node
{


struct Kind : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        INST_CONST_I32    ,
        INST_CONST_I64    ,
        INST_CONST_F32    ,
        INST_CONST_F64    ,
        INST_DROP         ,
        INST_ELEM_DROP    ,
        INST_GLOBAL_GET   ,
        INST_GLOBAL_SET   ,
        INST_LOCAL_GET    ,
        INST_LOCAL_SET    ,
        INST_LOCAL_TEE    ,
        INST_I32          ,
        INST_F32          ,
        INST_I64          ,
        INST_F64          ,
        INST_I32_S        ,
        INST_F32_S        ,
        INST_I64_S        ,
        INST_F64_S        ,
        INST_I32_U        ,
        INST_F32_U        ,
        INST_I64_U        ,
        INST_F64_U        ,
        INST_REF_FUNC     ,
        INST_REF_NULL     ,
        INST_REF_IS_NULL  ,
        INST_SELECT       ,
        INST_TABLE_COPY   ,
        INST_TABLE_GET    ,
        INST_TABLE_SET    ,
        INST_TABLE_FILL   ,
        INST_TABLE_GROW   ,
        INST_TABLE_INIT   ,
        INST_TABLE_SIZE   ,
        TYPE_EXTERN       ,
        TYPE_GLOBAL_CONST ,
        TYPE_GLOBAL_VAR   ,
        TYPE_F32          ,
        TYPE_F64          ,
        TYPE_FUNC         ,
        TYPE_I32          ,
        TYPE_I64          ,
        TYPE_LIMITS       ,
        TYPE_MEM          ,
        TYPE_REF_FUNC     ,
        TYPE_REF_EXTERN   ,
        TYPE_RESULT       ,
        TYPE_TABLE        ,
        TYPE_VAL          ,
        BLOCK             ,
        MODULE
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Kind);

    using IntegerSequence = std::make_integer_sequence<uint8_t, MODULE + 1>;
};

template <com::TEnumIntegerType<Kind> KIND>
using TRange = typename com::tree::TMetaNode<Kind>::template TRange<KIND>;

template <com::TEnumIntegerType<Kind> KIND>
using TIndex = typename com::tree::TMetaNode<Kind>::template TIndex<KIND>;

} // namespace node

template <com::TEnumIntegerType<node::Kind> KIND>
struct TNode;

namespace node
{

template <uint8_t LOG2_SIZE>
using TPool = typename com::tree::TTMetaPool<Kind, TNode>::TPool<LOG2_SIZE>;

} // namespace node

template <> struct TNode<node::Kind::TYPE_I32> {};
template <> struct TNode<node::Kind::TYPE_F32> {};
template <> struct TNode<node::Kind::TYPE_I64> {};
template <> struct TNode<node::Kind::TYPE_F64> {};

using NumType = std::variant<TNode<node::Kind::TYPE_I32>,
                             TNode<node::Kind::TYPE_F32>,
                             TNode<node::Kind::TYPE_I64>,
                             TNode<node::Kind::TYPE_F64>>;

template <> struct TNode<node::Kind::TYPE_REF_EXTERN > {};
template <> struct TNode<node::Kind::TYPE_REF_FUNC   > {};

using RefType = std::variant<TNode<node::Kind::TYPE_REF_EXTERN>,
                             TNode<node::Kind::TYPE_REF_FUNC>>;

using ValType = std::variant<NumType,
                             RefType>;
template <>
struct TNode<node::Kind::TYPE_VAL>
{
    ValType _asVariant;
};

template <>
struct TNode<node::Kind::TYPE_RESULT>
{
    node::TRange<node::Kind::TYPE_VAL> _valTypes;
};

template <>
struct TNode<node::Kind::TYPE_FUNC>
{
    node::TIndex<node::Kind::TYPE_RESULT>   _domain;
    node::TIndex<node::Kind::TYPE_RESULT> _codomain;
};

template <>
struct TNode<node::Kind::TYPE_LIMITS>
{
    uint32_t                _min;
    std::optional<uint32_t> _maxOpt;
};

template <>
struct TNode<node::Kind::TYPE_MEM>
{
    uint32_t                _min;
    std::optional<uint32_t> _maxOpt;
};

template <>
struct TNode<node::Kind::TYPE_TABLE>
{
    node::TIndex<node::Kind::TYPE_LIMITS> _limits;
    RefType                               _refType;
};

template <>
struct TNode<node::Kind::TYPE_GLOBAL_VAR>
{
    node::TIndex<node::Kind::TYPE_VAL> _valType;
};

template <>
struct TNode<node::Kind::TYPE_GLOBAL_CONST>
{
    node::TIndex<node::Kind::TYPE_VAL> _valType;
};

using ExternType = std::variant<TNode<node::Kind::TYPE_GLOBAL_CONST>,
                                TNode<node::Kind::TYPE_GLOBAL_VAR>,
                                TNode<node::Kind::TYPE_FUNC>,
                                TNode<node::Kind::TYPE_MEM>,
                                TNode<node::Kind::TYPE_TABLE>>;
template <>
struct TNode<node::Kind::TYPE_EXTERN>
{
    ExternType _asVariant;
};

template <> struct TNode<node::Kind::INST_CONST_I32> { int32_t _value ; };
template <> struct TNode<node::Kind::INST_CONST_I64> { int64_t _value ; };
template <> struct TNode<node::Kind::INST_CONST_F32> { float   _value ; };
template <> struct TNode<node::Kind::INST_CONST_F64> { double  _value ; };

struct NumericInstruction : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        CLZ,
        CTZ,
        POPCNT,
        ADD,
        SUB,
        MUL,
        DIV,
        REM,
        AND,
        OR,
        XOR,
        SHL,
        SHR,
        ROTL,
        ROTR,
        ABS,
        NEG,
        SQRT,
        CEIL,
        FLOOR,
        TRUNC,
        NEAREST,
        MAX,
        COPYSIGN,
        EQZ,
        EQ,
        NE,
        LT,
        GT,
        LE,
        GE,
        EXTEND_8_S,
        EXTEND_16_S,
        EXTEND_32_S,
        EXTEND_I32,
        WRAP_I64,
        TRUNC_SAT,
        DEMOTE_F64,
        PROMOTE_F32,
        CONVERT,
        REINTERPRET
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(NumericInstruction);
};

template <>
struct TNode<node::Kind::INST_I32>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_F32>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_I64>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_F64>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_I32_S>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_F32_S>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_I64_S>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_F64_S>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_I32_U>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_F32_U>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_I64_U>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_F64_U>
{
    NumericInstruction      _asEnum;
    std::optional<NumType > _suffix;
};

template <>
struct TNode<node::Kind::INST_REF_NULL>
{
    RefType _refType;
};

template <>
struct TNode<node::Kind::INST_REF_IS_NULL>
{};

template <>
struct TNode<node::Kind::INST_REF_FUNC>
{
    uint32_t _funcIdx;
};

template <>
struct TNode<node::Kind::INST_DROP>
{};

template <>
struct TNode<node::Kind::INST_SELECT>
{
    node::TRange<node::Kind::TYPE_VAL> _valTypes;
};

template <> struct TNode<node::Kind::INST_LOCAL_GET> { uint32_t _localIdx; };
template <> struct TNode<node::Kind::INST_LOCAL_SET> { uint32_t _localIdx; };
template <> struct TNode<node::Kind::INST_LOCAL_TEE> { uint32_t _localIdx; };

template <> struct TNode<node::Kind::INST_GLOBAL_GET> { uint32_t _globalIdx; };
template <> struct TNode<node::Kind::INST_GLOBAL_SET> { uint32_t _globalIdx; };

template <> struct TNode<node::Kind::INST_TABLE_GET  > { uint32_t _tableIdx; };
template <> struct TNode<node::Kind::INST_TABLE_SET  > { uint32_t _tableIdx; };
template <> struct TNode<node::Kind::INST_TABLE_FILL > { uint32_t _tableIdx; };
template <> struct TNode<node::Kind::INST_TABLE_GROW > { uint32_t _tableIdx; };
template <> struct TNode<node::Kind::INST_TABLE_SIZE > { uint32_t _tableIdx; };

template <> struct TNode<node::Kind::INST_TABLE_COPY>
{
    uint32_t _screIdx;
    uint32_t _destIdx;
};

template <> struct TNode<node::Kind::INST_TABLE_INIT>
{
    uint32_t _tableIdx;
    uint32_t _elemIdx;
};

template <> struct TNode<node::Kind::INST_ELEM_DROP>
{
    uint32_t _elemIdx;
};

} // namespace dmit::wsm
