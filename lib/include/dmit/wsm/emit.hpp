#pragma once

#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/tree_visitor.hpp"
#include "dmit/com/assert.hpp"
#include "dmit/com/enum.hpp"

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

template <class Writer>
struct TFixUpSection
{
    TFixUpSection(const SectionId sectionId, Writer& writer) :
        _sectionId{sectionId},
        _writer{writer}
    {
        writer.write(SectionId::CUSTOM);

        _fork = writer.fork();

        uint8_t nullBytes[K_LEB128_MAX_SIZE + 1] = {0};

        writer.write(nullBytes, sizeof(nullBytes));
    }

    ~TFixUpSection()
    {
        uint32_t diff = _writer.diff(_fork) - K_LEB128_MAX_SIZE - 1;

        Leb128 diffAsLeb128{diff};

        const auto customSectionSize = K_LEB128_MAX_SIZE -1 - diffAsLeb128._size;

        _fork.write (customSectionSize);
        _fork.skip  (customSectionSize);
        _fork.write (_sectionId._asInt);

        _fork.write(diffAsLeb128);
    }

    const SectionId _sectionId;
    const Writer&   _writer;
          Writer    _fork;
};

template <class NodePool, class Writer>
struct TExportDescriptorEmmiter
{
    TExportDescriptorEmmiter(NodePool& nodePool,
                             Writer& writer) :
        _nodePool{nodePool},
        _writer{writer}
    {}

    void operator()(node::TIndex<node::Kind::INST_GLOBAL_GET> instGlobalGetIdx)
    {
        auto& instGlobalGet = _nodePool.get(instGlobalGetIdx);

        Leb128 globalIdxAsLeb128{instGlobalGet._globalIdx};

        _writer.write(0x03);
        _writer.write(globalIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_TABLE_GET> instTableGetIdx)
    {
        auto& instTableGet = _nodePool.get(instTableGetIdx);

        Leb128 tableIdxAsLeb128{instTableGet._tableIdx};

        _writer.write(0x01);
        _writer.write(tableIdxAsLeb128);
    }

    void operator()(node::TIndex<node::Kind::INST_REF_FUNC> instRefFuncIdx)
    {
        auto& instRefFunc = _nodePool.get(instRefFuncIdx);

        Leb128 funcIdxAsLeb128{instRefFunc._funcIdx};

        _writer.write(0x00);
        _writer.write(funcIdxAsLeb128);
    }

    void operator()(uint32_t memIdx)
    {
        Leb128 memIdxAsLeb128{memIdx};

        _writer.write(0x02);
        _writer.write(memIdxAsLeb128);
    }

    NodePool & _nodePool;
    Writer   & _writer;
};

template <class NodePool, class Writer>
struct TImportDescriptorEmmiter
{
    TImportDescriptorEmmiter(NodePool& nodePool,
                             Writer& writer) :
        _nodePool{nodePool},
        _writer{writer}
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

    template <class... Types>
    void operator()(std::variant<Types...>& variant)
    {
        std::visit(*this, variant);
    }

    void operator()(node::TIndex<node::Kind::TYPE_VAL> typeValIdx)
    {
        auto& typeVal = _nodePool.get(typeValIdx);

        (*this)(typeVal._asVariant);
    }

    void operator()(node::TIndex<node::Kind::TYPE_GLOBAL_CONST> typeGlobalConstIdx)
    {
        auto& typeGlobalConst = _nodePool.get(typeGlobalConstIdx);

        _writer.write(0x03);
        _writer.write(0x00);

        (*this)(typeGlobalConst._valType);
    }

    void operator()(node::TIndex<node::Kind::TYPE_GLOBAL_VAR> typeGlobalVarIdx)
    {
        auto& typeGlobalVar = _nodePool.get(typeGlobalVarIdx);

        _writer.write(0x03);
        _writer.write(0x01);

        (*this)(typeGlobalVar._valType);
    }

