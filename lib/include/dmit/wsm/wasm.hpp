#pragma once

#include "dmit/com/tree_node.hpp"
#include "dmit/com/tree_pool.hpp"
#include "dmit/com/ieee754.hpp"
#include "dmit/com/enum.hpp"

#include <optional>
#include <cstdint>
#include <variant>

namespace dmit::wsm
{

namespace node
{

struct Kind : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        INST_BLOCK         ,
        INST_BR            ,
        INST_BR_IF         ,
        INST_BR_TABLE      ,
        INST_IF            ,
        INST_CALL          ,
        INST_CALL_INDIRECT ,
        INST_LOOP          ,
        INST_RETURN        ,
        INST_CONST_I32     ,
        INST_CONST_I64     ,
        INST_CONST_F32     ,
        INST_CONST_F64     ,
        INST_DATA_DROP     ,
        INST_DROP          ,
        INST_ELEM_DROP     ,
        INST_GLOBAL_GET    ,
        INST_GLOBAL_SET    ,
        INST_LOCAL_GET     ,
        INST_LOCAL_SET     ,
        INST_LOCAL_TEE     ,
        INST_I32           ,
        INST_F32           ,
        INST_I64           ,
        INST_F64           ,
        INST_MEM_I32       ,
        INST_MEM_F32       ,
        INST_MEM_I64       ,
        INST_MEM_F64       ,
        INST_MEM_SIZE      ,
        INST_MEM_GROW      ,
        INST_MEM_FILL      ,
        INST_MEM_COPY      ,
        INST_MEM_INIT      ,
        INST_NOP           ,
        INST_REF_FUNC      ,
        INST_REF_NULL      ,
        INST_REF_IS_NULL   ,
        INST_SELECT        ,
        INST_TABLE_COPY    ,
        INST_TABLE_GET     ,
        INST_TABLE_SET     ,
        INST_TABLE_FILL    ,
        INST_TABLE_GROW    ,
        INST_TABLE_INIT    ,
        INST_TABLE_SIZE    ,
        INST_UNREACHABLE   ,
        INSTRUCTION        ,
        MEMARG             ,
        TYPE_EXTERN        ,
        TYPE_GLOBAL_CONST  ,
        TYPE_GLOBAL_VAR    ,
        TYPE_F32           ,
        TYPE_F64           ,
        TYPE_FUNC          ,
        TYPE_I32           ,
        TYPE_I64           ,
        TYPE_LIMITS        ,
        TYPE_MEM           ,
        TYPE_REF_FUNC      ,
        TYPE_REF_EXTERN    ,
        TYPE_RESULT        ,
        TYPE_TABLE         ,
        TYPE_VAL           ,
        GLOBAL_CONST       ,
        GLOBAL_VAR         ,
        BYTE_              ,
        LABEL              ,
        FUNCTION           ,
        ELEMENT            ,
        ELEM_ACTIVE        ,
        ELEM_PASSIVE       ,
        ELEM_DECLARATIVE   ,
        EXPRESSION         ,
        DATA               ,
        DATA_PASSIVE       ,
        DATA_ACTIVE        ,
        START              ,
        IMPORT             ,
        EXPORT             ,
        NAME               ,
        RELOCATION         ,
        SYMBOL             ,
        SYMBOL_DATA        ,
        SYMBOL_IMPORT      ,
        MODULE
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Kind);

    using IntegerSequence = std::make_integer_sequence<uint8_t, MODULE + 1>;
};

template <com::TEnumIntegerType<Kind> KIND>
using TRange = typename com::tree::TMetaNode<Kind>::template TRange<KIND>;

template <com::TEnumIntegerType<Kind> KIND>
using TIndex = typename com::tree::TMetaNode<Kind>::template TIndex<KIND>;

using VIndex = typename com::tree::TMetaNode<Kind>::VIndex;

using Index = typename com::tree::TMetaNode<Kind>::Index;

template <com::TEnumIntegerType<Kind> KIND>
auto as = [](auto value) -> TIndex<KIND>
{
    return com::tree::as<Kind, KIND>(value);
};

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

using NumType = std::variant<node::TIndex<node::Kind::TYPE_I32>,
                             node::TIndex<node::Kind::TYPE_F32>,
                             node::TIndex<node::Kind::TYPE_I64>,
                             node::TIndex<node::Kind::TYPE_F64>>;

