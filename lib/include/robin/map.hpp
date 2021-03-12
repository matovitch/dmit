#pragma once

#include "robin/details/pair.hpp"
#include "robin/table.hpp"

#include <assert.h>

namespace robin
{

template <class Traits>
class TMap
{
    using Table     = typename Traits::Table;
    using Key       = typename Traits::Key;
    using Value     = typename Traits::Value;
    using Pair      = typename Traits::Pair;

public:

    using       iterator = typename Traits::     Iterator;
    using const_iterator = typename Traits::ConstIterator;

    TMap() = default;

    template <class... Args>
    void emplace(Args&&... args)
    {
        _table.emplace(std::forward<Args>(args)...);
    }

    iterator begin()
    {
        return _table.begin();
    }

    const_iterator begin() const
    {
        return _table.begin();
    }

    iterator end()
    {
        return _table.end();
    }

    const_iterator end() const
    {
        return _table.end();
    }

    std::size_t size() const
    {
        return _table.size();
    }

    bool empty() const
    {
        return _table.empty();
    }

    iterator find(const Key& key)
    {
        return _table.find(Pair{robin_details::pair::NoValue{}, key});
    }

    const_iterator find(const Key& key) const
    {
        return _table.find(Pair{robin_details::pair::NoValue{}, key});
    }

    void erase(const_iterator it)
    {
        _table.erase(it);
    }

    void erase(const Key& key)
    {
        _table.erase(Pair{robin_details::pair::NoValue{}, key});
    }

    Value& operator[](const Key& key)
    {
        auto&& fit = find(key);

        if (fit == end())
        {
            emplace(key, Value{});
        }

        return fit->second;
    }

    Value& at(const Key& key)
    {
        auto&& fit = find(key);

        if (fit == end())
        {
            assert(!"robin::map::at was called on an empty key");
        }

        return fit->second;
    }

    const Value& at(const Key& key) const
    {
        auto&& fit = find(key);

        if (fit == end())
        {
            assert(!"robin::map::at was called on an empty key");
        }

        return fit->second;
    }

private:

    Table _table;
};

} // namespace robin

namespace robin_details::map
{

template <class MapKey,
          class MapValue,
          class Hasher,
          class Comparator,
          std::size_t STACK_SIZE_LOG2,
          std::size_t TABLE_LOAD_FACTOR_LEVEL>
struct TTraits
{
    using Key            = MapKey;
    using Value          = MapValue;
    using Pair           = robin_details::TPair<Key, Value>;
    using PairHasher     = robin_details::pair::hasher     ::TMake<Key, Value, Hasher>;
    using PairComparator = robin_details::pair::comparator ::TMake<Key, Comparator>;

    using Table = robin::table::TMake<Pair,
                                      PairHasher,
                                      PairComparator,
                                      STACK_SIZE_LOG2,
                                      TABLE_LOAD_FACTOR_LEVEL>;

    using      Iterator = typename Table::      iterator;
    using ConstIterator = typename Table::const_iterator;
};

} // namespace robin_details::map

namespace robin::map
{

template <class Key,
          class Value,
          class Hasher,
          class Comparator,
          std::size_t STACK_SIZE_LOG2,
          std::size_t TABLE_LOAD_FACTOR_LEVEL>
using TMake = TMap<robin_details::map::TTraits<Key,
                                               Value,
                                               Hasher,
                                               Comparator,
                                               STACK_SIZE_LOG2,
                                               TABLE_LOAD_FACTOR_LEVEL>>;

} // namespace robin::map
