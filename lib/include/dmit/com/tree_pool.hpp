#pragma once

#include "dmit/com/tree_storage.hpp"
#include "dmit/com/tree_node.hpp"

#include "dmit/com/enum.hpp"
#include "dmit/com/blit.hpp"

#include <cstdint>
#include <optional>
#include <tuple>

namespace dmit::com::tree
{

template <class Kind, template <TEnumIntegerType<Kind>> class TNode>
struct TTMetaPool
{

    template <TEnumIntegerType<Kind> KIND, uint8_t LOG2_SIZE>
    class TSub
    {
        using Index = typename TMetaNode<Kind>::template TIndex<KIND>;
        using Range = typename TMetaNode<Kind>::template TRange<KIND>;

    public:

        const TNode<KIND>& get(const Index index) const
        {
            return reinterpret_cast<const TNode<KIND>&>(_storage.get(index._value));
        }

        TNode<KIND>& get(const Index index)
        {
            return reinterpret_cast<TNode<KIND>&>(_storage.get(index._value));
        }

        Index make()
        {
            return Index{_storage.make()};
        }

        void make(Range& range, const uint32_t size)
        {
            range._size = size;
            range._index = Index{_storage.make(size)};
        }

        void trim(Range& range, const uint32_t size)
        {
            _storage.trim(range._size - size);
            range._size = size;
        }

    private:

        storage::TMake<TNode<KIND>, LOG2_SIZE> _storage;
    };

    template <uint8_t LOG2_SIZE>
    struct TPool
    {
        template <TEnumIntegerType<Kind> KIND>
        using TSubPool = TSub<KIND, LOG2_SIZE>;

        using SubPoolTuple = typename TMetaNode<Kind>::template TTVector<TSubPool, std::tuple>::Type;

        template <TEnumIntegerType<Kind> KIND>
        using TIndex = typename TMetaNode<Kind>::template TIndex<KIND>;

        template <TEnumIntegerType<Kind> KIND>
        using TRange = typename TMetaNode<Kind>::template TRange<KIND>;

        template <TEnumIntegerType<Kind> KIND>
        const TNode<KIND>& get(const TIndex<KIND> index) const
        {
            return std::get<KIND>(_subs).get(index);
        }

        template <TEnumIntegerType<Kind> KIND>
        TNode<KIND>& get(const TIndex<KIND> index)
        {
            return std::get<KIND>(_subs).get(index);
        }

        template <TEnumIntegerType<Kind> KIND>
        TNode<KIND>& get(const std::optional<TIndex<KIND>> indexOpt)
        {
            return std::get<KIND>(_subs).get(indexOpt.value());
        }

        template <TEnumIntegerType<Kind> KIND>
        void make(TIndex<KIND>& index)
        {
            index = std::get<KIND>(_subs).make();
        }

        template <TEnumIntegerType<Kind> KIND>
        void make(std::optional<TIndex<KIND>>& indexOpt)
        {
            blitDefault(indexOpt);
            indexOpt = std::get<KIND>(_subs).make();
        }

        template <TEnumIntegerType<Kind> KIND>
        void make(TRange<KIND>& range, const uint32_t size)
        {
            std::get<KIND>(_subs).make(range, size);
        }

        template <TEnumIntegerType<Kind> KIND>
        void trim(TRange<KIND>& range, const uint32_t size)
        {
            std::get<KIND>(_subs).trim(range, size);
        }

        SubPoolTuple _subs;
    };
};

} // namespace dmit::com::tree