template <> struct TNode<node::Kind::TYPE_REF_EXTERN > {};
template <> struct TNode<node::Kind::TYPE_REF_FUNC   > {};

using RefType = std::variant<node::TIndex<node::Kind::TYPE_REF_EXTERN>,
                             node::TIndex<node::Kind::TYPE_REF_FUNC>>;

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

using ExternType = std::variant<node::TIndex<node::Kind::TYPE_GLOBAL_CONST>,
                                node::TIndex<node::Kind::TYPE_GLOBAL_VAR>,
                                node::TIndex<node::Kind::TYPE_FUNC>,
                                node::TIndex<node::Kind::TYPE_MEM>,
                                node::TIndex<node::Kind::TYPE_TABLE>>;
template <>
struct TNode<node::Kind::TYPE_EXTERN>
{
    ExternType _asVariant;
};

struct RelocationType : com::TEnum<int8_t>
{
    enum : int8_t
    {
        NONE = -1,
        FUNCTION_INDEX_LEB,
        TABLE_INDEX_SLEB,
        TABLE_INDEX_I32,
        MEMORY_ADDR_LEB,
        MEMORY_ADDR_SLEB,
        MEMORY_ADDR_I32,
        TYPE_INDEX_LEB,
        GLOBAL_INDEX_LEB,
        FUNCTION_OFFSET_I32,
        SECTION_OFFSET_I32,
        EVENT_INDEX_LEB,
        GLOBAL_INDEX_I32
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(RelocationType);
};

template <>
struct TNode<node::Kind::RELOCATION>
{
    node::TIndex<node::Kind::RELOCATION> _next;
    uint32_t _addend;
    uint32_t _offset;
    uint32_t _index;
    RelocationType _type;
};

using flt32_t = com::ieee754::Binary<32>;
using flt64_t = com::ieee754::Binary<64>;

template <> struct TNode<node::Kind::INST_CONST_I32> { node::TIndex<node::Kind::RELOCATION> _relocation;
                                                       int32_t _value; };
template <> struct TNode<node::Kind::INST_CONST_I64> { int64_t _value; };
template <> struct TNode<node::Kind::INST_CONST_F32> { flt32_t _value; };
template <> struct TNode<node::Kind::INST_CONST_F64> { flt64_t _value; };

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
        DIV_U,
        DIV_S,
        REM,
        REM_U,
        REM_S,
        AND,
        OR,
        XOR,
        SHL,
        SHR_U,
        SHR_S,
        ROTL,
        ROTR,
        ABS,
        NEG,
        SQRT,
        CEIL,
        FLOOR,
        TRUNC,
        TRUNC_S,
        TRUNC_U,
        NEAREST,
        MIN,
        MAX,
        COPYSIGN,
        EQZ,
        EQ,
        NE,
        LT,
        LT_U,
        LT_S,
        GT,
        GT_U,
        GT_S,
        LE,
        LE_U,
        LE_S,
        GE,
        GE_U,
        GE_S,
        EXTEND_8_S,
        EXTEND_16_S,
        EXTEND_32_S,
        EXTEND_I32_U,
        EXTEND_I32_S,
        WRAP_I64,
        TRUNC_SAT_S,
        TRUNC_SAT_U,
        DEMOTE_F64,
        PROMOTE_F32,
        CONVERT_S,
        CONVERT_U,
        REINTERPRET
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(NumericInstruction);
};

template <>
struct TNode<node::Kind::INST_I32>
{
    NumericInstruction _asEnum;
    NumType            _suffix;
};

template <>
struct TNode<node::Kind::INST_F32>
{
    NumericInstruction _asEnum;
    NumType            _suffix;
};

template <>
struct TNode<node::Kind::INST_I64>
{
    NumericInstruction _asEnum;
    NumType            _suffix;
};

