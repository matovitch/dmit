#pragma once

#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/wasm.hpp"

#include "dmit/com/tree_visitor.hpp"
#include "dmit/com/enum.hpp"

#include <cstdint>

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

template <class Derived, class NodePool>
using TBaseVisitor = typename com::tree::TTMetaVisitor<node::Kind,
                                                       TNode,
                                                       NodePool>::template TVisitor<Derived>;
template <class NodePool, class Writer>
struct TEmitter : TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>
{
    using TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>::base;
    using TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>::get;

    TEmitter(NodePool& nodePool, Writer& writer) :
        TBaseVisitor<TEmitter<NodePool, Writer>, NodePool>{nodePool},
        _writer(writer)
    {}

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
        // TODO base()(import._descriptor);
    }

    void operator()(node::TIndex<node::Kind::MODULE> moduleIdx)
    {
        _writer.write(K_MAGIC   , sizeof(K_MAGIC   ));
        _writer.write(K_VERSION , sizeof(K_VERSION ));

        auto& module = get(moduleIdx);

        {
            TFixUpSection<Writer> fixupSection(SectionId::TYPE, _writer);
            base()(module._types);
        }
        {
            TFixUpSection<Writer> fixupSection(SectionId::IMPORT, _writer);
            base()(module._imports);
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

    Writer& _writer;

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
