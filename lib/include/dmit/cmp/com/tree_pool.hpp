#pragma once

#include "dmit/com/tree_pool.hpp"
#include "dmit/com/enum.hpp"

#include "dmit/cmp/cmp.hpp"

#include <cstdint>

namespace dmit::cmp
{

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

    if (!writeU32(context, sub.size() >> decltype(sub)::LOG2_SIZE_RATIO) + 1)
    {
        return false;
    }

    for (uint32_t i = 0; (i + 1) << decltype(sub)::LOG2_SIZE_RATIO < sub.size(); i++)
    {
        if (!writeBin(context, &(sub.get(i << decltype(sub)::LOG2_SIZE_RATIO)),
                                        (1 << decltype(sub)::LOG2_SIZE_RATIO) - 1))
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

template <class Kind, template <com::TEnumIntegerType<Kind>> class TNode, uint8_t LOG2_SIZE>
bool write(cmp_ctx_t* context, const typename com::tree::TTMetaPool<Kind, TNode>::template TPool<LOG2_SIZE>& pool)
{
    bool result = true;

    std::apply([&](auto&&... args) {((result |= write(context, args)), ...);}, pool._subs);

    return result;
}

} // namespace dmit::cmp