template <>
struct TNode<node::Kind::INST_F64>
{
    NumericInstruction _asEnum;
    NumType            _suffix;
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

template <>
struct TNode<node::Kind::INST_TABLE_COPY>
{
    uint32_t _srceIdx;
    uint32_t _destIdx;
};

template <>
struct TNode<node::Kind::INST_TABLE_INIT>
{
    uint32_t _tableIdx;
    uint32_t _elemIdx;
};

template <>
struct TNode<node::Kind::INST_ELEM_DROP>
{
    uint32_t _elemIdx;
};

template <>
struct TNode<node::Kind::MEMARG>
{
    uint32_t _offset;
    uint32_t _align;
};

struct MemoryInstruction : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        LOAD,
        LOAD_8_S,
        LOAD_8_U,
        LOAD_16_S,
        LOAD_16_U,
        LOAD_32_S,
        LOAD_32_U,
        STORE,
        STORE_8,
        STORE_16,
        STORE_32
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(MemoryInstruction);
};

template <>
struct TNode<node::Kind::INST_MEM_I32>
{
    MemoryInstruction                _asEnum;
    node::TIndex<node::Kind::MEMARG> _memarg;
};

template <>
struct TNode<node::Kind::INST_MEM_F32>
{
    MemoryInstruction                _asEnum;
    node::TIndex<node::Kind::MEMARG> _memarg;
};

template <>
struct TNode<node::Kind::INST_MEM_I64>
{
    MemoryInstruction                _asEnum;
    node::TIndex<node::Kind::MEMARG> _memarg;
};

template <>
struct TNode<node::Kind::INST_MEM_F64>
{
    MemoryInstruction                _asEnum;
    node::TIndex<node::Kind::MEMARG> _memarg;
};

template <> struct TNode<node::Kind::INST_MEM_SIZE> {};
template <> struct TNode<node::Kind::INST_MEM_GROW> {};
template <> struct TNode<node::Kind::INST_MEM_FILL> {};
template <> struct TNode<node::Kind::INST_MEM_COPY> {};

template <>
struct TNode<node::Kind::INST_MEM_INIT>
{
    uint32_t _dataIdx;
};

template <>
struct TNode<node::Kind::INST_DATA_DROP>
{
    uint32_t _dataIdx;
};


template <> struct TNode<node::Kind::INST_NOP         > {};
template <> struct TNode<node::Kind::INST_UNREACHABLE > {};

using BlockType = std::variant<uint32_t,
                               std::optional<ValType>>;

using Instruction = std::variant<
    node::TIndex<node::Kind::INST_BLOCK         > ,
    node::TIndex<node::Kind::INST_BR            > ,
    node::TIndex<node::Kind::INST_BR_IF         > ,
    node::TIndex<node::Kind::INST_BR_TABLE      > ,
    node::TIndex<node::Kind::INST_IF            > ,
    node::TIndex<node::Kind::INST_CALL          > ,
    node::TIndex<node::Kind::INST_CALL_INDIRECT > ,
    node::TIndex<node::Kind::INST_LOOP          > ,
    node::TIndex<node::Kind::INST_RETURN        > ,
    node::TIndex<node::Kind::INST_CONST_I32     > ,
    node::TIndex<node::Kind::INST_CONST_I64     > ,
    node::TIndex<node::Kind::INST_CONST_F32     > ,
    node::TIndex<node::Kind::INST_CONST_F64     > ,
    node::TIndex<node::Kind::INST_DATA_DROP     > ,
    node::TIndex<node::Kind::INST_DROP          > ,
    node::TIndex<node::Kind::INST_ELEM_DROP     > ,
    node::TIndex<node::Kind::INST_GLOBAL_GET    > ,
    node::TIndex<node::Kind::INST_GLOBAL_SET    > ,
    node::TIndex<node::Kind::INST_LOCAL_GET     > ,
    node::TIndex<node::Kind::INST_LOCAL_SET     > ,
    node::TIndex<node::Kind::INST_LOCAL_TEE     > ,
    node::TIndex<node::Kind::INST_I32           > ,
    node::TIndex<node::Kind::INST_F32           > ,
    node::TIndex<node::Kind::INST_I64           > ,
    node::TIndex<node::Kind::INST_F64           > ,
    node::TIndex<node::Kind::INST_MEM_I32       > ,
    node::TIndex<node::Kind::INST_MEM_F32       > ,
    node::TIndex<node::Kind::INST_MEM_I64       > ,
    node::TIndex<node::Kind::INST_MEM_F64       > ,
    node::TIndex<node::Kind::INST_MEM_SIZE      > ,
    node::TIndex<node::Kind::INST_MEM_GROW      > ,
    node::TIndex<node::Kind::INST_MEM_FILL      > ,
    node::TIndex<node::Kind::INST_MEM_COPY      > ,
    node::TIndex<node::Kind::INST_MEM_INIT      > ,
    node::TIndex<node::Kind::INST_NOP           > ,
    node::TIndex<node::Kind::INST_REF_FUNC      > ,
    node::TIndex<node::Kind::INST_REF_NULL      > ,
    node::TIndex<node::Kind::INST_REF_IS_NULL   > ,
    node::TIndex<node::Kind::INST_SELECT        > ,
    node::TIndex<node::Kind::INST_TABLE_COPY    > ,
    node::TIndex<node::Kind::INST_TABLE_GET     > ,
    node::TIndex<node::Kind::INST_TABLE_SET     > ,
    node::TIndex<node::Kind::INST_TABLE_FILL    > ,
    node::TIndex<node::Kind::INST_TABLE_GROW    > ,
    node::TIndex<node::Kind::INST_TABLE_INIT    > ,
    node::TIndex<node::Kind::INST_TABLE_SIZE    > ,
    node::TIndex<node::Kind::INST_UNREACHABLE   >