    void operator()(node::TIndex<node::Kind::TYPE_TABLE> typeTableIdx)
    {
        auto& typeTable = _nodePool.get(typeTableIdx);

        _writer.write(0x01);

        writeLimits(typeTable._limits);
        std::visit(*this, typeTable._refType);
    }

    void operator()(node::TIndex<node::Kind::TYPE_MEM> typeMemIdx)
    {
        _writer.write(0x02);
        writeLimits(typeMemIdx);
    }

    void operator()(uint32_t funcIdx)
    {
        Leb128 funcIdxAsLeb128{funcIdx};

        _writer.write(0x00);
        _writer.write(funcIdxAsLeb128);
    }

    NodePool & _nodePool;
    Writer   & _writer;
};

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
        _sectionId{SectionId::CUSTOM}
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

        base()(typeResult._valTypes);
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

        base()(name._bytes);
    }

    void operator()(node::TIndex<node::Kind::IMPORT> importIdx)
    {
        auto& import = get(importIdx);

        base()(import._module);
        base()(import._name);

        TImportDescriptorEmmiter<NodePool, Writer> importDescriptorEmmiter{_nodePool, _writer};

        std::visit(importDescriptorEmmiter, import._descriptor);
    }

    void operator()(node::TIndex<node::Kind::EXPORT> exportIdx)
    {
        auto& export_ = get(exportIdx);

        base()(export_._name);

        TExportDescriptorEmmiter<NodePool, Writer> exportDescriptorEmmiter{_nodePool, _writer};

        std::visit(exportDescriptorEmmiter, export_._descriptor);
    }

    void operator()(node::TIndex<node::Kind::FUNCTION> functionIdx)
    {
        auto& function = get(functionIdx);

        if (_sectionId == SectionId::FUNCTION)
        {
            Leb128 typeIdxAsLeb128{function._typeIdx};

            _writer.write(typeIdxAsLeb128);
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

    void operator()(node::TIndex<node::Kind::ELEMENT> elementIdx)
    {
        auto& element = get(elementIdx);

        // TODO
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
            base()(module._types);
        }

        if (module._imports._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::IMPORT, _writer);
            _sectionId = SectionId::IMPORT;
            base()(module._imports);
        }

        if (module._funcs._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::FUNCTION, _writer);
            _sectionId = SectionId::FUNCTION;
            base()(module._funcs);
        }

        if (module._tables._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::TABLE, _writer);
            _sectionId = SectionId::TABLE;
            base()(module._tables);
        }

        if (module._mems._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::MEMORY, _writer);
            _sectionId = SectionId::MEMORY;
            base()(module._mems);
        }

        if(module._globalConsts ._size ||
           module._globalVars   ._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::GLOBAL, _writer);
            _sectionId = SectionId::GLOBAL;
            base()(module._globalConsts);
            base()(module._globalVars);
        }

        if(module._exports._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::EXPORT, _writer);
            _sectionId = SectionId::EXPORT;
            base()(module._exports);
        }

        if (module._startOpt)
        {
            TFixUpSection<Writer> fixupSection(SectionId::START, _writer);
            _sectionId = SectionId::START;
            base()(module._startOpt.value());
        }

        if (module._elems._size)
        {
            TFixUpSection<Writer> fixupSection(SectionId::START, _writer);
            _sectionId = SectionId::ELEMENT;
            base()(module._elems);
        }
    }

    template <class Type>
    void emptyOption() {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopConclusion(node::TRange<KIND>& range) {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopPreamble(node::TRange<KIND>& range)
    {
        Leb128 rangeSizeAsLeb128{range._size};

        _writer.write(rangeSizeAsLeb128);
    }

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationConclusion(node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void loopIterationPreamble(node::TIndex<KIND>) {}

    template <com::TEnumIntegerType<node::Kind> KIND>
    void operator()(node::TIndex<KIND>)
    {
        DMIT_COM_ASSERT(!"Not implemented");
    }

    Writer& _writer;
    SectionId _sectionId;

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
