#pragma once

#include "dmit/wsm/emit_block_type.hpp"
#include "dmit/wsm/emit_section.hpp"
#include "dmit/wsm/emit_port.hpp"
#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/tree_visitor.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/enum.hpp"

#include <optional>
#include <cstdint>
#include <variant>

namespace dmit::wsm
{

template <class Derived, class NodePool>
using TBaseVisitor = typename com::tree::TTMetaVisitor<node::Kind,
                                                       TNode,
                                                       NodePool>::template TVisitor<Derived>;
template <class NodePool, class Writer>
struct TEmitter : TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>
{
    using TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>::_nodePool;
    using TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>::base;
    using TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>::get;

    TEmitter(NodePool& nodePool, Writer& writer) :
        TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>{nodePool},
        _writer{writer},
        _sectionId{SectionId::CUSTOM},
        _importDescriptorEmitter{nodePool, writer},
        _exportDescriptorEmitter{nodePool, writer},
        _blockTypeEmitter{nodePool, writer}
    {}

    template <class LimitsIdx>
    void writeLimits(LimitsIdx limitsIdx)
    {
        auto& limits = _nodePool.get(limitsIdx);

        Leb128 limitsMinAsLeb128{limits._min};

        if (limits._maxOpt)
        {
            Leb128 limitsMaxAsLeb128{limits._maxOpt.value()};

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

    void operator()(node::TIndex<node::Kind::FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        if (_sectionId == SectionId::FUNCTION)
        {
            Leb128 typeIdxAsLeb128{function._typeIdx};

            _writer.write(typeIdxAsLeb128);
        }
        else if (_sectionId == SectionId::CODE)
        {
            Leb128 sizeLocalsAsLeb128{function._locals._size};

            _writer.write(sizeLocalsAsLeb128);

            for (uint32_t i = 0; i < function._locals._size; i++)
            {
                _writer.write(0x01);
                base()(function._locals[i]);
            }

            emitRangeWithSentinel(function._body);
        }
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

        Leb128 funcIdxAsLeb128{start._funcIdx};

        _writer.write(funcIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::LABEL> labelIdx)
    {
        auto& label = get(labelIdx);

        Leb128 labelIdxAsLeb128{label._labelIdx};

        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::MEMARG> memargIdx)
    {
        auto& memarg = get(memargIdx);

        Leb128  alignAsLeb128{memarg._align  };
        Leb128 offsetAsLeb128{memarg._offset };

        _writer.write( alignAsLeb128);
        _writer.write(offsetAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_BLOCK> instBlockIdx)
    {
        auto& instBlock = get(instBlockIdx);

        _writer.write(0x02);

        std::visit(_blockTypeEmitter, instBlock._type);

        emitRangeWithSentinel(instBlock._instructions);
    }

    void operator()(node::TIndex<node::Kind::INST_LOOP> instLoopIdx)
    {
        auto& instLoop = get(instLoopIdx);

        _writer.write(0x03);

        std::visit(_blockTypeEmitter, instLoop._type);

        emitRangeWithSentinel(instLoop._instructions);
    }

    void operator()(node::TIndex<node::Kind::INST_IF> instIfIdx)
    {
        auto& instIf = get(instIfIdx);

        _writer.write(0x04);

        TBlockTypeEmitter<NodePool, Writer> blockTypeEmitter{_nodePool, _writer};

        std::visit(blockTypeEmitter, instIf._type);

        for (uint32_t i = 0; i < instIf._then._size; i++)
        {
            base()(instIf._then[i]);
        }

        if (instIf._else._size)
        {
            _writer.write(0x05);

            for (uint32_t i = 0; i < instIf._else._size; i++)
            {
                base()(instIf._else[i]);
            }
        }

        _writer.write(0x0B);
    }

    void operator()(node::TIndex<node::Kind::INST_BR> instBrIdx)
    {
        auto& instBr = get(instBrIdx);

        Leb128 labelIdxAsLeb128{instBr._labelIdx};

        _writer.write(0x0C);
        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_BR_IF> instBrIfIdx)
    {
        auto& instBrIf = get(instBrIfIdx);

        Leb128 labelIdxAsLeb128{instBrIf._labelIdx};

        _writer.write(0x0D);
        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_BR_TABLE> instBrTableIdx)
    {
        auto& instBrTable = get(instBrTableIdx);

        emitRangeWithSize(instBrTable._labels);

        Leb128 labelIdxAsLeb128{instBrTable._labelIdx};

        _writer.write(0x0E);
        _writer.write(labelIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CALL> instCallIdx)
    {
        auto& instCall = get(instCallIdx);

        Leb128 funcIdxAsLeb128{instCall._funcIdx};

        _writer.write(0x10);
        _writer.write(funcIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CALL_INDIRECT> instCallIndirectIdx)
    {
        auto& instCallIndirect = get(instCallIndirectIdx);

        Leb128  typeIdxAsLeb128{instCallIndirect._typeIdx  };
        Leb128 tableIdxAsLeb128{instCallIndirect._tableIdx };

        _writer.write(0x11);
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

        Leb128 funcIdxAsLeb128{instRefFunc._funcIdx};

        _writer.write(0xD2);
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

        Leb128 localIdxAsLeb128{instLocalGet._localIdx};

        _writer.write(0x20);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_LOCAL_SET> instLocalSetIdx)
    {
        auto& instLocalSet = get(instLocalSetIdx);

        Leb128 localIdxAsLeb128{instLocalSet._localIdx};

        _writer.write(0x21);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_LOCAL_TEE> instLocalTeeIdx)
    {
        auto& instLocalTee = get(instLocalTeeIdx);

        Leb128 localIdxAsLeb128{instLocalTee._localIdx};

        _writer.write(0x22);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_GLOBAL_GET> instGlobalGetIdx)
    {
        auto& instGlobalGet = get(instGlobalGetIdx);

        Leb128 localIdxAsLeb128{instGlobalGet._globalIdx};

        _writer.write(0x23);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_GLOBAL_SET> instGlobalSetIdx)
    {
        auto& instGlobalSet = get(instGlobalSetIdx);

        Leb128 localIdxAsLeb128{instGlobalSet._globalIdx};

        _writer.write(0x24);
        _writer.write(localIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_GET> instTableGetIdx)
    {
        auto& instTableGet = get(instTableGetIdx);

        Leb128 tableIdxAsLeb128{instTableGet._tableIdx};

        _writer.write(0x25);
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_SET> instTableSetIdx)
    {
        auto& instTableSet = get(instTableSetIdx);

        Leb128 tableIdxAsLeb128{instTableSet._tableIdx};

        _writer.write(0x26);
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_INIT> instTableInitIdx)
    {
        auto& instTableInit = get(instTableInitIdx);

        Leb128 tableIdxAsLeb128{instTableInit._tableIdx };
        Leb128  elemIdxAsLeb128{instTableInit._elemIdx  };

        _writer.write(0xFC);
        _writer.write(Leb128{12u});
        _writer.write( elemIdxAsLeb128);
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_ELEM_DROP> instElemDropIdx)
    {
        auto& instElemDrop = get(instElemDropIdx);

        Leb128  elemIdxAsLeb128{instElemDrop._elemIdx};

        _writer.write(0xFC);
        _writer.write(Leb128{13u});
        _writer.write(elemIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_COPY> instTableCopyIdx)
    {
        auto& instTableCopy = get(instTableCopyIdx);

        Leb128 srceIdxAsLeb128{instTableCopy._srceIdx };
        Leb128 destIdxAsLeb128{instTableCopy._destIdx };

        _writer.write(0xFC);
        _writer.write(Leb128{14u});
        _writer.write(srceIdxAsLeb128);
        _writer.write(destIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_GROW> instTableGrowIdx)
    {
        auto& instTableGrow = get(instTableGrowIdx);

        Leb128 tableIdxAsLeb128{instTableGrow._tableIdx};

        _writer.write(0xFC);
        _writer.write(Leb128{15u});
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_SIZE> instTableSizeIdx)
    {
        auto& instTableSize = get(instTableSizeIdx);

        Leb128 tableIdxAsLeb128{instTableSize._tableIdx};

        _writer.write(0xFC);
        _writer.write(Leb128{16u});
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_FILL> instTableFillIdx)
    {
        auto& instTableFill = get(instTableFillIdx);

        Leb128 tableIdxAsLeb128{instTableFill._tableIdx};

        _writer.write(0xFC);
        _writer.write(Leb128{17u});
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

    void operator()(node::TIndex<node::Kind::INST_MEM_INIT> instMemInitIdx)
    {
        auto& instMemInit = get(instMemInitIdx);

        Leb128 dataIdxAsLeb128{instMemInit._dataIdx};

        _writer.write(0xFC);
        _writer.write(Leb128{8u});
        _writer.write(dataIdxAsLeb128);
        _writer.write(0x00);
    }

    void operator()(node::TIndex<node::Kind::INST_DATA_DROP> instDataDropIdx)
    {
        auto& instDataDrop = get(instDataDropIdx);

        Leb128 dataIdxAsLeb128{instDataDrop._dataIdx};

        _writer.write(0xFC);
        _writer.write(Leb128{9u});
        _writer.write(dataIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_COPY>)
    {
        _writer.write(0xFC);
        _writer.write(Leb128{10u});
        _writer.write(0x00);
        _writer.write(0x00);
    }

    void operator()(node::TIndex<node::Kind::INST_MEM_FILL>)
    {
        _writer.write(0xFC);
        _writer.write(Leb128{11u});
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

        Leb128 valueAsLeb128{static_cast<std::int64_t>(instConstI32._value)};

        _writer.write(0x41);
        _writer.write(valueAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CONST_I64> instConstI64Idx)
    {
        auto& instConstI64 = get(instConstI64Idx);

        Leb128 valueAsLeb128{instConstI64._value};

        _writer.write(0x41);
        _writer.write(valueAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_CONST_F32> instConstF32Idx)
    {
        auto& instConstF32 = get(instConstF32Idx);

        _writer.write(0x41);
        _writer.writeF32(instConstF32._value);
    }

    void operator()(node::TIndex<node::Kind::INST_CONST_F64> instConstF64Idx)
    {
        auto& instConstF64 = get(instConstF64Idx);

        _writer.write(0x41);
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

    void operator()(node::TIndex<node::Kind::EXPRESSION> expressionIdx)
    {
        auto& expression = get(expressionIdx);

        emitRangeWithSentinel(expression._instructions);
    }

    bool isFuncRef(node::TIndex<node::Kind::EXPRESSION> expressionIdx)
    {
        auto& expression = get(expressionIdx);

        return std::holds_alternative<node::TIndex<node::Kind::INST_REF_FUNC>>(get(expression._instructions[0])._asVariant);
    }

    bool isOnlyFuncRefs(const node::TRange<node::Kind::EXPRESSION>& expressions)
    {
        for (uint32_t i = 0; i < expressions._size; i++)
        {
            if (!isFuncRef(expressions[i]))
            {
                return false;
            }
        }

        return true;
    }

    void operator()(node::TIndex<node::Kind::ELEMENT> elementIdx)
    {
        auto& element = get(elementIdx);

        // 1. Compute and write the flag

        uint8_t flags = 0;

        flags |= 0b001 * std::holds_alternative<node::TIndex<node::Kind::ELEM_PASSIVE     >>(element._mode);
        flags |= 0b011 * std::holds_alternative<node::TIndex<node::Kind::ELEM_DECLARATIVE >>(element._mode);
        flags |= 0b010 * (!flags && get(std::get<node::TIndex<node::Kind::ELEM_ACTIVE>>(element._mode))._tableIdx != 0);
        flags |= 0b100 * !isOnlyFuncRefs(element._init);

        _writer.write(flags);

        // 2. Create small helper to write the list of function ids

        auto writeFuncIds = [this, &element]()
        {
            Leb128 elemInitSizeAsLeb128{element._init._size};

            _writer.write(elemInitSizeAsLeb128);

            for (uint32_t i = 0; i < element._init._size; i++)
            {
                Leb128 funcIdxAsLeb128
                {
                    get(std::
                        get<node::TIndex<node::Kind::INST_REF_FUNC>>(
                            get(
                                get(element._init[i])._instructions[0])._asVariant))._funcIdx
                };

                _writer.write(funcIdxAsLeb128);
            }
        };

        auto writeOffset = [this, &element]()
        {
            auto& offset = get(std::get<node::TIndex<node::Kind::ELEM_ACTIVE>>(element._mode))._offset;

            emitRangeWithSentinel(offset);
        };

        // 3. Write the element segment according to its flags value

        if (flags == 0x00)
        {
            writeOffset();
            writeFuncIds();
        }
        else if (flags == 0x01)
        {
            _writer.write(0x00);
            writeFuncIds();
        }
        else if (flags == 0x02)
        {
            auto& mode = get(std::get<node::TIndex<node::Kind::ELEM_ACTIVE>>(element._mode));

            Leb128 tableIdxAsLeb128{mode._tableIdx};

            _writer.write(tableIdxAsLeb128);

            writeOffset();

            _writer.write(0x00);
            writeFuncIds();
        }
        else if (flags == 0x03)
        {
            _writer.write(0x00);
            writeFuncIds();
        }
        else if (flags == 0x04)
        {
            writeOffset();
            base()(element._init);
        }
        else if (flags == 0x05)
        {
            base()(element._type);
            base()(element._init);
        }
        else if (flags == 0x06)
        {
            auto& mode = get(std::get<node::TIndex<node::Kind::ELEM_ACTIVE>>(element._mode));

            Leb128 tableIdxAsLeb128{mode._tableIdx};

            _writer.write(tableIdxAsLeb128);

            writeOffset();

            base()(element._type);
            base()(element._init);
        }
        else if (flags == 0x07)
        {
            base()(element._type);
            base()(element._init);
        }
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

            Leb128 memIdxAsLeb128{mode._memIdx};

            _writer.write(memIdxAsLeb128);

            emitRangeWithSentinel(mode._offset);

            base()(data._init);
        }
    }

    void fixupType()
    {
        _writer.write(0x60);
        _writer.write(0x00);
        _writer.write(0x00);
    }

    void fixupFunction()
    {
        if (_sectionId == SectionId::FUNCTION)
        {
            _writer.write(Leb128{0u});
        }
        else if (_sectionId == SectionId::CODE)
        {
            for (uint32_t i = 0; i < K_LEB128_MAX_SIZE; i++)
            {
                _writer.write(0x00);
            }

            _writer.write(0x00); // future code size
            _writer.write(0x00); // empty locals
            _writer.write(0x00); // future end byte
        }
    }

    void fixupFunction(Writer fork)
    {
        Leb128 codeSize{_writer.diff(fork) - K_LEB128_MAX_SIZE - 1 - 1 - 1};

        uint32_t fixupSize = K_LEB128_MAX_SIZE + 1 + 1 - codeSize._size;

        fork.write(Leb128{fixupSize});
        fork.skip(fixupSize - 1);
        fork.write(0x0B);
        fork.write(codeSize);
    }

    void operator()(node::TIndex<node::Kind::MODULE> moduleIdx)
    {
        _writer.write(K_MAGIC   , sizeof(K_MAGIC   ));
        _writer.write(K_VERSION , sizeof(K_VERSION ));

        auto& module = get(moduleIdx);

        if (module._types._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::TYPE, _writer);
            _sectionId = SectionId::TYPE;

            Leb128 rangeSizeAsLeb128{module._types._size + 1};

            _writer.write(rangeSizeAsLeb128);

            fixupType();

            for (uint32_t i = 0; i < module._types._size; i++)
            {
                base()(module._types[i]);
            }
        }

        if (module._imports._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::IMPORT, _writer);
            _sectionId = SectionId::IMPORT;
            emitRangeWithSize(module._imports);
        }

        if (module._funcs._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::FUNCTION, _writer);
            _sectionId = SectionId::FUNCTION;

            Leb128 rangeSizeAsLeb128{module._funcs._size << 1};

            _writer.write(rangeSizeAsLeb128);

            for (uint32_t i = 0; i < module._funcs._size; i++)
            {
                fixupFunction();
                base()(module._funcs[i]);
            }
        }

        if (module._tables._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::TABLE, _writer);
            _sectionId = SectionId::TABLE;
            emitRangeWithSize(module._tables);
        }

        if (module._mems._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::MEMORY, _writer);
            _sectionId = SectionId::MEMORY;
            emitRangeWithSize(module._mems);
        }

        if(module._globalConsts ._size ||
           module._globalVars   ._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::GLOBAL, _writer);
            _sectionId = SectionId::GLOBAL;
            emitRangeWithSize(module._globalConsts);
            emitRangeWithSize(module._globalVars);
        }

        if(module._exports._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::EXPORT, _writer);
            _sectionId = SectionId::EXPORT;
            emitRangeWithSize(module._exports);
        }

        if (module._startOpt)
        {
            TFixUpSection<Writer> fixupSection(SectionId::START, _writer);
            _sectionId = SectionId::START;
            base()(module._startOpt.value());
        }

        if (module._elems._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::ELEMENT, _writer);
            _sectionId = SectionId::ELEMENT;
            emitRangeWithSize(module._elems);
        }

        if (module._datas._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::DATA_COUNT, _writer);
            _sectionId = SectionId::DATA_COUNT;

            Leb128 datasSizeAsLeb128{module._datas._size};

            _writer.write(datasSizeAsLeb128);
        }

        if (module._funcs._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::CODE, _writer);
            _sectionId = SectionId::CODE;

            Leb128 rangeSizeAsLeb128{module._funcs._size << 1};

            _writer.write(rangeSizeAsLeb128);

            for (uint32_t i = 0; i < module._funcs._size; i++)
            {
                auto fork = _writer.fork();

                fixupFunction();
                base()(module._funcs[i]);
                fixupFunction(fork);
            }
        }

        if (module._datas._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::DATA, _writer);
            _sectionId = SectionId::DATA;
            emitRangeWithSize(module._datas);
        }
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void emitRangeWithSize(node::TRange<KIND>& range)
    {
        Leb128 rangeSizeAsLeb128{range._size};

        _writer.write(rangeSizeAsLeb128);

        base()(range);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void emitRangeWithSentinel(node::TRange<KIND>& range)
    {
        base()(range);

        _writer.write(0x0B);
    }

    DMIT_COM_TREE_VISITOR_SIMPLE(node, Kind);

    Writer& _writer;
    SectionId _sectionId;

    TImportDescriptorEmitter <NodePool, Writer> _importDescriptorEmitter;
    TExportDescriptorEmitter <NodePool, Writer> _exportDescriptorEmitter;
    TBlockTypeEmitter        <NodePool, Writer> _blockTypeEmitter;

    static constexpr uint8_t K_MAGIC   [] = {0x00, 0x61, 0x73, 0x6D};
    static constexpr uint8_t K_VERSION [] = {0x01, 0x00, 0x00, 0x00};

};

template <class NodePool, class Writer>
void emit(node::TIndex<node::Kind::MODULE> module, NodePool& nodePool, Writer& writer)
{
    TEmitter<NodePool, Writer> emitter{nodePool, writer};

    emitter.base()(module);
}

} // namespace dmit::wsm