>;

template <>
struct TNode<node::Kind::INSTRUCTION>
{
    Instruction _asVariant;
};

template<>
struct TNode<node::Kind::INST_BLOCK>
{
    BlockType _type;
    node::TRange<node::Kind::INSTRUCTION> _instructions;
};

template<>
struct TNode<node::Kind::INST_LOOP>
{
    BlockType _type;
    node::TRange<node::Kind::INSTRUCTION> _instructions;
};

template<>
struct TNode<node::Kind::INST_IF>
{
    BlockType _type;
    node::TRange<node::Kind::INSTRUCTION> _then;
    node::TRange<node::Kind::INSTRUCTION> _else;
};

template<>
struct TNode<node::Kind::INST_BR>
{
    uint32_t _labelIdx;
};

template<>
struct TNode<node::Kind::INST_BR_IF>
{
    uint32_t _labelIdx;
};

template<>
struct TNode<node::Kind::LABEL>
{
    uint32_t _labelIdx;
};

template<>
struct TNode<node::Kind::INST_BR_TABLE>
{
    node::TRange<node::Kind::LABEL> _labels;
    uint32_t                        _labelIdx;
};

template<> struct TNode<node::Kind::INST_RETURN> {};

template<>
struct TNode<node::Kind::INST_CALL>
{
    node::TIndex<node::Kind::RELOCATION> _relocation;
    uint32_t _funcIdx;
};

template<>
struct TNode<node::Kind::INST_CALL_INDIRECT>
{
    uint32_t _tableIdx;
    uint32_t _typeIdx;
};

template<>
struct TNode<node::Kind::FUNCTION>
{
    uint32_t                               _typeIdx;
    node::TRange<node::Kind::TYPE_VAL    > _locals;
    node::TRange<node::Kind::INSTRUCTION > _body;
};

template<>
struct TNode<node::Kind::GLOBAL_CONST>
{
    node::TIndex<node::Kind::TYPE_GLOBAL_CONST > _type;
    node::TRange<node::Kind::INSTRUCTION       > _init;
};

template<>
struct TNode<node::Kind::GLOBAL_VAR>
{
    node::TIndex<node::Kind::TYPE_GLOBAL_VAR > _type;
    node::TRange<node::Kind::INSTRUCTION     > _init;
};

struct SymbolKind : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        DATA,
        FUNCTION,
        GLOBAL,
        EVENT,
        TABLE,
        SECTION
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(SymbolKind);
};

struct SymbolFlag
{
    enum : uint32_t
    {
        BINDING_WEAK        = 0b00000001,
        BINDING_LOCAL       = 0b00000010,
        VISIBILITY_HIDDEN   = 0b00000100,
        UNDEFINED           = 0b00010000, // No 0x8 for some reason
        EXPORTED            = 0b00100000,
        EXPLICIT_NAME       = 0b01000000,
        NO_STRIP            = 0b10000000
    };
};

template<>
struct TNode<node::Kind::SYMBOL_IMPORT>
{
    uint32_t _importIdx;
};

template<>
struct TNode<node::Kind::SYMBOL_DATA>
{
    node::TIndex<node::Kind::NAME> _name;
    uint32_t _index;
    uint32_t _offset;
    uint32_t _size;
};

