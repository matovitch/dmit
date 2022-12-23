#pragma once

#include "dmit/wsm/leb128.hpp"
#include "dmit/wsm/wasm.hpp"

#include <optional>
#include <cstdint>
#include <variant>

namespace dmit::wsm
{

template <class NodePool, class Writer>
struct TBlockTypeEmitter
{
    TBlockTypeEmitter(NodePool& nodePool,
                      Writer& writer) :
        _nodePool{nodePool},
        _writer{writer}
    {}

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

    void operator()(node::TIndex<node::Kind::TYPE_FUNC> typeIdx)
    {
        Leb128</*IS_OBJECT=*/false> typeIdxAsLeb128{typeIdx._value};

        _writer.write(typeIdxAsLeb128);
    }

    template <class Type>
    void operator()(std::optional<Type>& option)
    {
        if (!option)
        {
            _writer.write(0x40);
        }

        (*this)(option.value());
    }

    NodePool & _nodePool;
    Writer   & _writer;
};

} // namespace dmit::wsm
