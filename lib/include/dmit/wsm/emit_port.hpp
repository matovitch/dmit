#pragma once

#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/wasm.hpp"

#include <cstdint>
#include <variant>

namespace dmit::wsm
{

template <class NodePool, class Writer>
struct TExportDescriptorEmitter
{
    TExportDescriptorEmitter(NodePool& nodePool,
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
struct TImportDescriptorEmitter
{
    TImportDescriptorEmitter(NodePool& nodePool,
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

} // namespace dmit::wsm
