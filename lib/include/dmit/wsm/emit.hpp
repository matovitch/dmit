#pragma once

#include "dmit/wsm/emit_block_type.hpp"
#include "dmit/wsm/emit_port.hpp"
#include "dmit/wsm/emit_size.hpp"
#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/v_index.hpp"
#include "dmit/wsm/writer.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/tree_visitor.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/endian.hpp"
#include "dmit/com/enum.hpp"

#include <type_traits>
#include <algorithm>
#include <optional>
#include <cstdint>
#include <variant>

namespace dmit::wsm
{

struct SectionId : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        CUSTOM,
        TYPE,
        IMPORT,
        FUNCTION,
        TABLE,
        MEMORY,
        GLOBAL,
        EXPORT,
        START,
        ELEMENT,
        CODE,
        DATA,
        DATA_COUNT
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(SectionId);
};

struct StackDummy {};

template <class Derived, class NodePool, bool REVERSE_LIST>
using TBaseVisitor = typename com::tree::TTMetaVisitor<node::Kind,
                                                       TNode,
                                                       NodePool>::template TVisitor<Derived,
                                                                                    StackDummy,
                                                                                    StackDummy,
                                                                                    REVERSE_LIST>;
template <bool IS_OBJECT, class NodePool, class Writer>
struct TEmitter : TBaseVisitor<TEmitter<IS_OBJECT, NodePool, Writer>, NodePool, Writer::REVERSE_LIST>
{
    using Base = TBaseVisitor<TEmitter<IS_OBJECT, NodePool, Writer>, NodePool, Writer::REVERSE_LIST>;
    using Base::_nodePool, Base::base, Base::get, Base::empty;

    TEmitter(NodePool& nodePool, Writer& writer) :
        Base{nodePool},
        _writer{writer},
        _importDescriptorEmitter{nodePool, writer},
        _exportDescriptorEmitter{nodePool, writer},
        _blockTypeEmitter{nodePool, writer}
    {}

    template <class LimitsIdx>
    void writeLimits(LimitsIdx limitsIdx)
    {
        auto& limits = get(limitsIdx);

        Leb128</*IS_OBJECT=*/false> limitsMinAsLeb128{limits._min};

        if (limits._maxOpt)
        {
            Leb128</*IS_OBJECT=*/false> limitsMaxAsLeb128{limits._maxOpt.value()};

            _writer.write(0x01);
            _writer.write(limitsMinAsLeb128);
            _writer.write(limitsMaxAsLeb128);
        }
        else
        {
            _writer.write(0x00);
            _writer.write(limitsMinAsLeb128);
        }
    }

    void operator()(node::TIndex<node::Kind::TYPE_I32        >) { _writer.write(0x7F); }
    void operator()(node::TIndex<node::Kind::TYPE_F32        >) { _writer.write(0x7D); }
    void operator()(node::TIndex<node::Kind::TYPE_I64        >) { _writer.write(0x7E); }
    void operator()(node::TIndex<node::Kind::TYPE_F64        >) { _writer.write(0x7C); }
    void operator()(node::TIndex<node::Kind::TYPE_REF_FUNC   >) { _writer.write(0x70); }
    void operator()(node::TIndex<node::Kind::TYPE_REF_EXTERN >) { _writer.write(0x6F); }

    void operator()(node::TIndex<node::Kind::TYPE_VAL> typeValIdx)
    {
        auto& typeVal = get(typeValIdx);

        base()(typeVal._asVariant);
    }

    void operator()(node::TIndex<node::Kind::TYPE_RESULT> typeResultIdx)
    {
        auto& typeResult = get(typeResultIdx);

        emitRangeWithSize(typeResult._valTypes);
    }

    void operator()(node::TIndex<node::Kind::TYPE_FUNC> typeFuncIdx)
    {
        auto& typeFunc = get(typeFuncIdx);

        _writer.write(0x60);

        base()(typeFunc.  _domain);
        base()(typeFunc._codomain);
    }

    void operator()(node::TIndex<node::Kind::BYTE_> byteIdx)
    {
        auto& byte = get(byteIdx);

        _writer.write(byte._value);
    }

    void operator()(node::TIndex<node::Kind::NAME> nameIdx)
    {
        auto& name = get(nameIdx);

        emitRangeWithSize(name._bytes);
    }

    void operator()(node::TIndex<node::Kind::IMPORT> importIdx)
    {
        auto& import = get(importIdx);

        base()(import._module);
        base()(import._name);

        std::visit(_importDescriptorEmitter, import._descriptor);
    }

    void operator()(node::TIndex<node::Kind::EXPORT> exportIdx)
    {
        auto& export_ = get(exportIdx);

        base()(export_._name);

        std::visit(_exportDescriptorEmitter, export_._descriptor);
    }

    void operator()(node::TIndex<node::Kind::LOCAL> localIdx)
    {
        _writer.write(0x01);
        base()(get(localIdx)._type);
    }

    void operator()(node::TIndex<node::Kind::FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        Leb128</*IS_OBJECT=*/false> sizeLocalsAsLeb128{function._localsSize};

        _writer.write(sizeLocalsAsLeb128);

        base()(function._locals);

        emitListWithSentinel(function._body);
    }

    void operator()(node::TIndex<node::Kind::TYPE_TABLE> typeTableIdx)
    {
        auto& typeTable = get(typeTableIdx);

        writeLimits(typeTable._limits);
        base()(typeTable._refType);
    }

    void operator()(node::TIndex<node::Kind::TYPE_MEM> typeMemIdx)
    {
        writeLimits(typeMemIdx);
    }

    void operator()(node::TIndex<node::Kind::TYPE_GLOBAL_VAR> typeGlobalVarIdx)
    {
        auto& typeGlobalVar = get(typeGlobalVarIdx);

        _writer.write(0x01);
        base()(typeGlobalVar._valType);
    }

    void operator()(node::TIndex<node::Kind::TYPE_GLOBAL_CONST> typeGlobalConstIdx)
    {
        auto& typeGlobalConst = get(typeGlobalConstIdx);

        _writer.write(0x00);
        base()(typeGlobalConst._valType);
    }

