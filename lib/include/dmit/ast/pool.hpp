#pragma once

#include "dmit/ast/storage.hpp"
#include "dmit/ast/node.hpp"

#include <tuple>

namespace dmit
{

namespace ast
{

namespace node
{

namespace pool
{

template <com::TEnumIntegerType<Kind> KIND, uint8_t LOG2_SIZE>
class TSub
{

public:

    const TNode<KIND>& get(const TIndex<KIND> index) const
    {
        return reinterpret_cast<const TNode<KIND>&>(_storage.get(index._value));
    }

    TNode<KIND>& get(const TIndex<KIND> index)
    {
        return reinterpret_cast<TNode<KIND>&>(_storage.get(index._value));
    }

    TIndex<KIND> make()
    {
        return TIndex<KIND>{_storage.make()};
    }

    void make(const uint32_t size, TRange<KIND>& range)
    {
        range._size = size;
        range._index._value = _storage.make(size);
    }

private:

    storage::TMake<TNode<KIND>, LOG2_SIZE> _storage;
};

} // namespace pool

template <uint8_t LOG2_SIZE>
struct TPool
{
    template <com::TEnumIntegerType<Kind> KIND>
    const TNode<KIND>& get(const TIndex<KIND> index) const
    {
        return std::get<KIND>(_subs).get(index);
    }

    template <com::TEnumIntegerType<Kind> KIND>
    TNode<KIND>& get(const TIndex<KIND> index)
    {
        return std::get<KIND>(_subs).get(index);
    }

    template <com::TEnumIntegerType<Kind> KIND>
    void make(TIndex<KIND>& index)
    {
        index = std::get<KIND>(_subs).make();
    }

    template <com::TEnumIntegerType<Kind> KIND>
    void make(const uint32_t size, TRange<KIND>& range)
    {
        std::get<KIND>(_subs).make(size, range);
    }

    std::tuple<pool::TSub<Kind::ANNOTA_TYPE   , LOG2_SIZE>,
               pool::TSub<Kind::ARGUMENTS     , LOG2_SIZE>,
               pool::TSub<Kind::ASSIGNMENT    , LOG2_SIZE>,
               pool::TSub<Kind::DECLAR_LET    , LOG2_SIZE>,
               pool::TSub<Kind::EXPRESSION    , LOG2_SIZE>,
               pool::TSub<Kind::FUNCTION      , LOG2_SIZE>,
               pool::TSub<Kind::LEXEME        , LOG2_SIZE>,
               pool::TSub<Kind::RETURN_TYPE   , LOG2_SIZE>,
               pool::TSub<Kind::SCOPE         , LOG2_SIZE>,
               pool::TSub<Kind::SCOPE_VARIANT , LOG2_SIZE>,
               pool::TSub<Kind::STATEM_RETURN , LOG2_SIZE>> _subs;
};

} // namespace node
} // namespace ast
} // namespace dmit

