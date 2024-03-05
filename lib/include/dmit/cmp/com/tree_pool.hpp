#pragma once

#include "dmit/ast/node.hpp"
#include "dmit/com/tree_pool.hpp"
#include "dmit/com/enum.hpp"

#include "dmit/cmp/cmp.hpp"

#include <optional>
#include <cstdint>

namespace dmit::cmp
{

template <class Kind, com::TEnumIntegerType<Kind> KIND>
bool write(cmp_ctx_t* context, const typename com::tree::template TMetaNode<Kind>::template TIndex<KIND> index)
{
    if (!writeBool(context, index._isInterface))
    {
        return false;
    }

    if (!writeU32(context, index._value))
    {
        return false;
    }

    return true;
}

template <class Kind, template <com::TEnumIntegerType<Kind>> class TNode, com::TEnumIntegerType<Kind> KIND, uint8_t LOG2_SIZE>
bool write(cmp_ctx_t* context, const typename com::tree::TTMetaPool<Kind, TNode>::template TSub<KIND, LOG2_SIZE>& sub)
{
    if (!writeU8(context, KIND))
    {
        return false;
    }

    if (!writeU8(context, alignof(TNode<KIND>)))
    {
        return false;
    }

    if (!writeU16(context, sizeof(TNode<KIND>)))
    {
        return false;
    }

    if (!sub.size())
    {
        return writeU32(context, 0);
    }

    if (!writeU32(context, (sub.size() >> std::decay_t<decltype(sub)>::LOG2_SIZE_RATIO) + 1))
    {
        return false;
    }

    for (uint32_t i = 0; (i + 1) << std::decay_t<decltype(sub)>::LOG2_SIZE_RATIO < sub.size(); i++)
    {
        if (!writeBin(context, &(sub.get(i << std::decay_t<decltype(sub)>::LOG2_SIZE_RATIO)),
                                        (1 << std::decay_t<decltype(sub)>::LOG2_SIZE_RATIO) - 1))
        {
            return false;
        }
    }

    if (!writeBin(context, &(sub.get((sub.size() >> LOG2_SIZE) << LOG2_SIZE)),
                             sub.size() & ((1 << LOG2_SIZE) - 1)))
    {
        return false;
    }

    return true;
}

template <class Kind, template <com::TEnumIntegerType<Kind>> class TNode, uint8_t INDEX, uint8_t LOG2_SIZE>
bool write(cmp_ctx_t* context, const typename com::tree::TTMetaPool<Kind, TNode>::template TPool<LOG2_SIZE>::SubPoolTuple& subs)
{
    if (!write<Kind, TNode, INDEX, LOG2_SIZE>(context, std::get<INDEX>(subs)))
    {
        return false;
    }

    if constexpr (INDEX)
    {
        return write<Kind, TNode, INDEX - 1, LOG2_SIZE>(context, subs);
    }

    return true;
}

template <class Kind, template <com::TEnumIntegerType<Kind>> class TNode, uint8_t LOG2_SIZE>
bool write(cmp_ctx_t* context, const typename com::tree::TTMetaPool<Kind, TNode>::template TPool<LOG2_SIZE>& pool)
{
    return write<Kind, TNode, std::tuple_size<decltype(pool._subs)>::value - 1, LOG2_SIZE>(context, pool._subs);
}

template <class Kind, com::TEnumIntegerType<Kind> KIND>
std::optional<typename com::tree::template TMetaNode<Kind>::template TIndex<KIND>> readComTreeIndex(cmp_ctx_t* context)
{
    bool isInterface;

    if (!readBool(context, &isInterface))
    {
        return std::nullopt;
    }

    uint32_t value;

    if (!readU32(context, &value))
    {
        return std::nullopt;
    }

    return typename com::tree::template TMetaNode<Kind>::template TIndex<KIND>{isInterface, value};
}

template <class Kind, template <com::TEnumIntegerType<Kind>> class TNode, uint8_t LOG2_SIZE>
std::optional<typename com::tree::TTMetaPool<Kind, TNode>::template TPool<LOG2_SIZE>> readComTreePool(cmp_ctx_t* context)
{
    return std::nullopt;
}

} // namespace dmit::cmp