template<>
struct TNode<node::Kind::SYMBOL>
{
    SymbolKind _kind;
    uint32_t   _flags;
    std::variant<node::TIndex<node::Kind::SYMBOL_IMPORT>,
                 node::TIndex<node::Kind::SYMBOL_DATA>> _asVariant;
};

template<>
struct TNode<node::Kind::MODULE>
{
    node::TRange<node::Kind::TYPE_FUNC    > _types;
    node::TRange<node::Kind::FUNCTION     > _funcs;
    node::TRange<node::Kind::TYPE_TABLE   > _tables;
    node::TRange<node::Kind::TYPE_MEM     > _mems;
    node::TRange<node::Kind::GLOBAL_CONST > _globalConsts;
    node::TRange<node::Kind::GLOBAL_VAR   > _globalVars;
    node::TRange<node::Kind::ELEMENT      > _elems;
    node::TRange<node::Kind::DATA         > _datas;
    node::TRange<node::Kind::IMPORT       > _imports;
    node::TRange<node::Kind::EXPORT       > _exports;

    std::optional<node::TIndex<node::Kind::START>> _startOpt;
    node::TIndex<node::Kind::RELOCATION> _relocCode;
    node::TIndex<node::Kind::RELOCATION> _relocData;

    uint32_t _relocSizeCode;
    uint32_t _relocSizeData;

    node::TRange<node::Kind::SYMBOL> _symbols;
};

template<> struct TNode<node::Kind::ELEM_PASSIVE     > {};
template<> struct TNode<node::Kind::ELEM_DECLARATIVE > {};

template <>
struct TNode<node::Kind::ELEM_ACTIVE>
{
    uint32_t                              _tableIdx;
    node::TRange<node::Kind::INSTRUCTION> _offset;
};

using ElementMode = std::variant<node::TIndex<node::Kind::ELEM_ACTIVE>,
                                 node::TIndex<node::Kind::ELEM_PASSIVE>,
                                 node::TIndex<node::Kind::ELEM_DECLARATIVE>>;
template <>
struct TNode<node::Kind::EXPRESSION>
{
    node::TRange<node::Kind::INSTRUCTION> _instructions;
};

template <>
struct TNode<node::Kind::ELEMENT>
{
    RefType _type;
    node::TRange<node::Kind::EXPRESSION> _init;
    ElementMode _mode;
};

template <> struct TNode<node::Kind::DATA_PASSIVE> {};

template <>
struct TNode<node::Kind::DATA_ACTIVE>
{
    uint32_t _memIdx;
    node::TRange<node::Kind::INSTRUCTION> _offset;
};

using DataMode = std::variant<node::TIndex<node::Kind::DATA_PASSIVE>,
                              node::TIndex<node::Kind::DATA_ACTIVE>>;
template <>
struct TNode<node::Kind::BYTE_>
{
    uint8_t _value;
};

template <>
struct TNode<node::Kind::DATA>
{
    node::TRange<node::Kind::BYTE_> _init;
    DataMode                        _mode;
};

template <>
struct TNode<node::Kind::START>
{
    uint32_t _funcIdx;
};

using ImportDescriptor = std::variant<uint32_t, // typeidx
                                      node::TIndex<node::Kind::TYPE_GLOBAL_CONST >,
                                      node::TIndex<node::Kind::TYPE_GLOBAL_VAR   >,
                                      node::TIndex<node::Kind::TYPE_TABLE        >,
                                      node::TIndex<node::Kind::TYPE_MEM          >>;
template <>
struct TNode<node::Kind::NAME>
{
    node::TRange<node::Kind::BYTE_> _bytes;
};

template <>
struct TNode<node::Kind::IMPORT>
{
    node::TIndex<node::Kind::NAME> _module;
    node::TIndex<node::Kind::NAME> _name;
    ImportDescriptor               _descriptor;
};

using ExportDescriptor = std::variant<uint32_t, // memidx
                                      node::TIndex<node::Kind::INST_REF_FUNC   >,
                                      node::TIndex<node::Kind::INST_TABLE_GET  >,
                                      node::TIndex<node::Kind::INST_GLOBAL_GET >>;
template <>
struct TNode<node::Kind::EXPORT>
{
    node::TIndex<node::Kind::NAME> _name;
    ExportDescriptor               _descriptor;
};

} // namespace dmit::wsm