    void operator()(node::TIndex<node::Kind::START> startIdx)
    {
        auto& start = get(startIdx);

        Leb128<IS_OBJECT> funcIdxAsLeb128{node::v_index::makeId(_nodePool, start._function)};
        _writer.write(funcIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::LABEL> labelIdx)
    {
        auto& label = get(labelIdx);

        Leb128</*IS_OBJECT=*/false> labelIdxAsLeb128{label._labelIdx};

        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::MEMARG> memargIdx)
    {
        auto& memarg = get(memargIdx);

        Leb128</*IS_OBJECT=*/false>  alignAsLeb128{memarg._align  };
        Leb128</*IS_OBJECT=*/false> offsetAsLeb128{memarg._offset };

        _writer.write( alignAsLeb128);
        _writer.write(offsetAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_BLOCK> instBlockIdx)
    {
        auto& instBlock = get(instBlockIdx);

        _writer.write(0x02);

        std::visit(_blockTypeEmitter, instBlock._type);

        emitListWithSentinel(instBlock._instructions);
    }

    void operator()(node::TIndex<node::Kind::INST_LOOP> instLoopIdx)
    {
        auto& instLoop = get(instLoopIdx);

        _writer.write(0x03);

        std::visit(_blockTypeEmitter, instLoop._type);

        emitListWithSentinel(instLoop._instructions);
    }

    void operator()(node::TIndex<node::Kind::INST_IF> instIfIdx)
    {
        auto& instIf = get(instIfIdx);

        _writer.write(0x04);

        TBlockTypeEmitter<NodePool, Writer> blockTypeEmitter{_nodePool, _writer};

        std::visit(blockTypeEmitter, instIf._type);

        base()(instIf._then);

        if (!empty(instIf._else))
        {
            _writer.write(0x05);
            base()(instIf._else);
        }

        _writer.write(0x0B);
    }

    void operator()(node::TIndex<node::Kind::INST_BR> instBrIdx)
    {
        auto& instBr = get(instBrIdx);

        Leb128</*IS_OBJECT=*/false> labelIdxAsLeb128{instBr._labelIdx};

        _writer.write(0x0C);
        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_BR_IF> instBrIfIdx)
    {
        auto& instBrIf = get(instBrIfIdx);

        Leb128</*IS_OBJECT=*/false> labelIdxAsLeb128{instBrIf._labelIdx};

        _writer.write(0x0D);
        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_BR_TABLE> instBrTableIdx)
    {
        auto& instBrTable = get(instBrTableIdx);

        emitRangeWithSize(instBrTable._labels);

        Leb128</*IS_OBJECT=*/false> labelIdxAsLeb128{instBrTable._labelIdx};

        _writer.write(0x0E);
        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CALL> instCallIdx)
    {
        auto& instCall   = get(instCallIdx);
        auto& relocation = get(instCall._relocation);

        _writer.write(0x10);

        if (IS_OBJECT && relocation._type != RelocationType::NONE)
        {
            relocation._offset = _writer.diff(_writerSection);
            (*_relocOrderedCode)[_relocOrderedCode->_size++] = instCall._relocation;
        }

        Leb128<IS_OBJECT> funcIdxAsLeb128{node::v_index::makeId(_nodePool, instCall._function)};
        _writer.write(funcIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CALL_INDIRECT> instCallIndirectIdx)
    {
        auto& instCallIndirect = get(instCallIndirectIdx);

        _writer.write(0x11);

        Leb128<IS_OBJECT>  typeIdxAsLeb128{get(instCallIndirect._type)._id};
        Leb128<IS_OBJECT> tableIdxAsLeb128{instCallIndirect._tableIdx };
        _writer.write( typeIdxAsLeb128);
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_REF_NULL> instRefNullIdx)
    {
        auto& instRefNull = get(instRefNullIdx);

        _writer.write(0xD0);

        base()(instRefNull._refType);
    }

    void operator()(node::TIndex<node::Kind::INST_REF_FUNC> instRefFuncIdx)
    {
        auto& instRefFunc = get(instRefFuncIdx);

        _writer.write(0xD2);

        Leb128<IS_OBJECT> funcIdxAsLeb128{node::v_index::makeId(_nodePool, instRefFunc._function)};
        _writer.write(funcIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_SELECT> instSelectIdx)
    {
        auto& instSelect = get(instSelectIdx);

        if (instSelect._valTypes._size == 0)
        {
            _writer.write(0x1B);
            return;
        }

        _writer.write(0x1C);
        emitRangeWithSize(instSelect._valTypes);
    }

    void operator()(node::TIndex<node::Kind::INST_LOCAL_GET> instLocalGetIdx)
    {
        auto& instLocalGet = get(instLocalGetIdx);

        Leb128</*IS_OBJECT=*/false> localIdxAsLeb128{get(instLocalGet._local)._id};

        _writer.write(0x20);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_LOCAL_SET> instLocalSetIdx)
    {
        auto& instLocalSet = get(instLocalSetIdx);

        Leb128</*IS_OBJECT=*/false> localIdxAsLeb128{get(instLocalSet._local)._id};

        _writer.write(0x21);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_LOCAL_TEE> instLocalTeeIdx)
    {
        auto& instLocalTee = get(instLocalTeeIdx);

        Leb128</*IS_OBJECT=*/false> localIdxAsLeb128{get(instLocalTee._local)._id};

        _writer.write(0x22);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_GLOBAL_GET> instGlobalGetIdx)
    {
        auto& instGlobalGet = get(instGlobalGetIdx);

        _writer.write(0x23);

        Leb128<IS_OBJECT> localIdxAsLeb128{instGlobalGet._globalIdx};
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_GLOBAL_SET> instGlobalSetIdx)
    {
        auto& instGlobalSet = get(instGlobalSetIdx);

        _writer.write(0x24);

        Leb128<IS_OBJECT> localIdxAsLeb128{instGlobalSet._globalIdx};
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_GET> instTableGetIdx)
    {
        auto& instTableGet = get(instTableGetIdx);

        _writer.write(0x25);

        Leb128<IS_OBJECT> tableIdxAsLeb128{instTableGet._tableIdx};
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_SET> instTableSetIdx)
    {
        auto& instTableSet = get(instTableSetIdx);

        _writer.write(0x26);

        Leb128<IS_OBJECT> tableIdxAsLeb128{instTableSet._tableIdx};
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_COPY> instTableCopyIdx)
    {
        auto& instTableCopy = get(instTableCopyIdx);

        Leb128</*IS_OBJECT=*/false> srceIdxAsLeb128{instTableCopy._srceIdx };
        Leb128</*IS_OBJECT=*/false> destIdxAsLeb128{instTableCopy._destIdx };

        _writer.write(0xFC);
        _writer.write(Leb128</*IS_OBJECT=*/false>{14u});
        _writer.write(srceIdxAsLeb128);
        _writer.write(destIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_GROW> instTableGrowIdx)
    {
        auto& instTableGrow = get(instTableGrowIdx);


        _writer.write(0xFC);
        _writer.write(Leb128</*IS_OBJECT=*/false>{15u});

        Leb128<IS_OBJECT> tableIdxAsLeb128{instTableGrow._tableIdx};
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_SIZE> instTableSizeIdx)
    {
        auto& instTableSize = get(instTableSizeIdx);

        _writer.write(0xFC);
        _writer.write(Leb128</*IS_OBJECT=*/false>{16u});

        Leb128<IS_OBJECT> tableIdxAsLeb128{instTableSize._tableIdx};
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_FILL> instTableFillIdx)
    {
        auto& instTableFill = get(instTableFillIdx);

        _writer.write(0xFC);
        _writer.write(Leb128</*IS_OBJECT=*/false>{17u});

        Leb128<IS_OBJECT> tableIdxAsLeb128{instTableFill._tableIdx};
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_SIZE>)
    {
        _writer.write(0x3F);
        _writer.write(0x00);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_GROW>)
    {
        _writer.write(0x40);
        _writer.write(0x00);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_COPY>)
    {
        _writer.write(0xFC);
        _writer.write(Leb128</*IS_OBJECT=*/false>{10u});
        _writer.write(0x00);
        _writer.write(0x00);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_FILL>)
    {
        _writer.write(0xFC);
        _writer.write(Leb128</*IS_OBJECT=*/false>{11u});
        _writer.write(0x00);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_I64> instMemI64Idx)
    {
        auto& instMemI64 = get(instMemI64Idx);

        auto instMemI64AsEnum = instMemI64._asEnum;

        if (instMemI64AsEnum == MemoryInstruction::LOAD)
        {
            _writer.write(0x29);
        }
        else if (instMemI64AsEnum == MemoryInstruction::LOAD_8_S)
        {
            _writer.write(0x30);
        }
        else if (instMemI64AsEnum == MemoryInstruction::LOAD_8_U)
        {
            _writer.write(0x31);
        }
        else if (instMemI64AsEnum == MemoryInstruction::LOAD_16_S)
        {
            _writer.write(0x32);
        }
        else if (instMemI64AsEnum == MemoryInstruction::LOAD_16_U)
        {
            _writer.write(0x33);
        }
        else if (instMemI64AsEnum == MemoryInstruction::LOAD_32_S)
        {
            _writer.write(0x34);
        }
        else if (instMemI64AsEnum == MemoryInstruction::LOAD_32_U)
        {
            _writer.write(0x35);
        }
        else if (instMemI64AsEnum == MemoryInstruction::STORE)
        {
            _writer.write(0x37);
        }
        else if (instMemI64AsEnum == MemoryInstruction::STORE_8)
        {
            _writer.write(0x3C);
        }
        else if (instMemI64AsEnum == MemoryInstruction::STORE_16)
        {
            _writer.write(0x3D);
        }
        else if (instMemI64AsEnum == MemoryInstruction::STORE_32)
        {
            _writer.write(0x3E);
        }

        base()(instMemI64._memarg);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_I32> instMemI32Idx)
    {
        auto& instMemI32 = get(instMemI32Idx);

        auto instMemI32AsEnum = instMemI32._asEnum;

        if (instMemI32AsEnum == MemoryInstruction::LOAD)
        {
            _writer.write(0x28);
        }
        else if (instMemI32AsEnum == MemoryInstruction::LOAD_8_S)
        {
            _writer.write(0x2C);
        }
        else if (instMemI32AsEnum == MemoryInstruction::LOAD_8_U)
        {
            _writer.write(0x2D);
        }
        else if (instMemI32AsEnum == MemoryInstruction::LOAD_16_S)
        {
            _writer.write(0x2E);
        }
        else if (instMemI32AsEnum == MemoryInstruction::LOAD_16_U)
        {
            _writer.write(0x2F);
        }
        else if (instMemI32AsEnum == MemoryInstruction::STORE)
        {
            _writer.write(0x36);
        }
        else if (instMemI32AsEnum == MemoryInstruction::STORE_8)
        {
            _writer.write(0x3A);
        }
        else if (instMemI32AsEnum == MemoryInstruction::STORE_16)
        {
            _writer.write(0x3B);
        }

        base()(instMemI32._memarg);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_F32> instMemF32Idx)
    {
        auto& instMemF32 = get(instMemF32Idx);

        auto instMemF32AsEnum = instMemF32._asEnum;

        if (instMemF32AsEnum == MemoryInstruction::LOAD)
        {
            _writer.write(0x2A);
        }
        else if (instMemF32AsEnum == MemoryInstruction::STORE)
        {
            _writer.write(0x38);
        }

        base()(instMemF32._memarg);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_F64> instMemF64Idx)
    {
        auto& instMemF64 = get(instMemF64Idx);

        auto instMemF64AsEnum = instMemF64._asEnum;

        if (instMemF64AsEnum == MemoryInstruction::LOAD)
        {
            _writer.write(0x2B);
        }
        else if (instMemF64AsEnum == MemoryInstruction::STORE)
        {
            _writer.write(0x39);
        }

        base()(instMemF64._memarg);
    }

    void operator()(node::TIndex<node::Kind::INST_CONST_I32> instConstI32Idx)
    {
        auto& instConstI32 = get(instConstI32Idx);
        auto& relocation   = get(instConstI32._relocation);

        _writer.write(0x41);

        if (IS_OBJECT && relocation._type != RelocationType::NONE)
        {
            relocation._offset = _writer.diff(_writerSection);
        }

        Leb128<IS_OBJECT> valueAsLeb128{instConstI32._value};
        _writer.write(valueAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CONST_I64> instConstI64Idx)
    {
        auto& instConstI64 = get(instConstI64Idx);

        Leb128</*IS_OBJECT=*/false> valueAsLeb128{instConstI64._value};

        _writer.write(0x42);
        _writer.write(valueAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CONST_F32> instConstF32Idx)
    {
        auto& instConstF32 = get(instConstF32Idx);

        _writer.write(0x43);
        _writer.writeF32(instConstF32._value);
    }

    void operator()(node::TIndex<node::Kind::INST_CONST_F64> instConstF64Idx)
    {
        auto& instConstF64 = get(instConstF64Idx);

        _writer.write(0x44);
        _writer.writeF64(instConstF64._value);
    }

    void operator()(node::TIndex<node::Kind::INST_I32> instI32Idx)
    {
        auto& instI32 = get(instI32Idx);

        auto instI32AsEnum = instI32._asEnum;

        if (instI32AsEnum == NumericInstruction::EQZ)
        {
            _writer.write(0x45);
        }
        else if (instI32AsEnum == NumericInstruction::EQ)
        {
            _writer.write(0x46);
        }
        else if (instI32AsEnum == NumericInstruction::NE)
        {
            _writer.write(0x47);
        }
        else if (instI32AsEnum == NumericInstruction::LT_S)
        {
            _writer.write(0x48);
        }
        else if (instI32AsEnum == NumericInstruction::LT_U)
        {
            _writer.write(0x49);
        }
        else if (instI32AsEnum == NumericInstruction::GT_S)
        {
            _writer.write(0x4A);
        }
        else if (instI32AsEnum == NumericInstruction::GT_U)
        {
            _writer.write(0x4B);
        }
        else if (instI32AsEnum == NumericInstruction::LE_S)
        {
            _writer.write(0x4C);
        }
        else if (instI32AsEnum == NumericInstruction::LE_U)
        {
            _writer.write(0x4D);
        }
        else if (instI32AsEnum == NumericInstruction::GE_S)
        {
            _writer.write(0x4E);
        }
        else if (instI32AsEnum == NumericInstruction::GE_U)
        {
            _writer.write(0x4F);
        }
        else if (instI32AsEnum == NumericInstruction::CLZ)
        {
            _writer.write(0x67);
        }
        else if (instI32AsEnum == NumericInstruction::CTZ)
        {
            _writer.write(0x68);
        }
        else if (instI32AsEnum == NumericInstruction::POPCNT)
        {
            _writer.write(0x69);
        }
        else if (instI32AsEnum == NumericInstruction::ADD)
        {
            _writer.write(0x6A);
        }
        else if (instI32AsEnum == NumericInstruction::SUB)
        {
            _writer.write(0x6B);
        }
        else if (instI32AsEnum == NumericInstruction::MUL)
        {
            _writer.write(0x6C);
        }
        else if (instI32AsEnum == NumericInstruction::DIV_S)
        {
            _writer.write(0x6D);
        }
        else if (instI32AsEnum == NumericInstruction::DIV_U)
        {
            _writer.write(0x6E);
        }
        else if (instI32AsEnum == NumericInstruction::REM_S)
        {
            _writer.write(0x6F);
        }
        else if (instI32AsEnum == NumericInstruction::REM_U)
        {
            _writer.write(0x70);
        }
        else if (instI32AsEnum == NumericInstruction::AND)
        {
            _writer.write(0x71);
        }
        else if (instI32AsEnum == NumericInstruction::OR)
        {
            _writer.write(0x72);
        }
        else if (instI32AsEnum == NumericInstruction::XOR)
        {
            _writer.write(0x73);
        }
        else if (instI32AsEnum == NumericInstruction::SHL)
        {
            _writer.write(0x74);
        }
        else if (instI32AsEnum == NumericInstruction::SHR_S)
        {
            _writer.write(0x75);
        }
        else if (instI32AsEnum == NumericInstruction::SHR_U)
        {
            _writer.write(0x76);
        }
        else if (instI32AsEnum == NumericInstruction::ROTL)
        {
            _writer.write(0x77);
        }
        else if (instI32AsEnum == NumericInstruction::ROTR)
        {
            _writer.write(0x78);
        }
        else if (instI32AsEnum == NumericInstruction::WRAP_I64)
        {
            _writer.write(0xA7);
        }
        else if (instI32AsEnum == NumericInstruction::TRUNC_S)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F32>>(instI32._suffix))
            {
                _writer.write(0xA8);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F64>>(instI32._suffix))
            {
                _writer.write(0xAA);
            }
        }
        else if (instI32AsEnum == NumericInstruction::TRUNC_U)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F32>>(instI32._suffix))
            {
                _writer.write(0xA9);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F64>>(instI32._suffix))
            {
                _writer.write(0xAB);
            }
        }
        else if (instI32AsEnum == NumericInstruction::REINTERPRET)
        {
            _writer.write(0xBC);
        }
        else if (instI32AsEnum == NumericInstruction::EXTEND_8_S)
        {
            _writer.write(0xC0);
        }
        else if (instI32AsEnum == NumericInstruction::EXTEND_16_S)
        {
            _writer.write(0xC1);
        }
    }

    void operator()(node::TIndex<node::Kind::INST_I64> instI64Idx)
    {
        auto& instI64 = get(instI64Idx);

        auto instI64AsEnum = instI64._asEnum;

        if (instI64AsEnum == NumericInstruction::EQZ)
        {
            _writer.write(0x50);
        }
        else if (instI64AsEnum == NumericInstruction::EQ)
        {
            _writer.write(0x51);
        }
        else if (instI64AsEnum == NumericInstruction::NE)
        {
            _writer.write(0x52);
        }
        else if (instI64AsEnum == NumericInstruction::LT_S)
        {
            _writer.write(0x53);
        }
        else if (instI64AsEnum == NumericInstruction::LT_U)
        {
            _writer.write(0x54);
        }
        else if (instI64AsEnum == NumericInstruction::GT_S)
        {
            _writer.write(0x55);
        }
        else if (instI64AsEnum == NumericInstruction::GT_U)
        {
            _writer.write(0x56);
        }
        else if (instI64AsEnum == NumericInstruction::LE_S)
        {
            _writer.write(0x57);
        }
        else if (instI64AsEnum == NumericInstruction::LE_U)
        {
            _writer.write(0x58);
        }
        else if (instI64AsEnum == NumericInstruction::GE_S)
        {
            _writer.write(0x59);
        }
        else if (instI64AsEnum == NumericInstruction::GE_U)
        {
            _writer.write(0x5A);
        }
        else if (instI64AsEnum == NumericInstruction::CLZ)
        {
            _writer.write(0x79);
        }
        else if (instI64AsEnum == NumericInstruction::CTZ)
        {
            _writer.write(0x7A);
        }
        else if (instI64AsEnum == NumericInstruction::POPCNT)
        {
            _writer.write(0x7B);
        }
        else if (instI64AsEnum == NumericInstruction::ADD)
        {
            _writer.write(0x7C);
        }
        else if (instI64AsEnum == NumericInstruction::SUB)
        {
            _writer.write(0x7D);
        }
        else if (instI64AsEnum == NumericInstruction::MUL)
        {
            _writer.write(0x7E);
        }
        else if (instI64AsEnum == NumericInstruction::DIV_S)
        {
            _writer.write(0x7F);
        }
        else if (instI64AsEnum == NumericInstruction::DIV_U)
        {
            _writer.write(0x80);
        }
        else if (instI64AsEnum == NumericInstruction::REM_S)
        {
            _writer.write(0x81);
        }
        else if (instI64AsEnum == NumericInstruction::REM_U)
        {
            _writer.write(0x82);
        }
        else if (instI64AsEnum == NumericInstruction::AND)
        {
            _writer.write(0x83);
        }
        else if (instI64AsEnum == NumericInstruction::OR)
        {
            _writer.write(0x84);
        }
        else if (instI64AsEnum == NumericInstruction::XOR)
        {
            _writer.write(0x85);
        }
        else if (instI64AsEnum == NumericInstruction::SHL)
        {
            _writer.write(0x86);
        }
        else if (instI64AsEnum == NumericInstruction::SHR_S)
        {
            _writer.write(0x87);
        }
        else if (instI64AsEnum == NumericInstruction::SHR_U)
        {
            _writer.write(0x88);
        }
        else if (instI64AsEnum == NumericInstruction::ROTL)
        {
            _writer.write(0x89);
        }
        else if (instI64AsEnum == NumericInstruction::ROTR)
        {
            _writer.write(0x8A);
        }
        else if (instI64AsEnum == NumericInstruction::EXTEND_I32_S)
        {
            _writer.write(0xAC);
        }
        else if (instI64AsEnum == NumericInstruction::EXTEND_I32_U)
        {
            _writer.write(0xAD);
        }
        else if (instI64AsEnum == NumericInstruction::TRUNC_S)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F32>>(instI64._suffix))
            {
                _writer.write(0xAE);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F64>>(instI64._suffix))
            {
                _writer.write(0xB0);
            }
        }
        else if (instI64AsEnum == NumericInstruction::TRUNC_U)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F32>>(instI64._suffix))
            {
                _writer.write(0xAF);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_F64>>(instI64._suffix))
            {
                _writer.write(0xB1);
            }
        }
        else if (instI64AsEnum == NumericInstruction::REINTERPRET)
        {
            _writer.write(0xBD);
        }
        else if (instI64AsEnum == NumericInstruction::EXTEND_8_S)
        {
            _writer.write(0xC2);
        }
        else if (instI64AsEnum == NumericInstruction::EXTEND_16_S)
        {
            _writer.write(0xC3);
        }
        else if (instI64AsEnum == NumericInstruction::EXTEND_32_S)
        {
            _writer.write(0xC4);
        }
    }

    void operator()(node::TIndex<node::Kind::INST_F32> instF32Idx)
    {
        auto& instF32 = get(instF32Idx);

        auto instF32AsEnum = instF32._asEnum;

        if (instF32AsEnum == NumericInstruction::EQ)
        {
            _writer.write(0x5B);
        }
        else if (instF32AsEnum == NumericInstruction::NE)
        {
            _writer.write(0x5C);
        }
        else if (instF32AsEnum == NumericInstruction::LT)
        {
            _writer.write(0x5D);
        }
        else if (instF32AsEnum == NumericInstruction::GT)
        {
            _writer.write(0x5E);
        }
        else if (instF32AsEnum == NumericInstruction::LE)
        {
            _writer.write(0x5F);
        }
        else if (instF32AsEnum == NumericInstruction::GE)
        {
            _writer.write(0x60);
        }
        else if (instF32AsEnum == NumericInstruction::ABS)
        {
            _writer.write(0x8B);
        }
        else if (instF32AsEnum == NumericInstruction::NEG)
        {
            _writer.write(0x8C);
        }
        else if (instF32AsEnum == NumericInstruction::CEIL)
        {
            _writer.write(0x8D);
        }
        else if (instF32AsEnum == NumericInstruction::FLOOR)
        {
            _writer.write(0x8E);
        }
        else if (instF32AsEnum == NumericInstruction::TRUNC)
        {
            _writer.write(0x8F);
        }
        else if (instF32AsEnum == NumericInstruction::NEAREST)
        {
            _writer.write(0x90);
        }
        else if (instF32AsEnum == NumericInstruction::SQRT)
        {
            _writer.write(0x91);
        }
        else if (instF32AsEnum == NumericInstruction::ADD)
        {
            _writer.write(0x92);
        }
        else if (instF32AsEnum == NumericInstruction::SUB)
        {
            _writer.write(0x93);
        }
        else if (instF32AsEnum == NumericInstruction::MUL)
        {
            _writer.write(0x94);
        }
        else if (instF32AsEnum == NumericInstruction::DIV)
        {
            _writer.write(0x95);
        }
        else if (instF32AsEnum == NumericInstruction::MIN)
        {
            _writer.write(0x96);
        }
        else if (instF32AsEnum == NumericInstruction::MAX)
        {
            _writer.write(0x97);
        }
        else if (instF32AsEnum == NumericInstruction::COPYSIGN)
        {
            _writer.write(0x98);
        }
        else if (instF32AsEnum == NumericInstruction::CONVERT_S)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I32>>(instF32._suffix))
            {
                _writer.write(0xB2);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I64>>(instF32._suffix))
            {
                _writer.write(0xB4);
            }
        }
        else if (instF32AsEnum == NumericInstruction::CONVERT_U)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I32>>(instF32._suffix))
            {
                _writer.write(0xB3);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I64>>(instF32._suffix))
            {
                _writer.write(0xB5);
            }
        }
        else if (instF32AsEnum == NumericInstruction::DEMOTE_F64)
        {
            _writer.write(0xB6);
        }
        else if (instF32AsEnum == NumericInstruction::REINTERPRET)
        {
            _writer.write(0xBE);
        }
    }

    void operator()(node::TIndex<node::Kind::INST_F64> instF64Idx)
    {
        auto& instF64 = get(instF64Idx);

        auto instF64AsEnum = instF64._asEnum;

        if (instF64AsEnum == NumericInstruction::EQ)
        {
            _writer.write(0x61);
        }
        else if (instF64AsEnum == NumericInstruction::NE)
        {
            _writer.write(0x62);
        }
        else if (instF64AsEnum == NumericInstruction::LT)
        {
            _writer.write(0x63);
        }
        else if (instF64AsEnum == NumericInstruction::GT)
        {
            _writer.write(0x64);
        }
        else if (instF64AsEnum == NumericInstruction::LE)
        {
            _writer.write(0x65);
        }
        else if (instF64AsEnum == NumericInstruction::GE)
        {
            _writer.write(0x66);
        }
        else if (instF64AsEnum == NumericInstruction::ABS)
        {
            _writer.write(0x99);
        }
        else if (instF64AsEnum == NumericInstruction::NEG)
        {
            _writer.write(0x9A);
        }
        else if (instF64AsEnum == NumericInstruction::CEIL)
        {
            _writer.write(0x9B);
        }
        else if (instF64AsEnum == NumericInstruction::FLOOR)
        {
            _writer.write(0x9C);
        }
        else if (instF64AsEnum == NumericInstruction::TRUNC)
        {
            _writer.write(0x9D);
        }
        else if (instF64AsEnum == NumericInstruction::NEAREST)
        {
            _writer.write(0x9E);
        }
        else if (instF64AsEnum == NumericInstruction::SQRT)
        {
            _writer.write(0x9F);
        }
        else if (instF64AsEnum == NumericInstruction::ADD)
        {
            _writer.write(0xA0);
        }
        else if (instF64AsEnum == NumericInstruction::SUB)
        {
            _writer.write(0xA1);
        }
        else if (instF64AsEnum == NumericInstruction::MUL)
        {
            _writer.write(0xA2);
        }
        else if (instF64AsEnum == NumericInstruction::DIV)
        {
            _writer.write(0xA3);
        }
        else if (instF64AsEnum == NumericInstruction::MIN)
        {
            _writer.write(0xA4);
        }
        else if (instF64AsEnum == NumericInstruction::MAX)
        {
            _writer.write(0xA5);
        }
        else if (instF64AsEnum == NumericInstruction::COPYSIGN)
        {
            _writer.write(0xA6);
        }
        else if (instF64AsEnum == NumericInstruction::CONVERT_S)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I32>>(instF64._suffix))
            {
                _writer.write(0xB7);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I64>>(instF64._suffix))
            {
                _writer.write(0xB9);
            }
        }
        else if (instF64AsEnum == NumericInstruction::CONVERT_U)
        {
            if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I32>>(instF64._suffix))
            {
                _writer.write(0xB8);
            }
            else if (std::holds_alternative<node::TIndex<node::Kind::TYPE_I64>>(instF64._suffix))
            {
                _writer.write(0xBA);
            }
        }
        else if (instF64AsEnum == NumericInstruction::PROMOTE_F32)
        {
            _writer.write(0xBB);
        }
        else if (instF64AsEnum == NumericInstruction::REINTERPRET)
        {
            _writer.write(0xBF);
        }
    }

    void operator()(node::TIndex<node::Kind::INST_UNREACHABLE >) { _writer.write(0x00); }
    void operator()(node::TIndex<node::Kind::INST_NOP         >) { _writer.write(0x01); }
    void operator()(node::TIndex<node::Kind::INST_RETURN      >) { _writer.write(0x0F); }
    void operator()(node::TIndex<node::Kind::INST_REF_IS_NULL >) { _writer.write(0xD1); }
    void operator()(node::TIndex<node::Kind::INST_DROP        >) { _writer.write(0x1A); }

    void operator()(node::TIndex<node::Kind::INSTRUCTION> instructionIdx)
    {
        auto& instruction = get(instructionIdx);

        base()(instruction._asVariant);
    }

    void operator()(node::TIndex<node::Kind::DATA> dataIdx)
    {
        auto& data = get(dataIdx);

        // 1. Compute and write the flag

        uint8_t flags = 0;

        flags |= 0b001 * std::holds_alternative<node::TIndex<node::Kind::DATA_PASSIVE>>(data._mode);
        flags |= 0b010 * (!flags && get(std::get<node::TIndex<node::Kind::DATA_ACTIVE>>(data._mode))._memIdx != 0);

        _writer.write(flags);

        // 2. Write the data according to its flags value

        if (flags == 0x00)
        {
            base()(get(std::get<node::TIndex<node::Kind::DATA_ACTIVE>>(data._mode))._offset);
            _writer.write(0x0B);
            base()(data._init);
        }
        else if (flags == 0x01)
        {
            base()(data._init);
        }
        else if (flags == 0x02)
        {
            auto& mode = get(std::get<node::TIndex<node::Kind::DATA_ACTIVE>>(data._mode));

            Leb128</*IS_OBJECT=*/false> memIdxAsLeb128{mode._memIdx};

            _writer.write(memIdxAsLeb128);

            emitListWithSentinel(mode._offset);

            base()(data._init);
        }
    }

    void operator()(node::TIndex<node::Kind::SYMBOL_OBJECT> symbolObjectIdx)
    {
        auto& symbolObject = get(symbolObjectIdx);

        Leb128</*IS_OBJECT=*/false> importIndexAsLeb128{node::v_index::makeId(_nodePool, symbolObject._index)};
        _writer.write(importIndexAsLeb128);

        base()(symbolObject._name);
    }

    void operator()(node::TIndex<node::Kind::SYMBOL_DATA> symbolDataIdx)
    {
        auto& symbolData = get(symbolDataIdx);
        base()(symbolData._name);

        if (_isCurrentSymbolDefined)
        {
            Leb128</*IS_OBJECT=*/false> indexAsLeb128{symbolData._index};
            _writer.write(indexAsLeb128);
            Leb128</*IS_OBJECT=*/false> offsetAsLeb128{symbolData._offset};
            _writer.write(offsetAsLeb128);
            Leb128</*IS_OBJECT=*/false> sizeAsLeb128{symbolData._size};
            _writer.write(sizeAsLeb128);
        }
    }

    void operator()(node::TIndex<node::Kind::SYMBOL> symbolIdx)
    {
        auto& symbol = get(symbolIdx);

        _writer.write(symbol._kind._asInt);

        Leb128</*IS_OBJECT=*/false> flagsAsLeb128{symbol._flags};
        _writer.write(flagsAsLeb128);

        _isCurrentSymbolDefined = symbol._flags ^ SymbolFlag::UNDEFINED;
        base()(symbol._asVariant);
    }

    void operator()(node::TIndex<node::Kind::RELOCATION>& relocationIdx)
    {
        auto& relocation = get(relocationIdx);

        _writer.write(relocation._type._asInt);

        Leb128</*IS_OBJECT=*/false> offsetAsLeb128{relocation._offset};
        Leb128</*IS_OBJECT=*/false>  indexAsLeb128{relocation._index};

        _writer.write(offsetAsLeb128);
        _writer.write( indexAsLeb128);

        if (relocation._type == RelocationType::FUNCTION_OFFSET_I32 ||
            relocation._type == RelocationType::SECTION_OFFSET_I32  ||
            relocation._type == RelocationType::MEMORY_ADDR_SLEB    ||
            relocation._type == RelocationType::MEMORY_ADDR_LEB     ||
            relocation._type == RelocationType::MEMORY_ADDR_I32)
        {
            Leb128</*IS_OBJECT=*/false> addendAsLeb128{relocation._addend};
            _writer.write(addendAsLeb128);
        }
    }

    void operator()(node::TIndex<node::Kind::MODULE> moduleIdx)
    {
        _writer.write(K_MAGIC   , sizeof(K_MAGIC   ));
        _writer.write(K_VERSION , sizeof(K_VERSION ));

        uint8_t sectionCount = 0;

        auto& module = get(moduleIdx);

        std::decay_t<decltype(*_relocOrderedCode)> relocOrderedCode{module._relocSizeCode};
        std::decay_t<decltype(*_relocOrderedData)> relocOrderedData{module._relocSizeCode};
        _relocOrderedCode = &relocOrderedCode;
        _relocOrderedData = &relocOrderedData;
        _relocOrderedCode->_size = 0;
        _relocOrderedData->_size = 0;

        if (module._types._size)
        {
            _writer.write(SectionId::TYPE);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            Leb128</*IS_OBJECT=*/false> rangeSizeAsLeb128{module._types._size};
            _writer.write(rangeSizeAsLeb128);

            base()(module._types);
        }

        if (module._imports._size)
        {
            _writer.write(SectionId::IMPORT);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            emitRangeWithSize(module._imports);
        }

        if (module._funcs._size)
        {
            _writer.write(SectionId::FUNCTION);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            Leb128</*IS_OBJECT=*/false> rangeSizeAsLeb128{module._funcs._size};
            _writer.write(rangeSizeAsLeb128);

            for (uint32_t i = 0; i < module._funcs._size; i++)
            {
                Leb128<IS_OBJECT> typeIdxAsLeb128{get(get(module._funcs[i])._type)._id};
                _writer.write(typeIdxAsLeb128);
            }
        }

        if (module._tables._size)
        {
            _writer.write(SectionId::TABLE);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            emitRangeWithSize(module._tables);
        }

        if (module._mems._size)
        {
            _writer.write(SectionId::MEMORY);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            emitRangeWithSize(module._mems);
        }

        if (module._globalConsts ._size ||
            module._globalVars   ._size)
        {
            _writer.write(SectionId::GLOBAL);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            emitRangeWithSize(module._globalConsts);
            emitRangeWithSize(module._globalVars);
        }

        if (module._exports._size)
        {
            _writer.write(SectionId::EXPORT);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            emitRangeWithSize(module._exports);
        }

        if (module._startOpt)
        {
            _writer.write(SectionId::START);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            base()(module._startOpt.value());
        }

        if (module._datas._size)
        {
            _writer.write(SectionId::DATA_COUNT);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            Leb128</*IS_OBJECT=*/false> datasSizeAsLeb128{module._datas._size};
            _writer.write(datasSizeAsLeb128);
        }

        if (IS_OBJECT || module._funcs._size)
        {
            _writer.write(SectionId::CODE);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            _writerSection = _writer.fork();

            Leb128</*IS_OBJECT=*/false> rangeSizeAsLeb128{module._funcs._size};
            _writer.write(rangeSizeAsLeb128);

            for (uint32_t i = 0; i < module._funcs._size; i++)
            {
                TFixUpSize<Writer> _fixupFuncSize{_writer};
                base()(module._funcs[i]);
            }
        }

        if (IS_OBJECT || module._datas._size)
        {
            _writer.write(SectionId::DATA);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            _writerSection = _writer.fork();
            emitRangeWithSize(module._datas);
        }

        if (IS_OBJECT)
        {
            _writer.write(SectionId::CUSTOM);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            _writer.write(Leb128</*IS_OBJECT=*/false>{sizeof("linking") - 1});
            _writer.write(reinterpret_cast<const uint8_t*>("linking"), sizeof("linking") - 1);

            Leb128</*IS_OBJECT=*/false> versionAsLeb128{K_LINK_VERSION};
            _writer.write(versionAsLeb128);
            _writer.write(K_SYMBOL_TABLE);

            TFixUpSize<Writer> _fixupSizeSymbols{_writer};
            emitRangeWithSize(module._symbols);
        }

        if (IS_OBJECT)
        {
            _writer.write(SectionId::CUSTOM);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            _writer.write(Leb128</*IS_OBJECT=*/false>{sizeof("reloc.CODE") - 1});
            _writer.write(reinterpret_cast<const uint8_t*>("reloc.CODE"), sizeof("reloc.CODE") - 1);

            Leb128</*IS_OBJECT=*/false> sectionAsLeb128{sectionCount - K_RELOC_SECTION_OFFSET};
            _writer.write(sectionAsLeb128);

            Leb128</*IS_OBJECT=*/false> relocSizeCode128{module._relocSizeCode};
            _writer.write(relocSizeCode128);

            for (uint32_t i = 0; i < _relocOrderedCode->_size; i++)
            {
                base()((*_relocOrderedCode)[i]);
            }
        }

        if (IS_OBJECT)
        {
            _writer.write(SectionId::CUSTOM);
            TFixUpSize<Writer> _fixupSize{_writer};
            sectionCount++;

            _writer.write(Leb128</*IS_OBJECT=*/false>{sizeof("reloc.DATA") - 1});
            _writer.write(reinterpret_cast<const uint8_t*>("reloc.DATA"), sizeof("reloc.DATA") - 1);

            Leb128</*IS_OBJECT=*/false> sectionAsLeb128{sectionCount - K_RELOC_SECTION_OFFSET};
            _writer.write(sectionAsLeb128);

            Leb128</*IS_OBJECT=*/false> relocSizeData128{module._relocSizeData};
            _writer.write(relocSizeData128);

            for (uint32_t i = 0; i < _relocOrderedData->_size; i++)
            {
                base()((*_relocOrderedData)[i]);
            }
        }
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void emitRangeWithSize(node::TRange<KIND>& range)
    {
        Leb128</*IS_OBJECT=*/false> rangeSizeAsLeb128{range._size};
        _writer.write(rangeSizeAsLeb128);

        base()(range);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void emitListWithSentinel(node::TList<KIND>& list)
    {
        base()(list);

        _writer.write(0x0B);
    }

    DMIT_COM_TREE_VISITOR_SIMPLE(node, Kind);

    Writer& _writer;
    Writer  _writerSection;
    com::TStorage<node::TIndex<node::Kind::RELOCATION>>* _relocOrderedCode = nullptr;
    com::TStorage<node::TIndex<node::Kind::RELOCATION>>* _relocOrderedData = nullptr;

    TImportDescriptorEmitter <NodePool, Writer> _importDescriptorEmitter;
    TExportDescriptorEmitter <NodePool, Writer> _exportDescriptorEmitter;
    TBlockTypeEmitter        <NodePool, Writer> _blockTypeEmitter;

    bool _isCurrentSymbolDefined;

    static constexpr uint8_t  K_MAGIC   []                = {0x00, 0x61, 0x73, 0x6D};
    static constexpr uint8_t  K_VERSION []                = {0x01, 0x00, 0x00, 0x00};
    static constexpr uint8_t  K_SYMBOL_TABLE              = 8;
    static constexpr uint32_t K_LINK_VERSION              = 2;
    static constexpr uint32_t K_RELOC_SECTION_OFFSET      = 4;
};

template <bool IS_OBJECT, class NodePool, class Writer>
void emit(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool, Writer& writer)
{
    TEmitter<IS_OBJECT, NodePool, Writer> emitter{nodePool, writer};

    emitter.base()(module);
}

template <bool IS_OBJECT, class NodePool>
void emit(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool, uint8_t* const buffer)
{
    if (dmit::com::Endianness{} == dmit::com::Endianness::LITTLE)
    {
        writer::TScribe<dmit::com::Endianness::LITTLE> scribe{buffer};
        emit<IS_OBJECT, NodePool, decltype(scribe)>(module, nodePool, scribe);
    }
    else if (dmit::com::Endianness{} == dmit::com::Endianness::BIG)
    {
        writer::TScribe<dmit::com::Endianness::BIG> scribe{buffer};
        emit<IS_OBJECT, NodePool, decltype(scribe)>(module, nodePool, scribe);
    }
}

template <bool IS_OBJECT, class NodePool>
uint32_t emitSize(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool)
{
    writer::Bematist bematist;

    emit<IS_OBJECT, NodePool>(module, nodePool, bematist);

    return bematist._size;
}

template <class NodePool>
uint32_t emitSizeNoObject(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool)
{
    return emitSize<false /*IS_OBJECT*/, NodePool>(module, nodePool);
}

template <class NodePool>
uint32_t emitSizeObject(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool)
{
    return emitSize<true /*IS_OBJECT*/, NodePool>(module, nodePool);
}

template <class NodePool>
void emitNoObject(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool, uint8_t* const buffer)
{
    emit<false /*IS_OBJECT*/, NodePool>(module, nodePool, buffer);
}

template <class NodePool>
void emitObject(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool, uint8_t* const buffer)
{
    emit<true /*IS_OBJECT*/, NodePool>(module, nodePool, buffer);
}

} // namespace dmit::wsm
