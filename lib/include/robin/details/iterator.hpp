#pragma once

#include "robin/details/bucket.hpp"

#include <iterator>

namespace robin
{

template <class>
class TTable;

} // namespace robin

namespace robin_details
{

template <class>
class TIterator;

template <class Traits>
bool operator==(const TIterator<Traits>& lhs,
                const TIterator<Traits>& rhs);

template <class Traits>
bool operator!=(const TIterator<Traits>& lhs,
                const TIterator<Traits>& rhs);

namespace iterator
{

template <class TableTraits>
struct TTraits
{
    using Table  = robin::TTable<TableTraits>;
    using Type   = typename TableTraits::Type;
    using Bucket = typename TableTraits::Bucket;

    using Traits = TTraits<TableTraits>;

    using Iterator = TIterator<Traits>;
};

template <class Traits>
using TMake = TIterator<TTraits<Traits>>;

} // namespace iterator

template <class Traits>
class TIterator : std::iterator<std::forward_iterator_tag,
                                typename Traits::Type>
{
    using Bucket   = typename Traits::Bucket;
    using Table    = typename Traits::Table;
    using Type     = typename Traits::Type;
    using Iterator = typename Traits::Iterator;

    friend Table;

    friend bool operator==<Traits>(const Iterator& lhs,
                                   const Iterator& rhs);

    friend bool operator!=<Traits>(const Iterator& lhs,
                                   const Iterator& rhs);
public:

    TIterator(Bucket* const bucketPtr, const Table& table) :
        _bucketPtr{bucketPtr},
        _table{table}
    {}

    TIterator(const Iterator& toCopy) :
        _bucketPtr{toCopy._bucketPtr},
        _table{toCopy._table}
    {}

    void operator=(const Iterator& toCopy)
    {
        _bucketPtr = toCopy._bucketPtr;
    }

    Type& operator*()
    {
        return _bucketPtr->value();
    }

    const Type& operator*() const
    {
        return _bucketPtr->value();
    }

    Type* operator->()
    {
        return &(_bucketPtr->value());
    }

    const Type* operator->() const
    {
        return &(_bucketPtr->value());
    }

    Iterator& operator++()
    {
        do
        {
            _bucketPtr = (++_bucketPtr == _table._endPtr) ? _table._buckets
                                                          : _bucketPtr;
        } while (_bucketPtr->isEmpty());

        _bucketPtr = (_bucketPtr == _table._beginPtr) ? _table._endPtr
                                                      : _bucketPtr;
        return *this;
    }

    Iterator operator++(int)
    {
        Iterator tmp{*this};
        operator++();
        return tmp;
    }

private:

    Bucket* _bucketPtr;
    const Table&  _table;
};

template <class Traits>
bool operator==(const TIterator<Traits>& lhs,
                const TIterator<Traits>& rhs)
{
    return lhs._bucketPtr == rhs._bucketPtr;
}

template <class Traits>
bool operator!=(const TIterator<Traits>& lhs,
                const TIterator<Traits>& rhs)
{
    return lhs._bucketPtr != rhs._bucketPtr;
}

} // namespace robin_details
