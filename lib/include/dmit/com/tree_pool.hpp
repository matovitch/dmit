#pragma once

#include "dmit/com/tree_node.hpp"
#include "dmit/com/assert.hpp"

#include "dmit/com/blit.hpp"
#include "dmit/com/enum.hpp"
#include "dmit/com/log2.hpp"

#include "stack/stack.hpp"

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

    public:

        using Index = typename TMetaNode<Kind>::template TIndex<KIND>;
        using Range = typename TMetaNode<Kind>::template TRange<KIND>;

        static constexpr auto LOG2_SIZE_RATIO = LOG2_SIZE - log2(sizeof(TNode<KIND>));

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
            return Index{_storage.push()};
        }

        void make(Range& range, const uint32_t size)
        {
            range._size = size;
            range._index = Index{_storage.push(size)};
        }

        void trim(Range& range, const uint32_t size)
        {
            _storage.trim(range._size - size);
            range._size = size;
        }

        uint32_t size() const
        {
            return _storage.size();
        }

    private:

        using Bucket = std::aligned_storage_t< sizeof(TNode<KIND>),
                                              alignof(TNode<KIND>)>;

        stack::TMake<Bucket, LOG2_SIZE_RATIO> _storage;
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
        using TList = typename TMetaNode<Kind>::template TList<KIND>;

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
        TNode<KIND>& makeGet(TIndex<KIND>& index)
        {
            index = std::get<KIND>(_subs).make();

            return get<KIND>(index);
        }

        template <TEnumIntegerType<Kind> KIND>
        TNode<KIND>& makeGet(std::optional<TIndex<KIND>>& indexOpt)
        {
            blitDefault(indexOpt);
            indexOpt = std::get<KIND>(_subs).make();

            return get<KIND>(indexOpt.value());
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

        template <TEnumIntegerType<Kind> KIND>
        TNode<KIND>& grow(TRange<KIND>& range)
        {
            TIndex<KIND> index;
            auto& node = makeGet<KIND>(index);

            if (!range._size)
            {
                range._index = index;
            }

            DMIT_COM_ASSERT(index._value == range._index._value + range._size);
            range._size++;

            return node;
        }

        template <TEnumIntegerType<Kind> KIND>
        void make(TList<KIND>& list)
        {
            makeGet<KIND>(list._begin)._next = list._begin;
        }

        template <TEnumIntegerType<Kind> KIND>
        TNode<KIND>& grow(TList<KIND> list)
        {
            TIndex<KIND> index;
            auto& node = makeGet<KIND>(index);

            auto& nextBegin = get<KIND>(list._begin)._next;
            node._next = nextBegin;
            nextBegin = index;

            return node;
        }

        template <TEnumIntegerType<Kind> KIND>
        TIndex<KIND>& back(const TList<KIND> list)
        {
            return get<KIND>(list._begin)._next;
        }

        template <TEnumIntegerType<Kind> KIND>
        bool empty(const TList<KIND> list)
        {
            return get<KIND>(list._begin)._next == list._begin;
        }

        SubPoolTuple _subs;
    };
};

} // namespace dmit::com::tree
