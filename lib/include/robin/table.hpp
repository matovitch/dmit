#pragma once

#include "robin/details/iterator.hpp"
#include "robin/details/bucket.hpp"
#include "robin/details/xor_shifter.hpp"

#include "robin/details/buffer/manager.hpp"
#include "robin/details/likely.hpp"

#include <utility>

namespace robin
{

template <class Traits>
class TTable
{
    using Type          = typename Traits::Type;
    using Bucket        = typename Traits::Bucket;
    using Hasher        = typename Traits::Hasher;
    using Comparator    = typename Traits::Comparator;
    using XorShifter    = typename Traits::XorShifter;
    using BufferManager = typename Traits::BufferManager;

    static constexpr std::size_t MASK              = Traits::MASK;
    static constexpr std::size_t STACK_SIZE        = Traits::STACK_SIZE;
    static constexpr std::size_t LOAD_FACTOR_LEVEL = Traits::LOAD_FACTOR_LEVEL;

public:

    using       iterator = typename Traits::     Iterator;
    using const_iterator = typename Traits::ConstIterator;

    friend       iterator;
    friend const_iterator;

    TTable() :
        _mask{MASK},
        _size{0},
        _capacity{STACK_SIZE}
    {
        const auto& bufferView = _bufferManager.makeView();

        _buckets  = reinterpret_cast<Bucket*>(bufferView.data);
        _capacity =                           bufferView.size - 1;

        init();
    }

    template <class... Args>
    void emplace(Args&&... args)
    {
        Type t{std::forward<Args>(args)...};

        REDO: // goto label

        uint8_t dib = Bucket::FILLED;

        Bucket* head = _buckets + (_hasher(t) & _mask);

        // Skip filled buckets with larger dib
        while (dib < head->dib())
        {
            BUCKET_SCAN: // goto label

            ++dib;
            head = (++head == _endPtr) ? _buckets : head;

            if (ROBIN_UNLIKELY(!dib))
            {
                rehash();
                goto REDO;
            }
        }

        if (head->isEmpty())
        {
            // We check for rehashing here and not at the start to make insertion of existing element faster
            if (ROBIN_UNLIKELY(++_size << LOAD_FACTOR_LEVEL > (_capacity << LOAD_FACTOR_LEVEL) - _capacity))
            {
                rehash();
                goto REDO;
            }

            head->fill(dib, std::move(t));

            // If the table was empty, initialize the _beginPtr
            if (_beginPtr == _endPtr)
            {
                _beginPtr = head;
            }
        }
        else
        {
            if (dib != head->dib())
            {
                // Copy the value of the found bucket and insert our own
                Type tTmp = head->value();
                const uint8_t dibTmp = head->dib();

                head->fill(dib, std::move(t));

                new (&t) Type{std::move(tTmp)};
                dib = dibTmp;

                goto BUCKET_SCAN;
            }
            else if (_comparator(t, head->value()))
            {
                head->fillValue(std::move(t)); // Rewrite anyway to be able to use a table as a map
                return;
            }

            // Same dib but different value, we don't touch the bucket and go emplace ourselve further
            goto BUCKET_SCAN;
        }
    }

    void erase(const Type& t)
    {
        uint8_t dib = Bucket::FILLED;

        Bucket* prec = _buckets + (_hasher(t) & _mask);

        // Skip buckets with lower dib or different value
        while (dib < prec->dib() || (dib == prec->dib() && !_comparator(t, prec->value())))
        {
            dib++;
            prec = (++prec == _endPtr) ? _buckets : prec;
        }

        if (dib == prec->dib())
        {
            shiftBuckets(prec);
        }
    }

    std::size_t size() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size == 0;
    }

    iterator begin()
    {
        return iterator{_beginPtr, *this};
    }

    const_iterator begin() const
    {
        return const_iterator{_beginPtr, *this};
    }

    iterator end()
    {
        return iterator{_endPtr, *this};
    }

    const_iterator end() const
    {
        return const_iterator{_endPtr, *this};
    }

    template <class Type>
    iterator find(const Type& t)
    {
        return tFind<TTable<Traits>, iterator, Type>(*this, t);
    }

    template <class Type>
    const_iterator find(const Type& t) const
    {
        return tFind<const TTable<Traits>, const_iterator, Type>(*this, t);
    }

    void erase(const_iterator it)
    {
        Bucket* bucketPtr = it._bucketPtr;

        shiftBuckets(bucketPtr);
    }

private:

    void rehash()
    {
        Bucket* const     oldBuckets  = _buckets;
        const std::size_t oldCapacity = _capacity;

                                 _bufferManager.makeNext();
        const auto& bufferView = _bufferManager.makeView();

        _buckets = reinterpret_cast<Bucket*>(bufferView.data);

        _capacity <<= 1;

        init();

        for (std::size_t bucketIndex = 0;
                         bucketIndex < oldCapacity;
                         bucketIndex++)
        {
            auto& oldBucket = oldBuckets[bucketIndex];

            if (oldBucket.isFilled())
            {
                emplace(oldBucket.value());
                oldBucket.~Bucket();
            }
        }

        oldBuckets[oldCapacity].~Bucket();

        _bufferManager.dropPrevious();
    }

    void shiftBuckets(Bucket* prec)
    {
        Bucket* succ = (prec + 1 == _endPtr) ? _buckets : prec + 1;

        // Shift the right-adjacent buckets to the left
        while (succ->dib() > Bucket::FILLED)
        {
            prec->fill(succ->dib() - 1, succ->value());
            prec = succ;
            succ = (++succ == _endPtr) ? _buckets : succ;
        }

        // Empty the bucket and decrement the size
        prec->markEmpty();

        if (prec == _beginPtr)
        {
            if (_size == 0)
            {
                _beginPtr = _endPtr;
                return;
            }

            do
            {
                _beginPtr = _buckets + (_xorShifter() & _mask);

            } while (_beginPtr->isEmpty());
        }
    }

    template <class Table, class Iterator, class Type>
    static Iterator tFind(Table& table, const Type& t)
    {
        uint8_t dib =  Bucket::FILLED;

        Bucket* prec = table._buckets + (table._hasher(t) & table._mask);

        const Comparator& comparator = table._comparator;
        Bucket* const     buckets    = table._buckets;
        Bucket* const     endPtr     = table._endPtr;

        // Skip buckets with lower dib or different value
        while (dib < prec->dib() || (dib == prec->dib() && !comparator(t, prec->value())))
        {
            dib++;
            prec = (++prec == endPtr) ? buckets : prec;
        }

        if (dib == prec->dib())
        {
            return Iterator{prec, table};
        }

        // No luck :(
        return table.end();
    }

    void init()
    {
        for (std::size_t bucketIndex = 0;
                         bucketIndex <= _capacity;
                         bucketIndex++)
        {
            new (reinterpret_cast<void*>(_buckets + bucketIndex)) Bucket{};
        }

        _mask = _capacity - 1;

        _endPtr   = _buckets + _capacity;
        _beginPtr = _endPtr;

        _endPtr->markFilled(); // for it == end() optimization

        _size = 0;
    }

    Bucket*       _buckets;
    std::size_t   _mask;
    Bucket*       _endPtr;
    Bucket*       _beginPtr;
    Hasher        _hasher;
    Comparator    _comparator;
    std::size_t   _size;
    std::size_t   _capacity;
    XorShifter    _xorShifter;
    BufferManager _bufferManager;
};

} // namespace robin

namespace robin_details::table
{

template <class TableType,
          class TableHasher,
          class TableComparator,
          std::size_t STACK_SIZE_LOG2,
          std::size_t TABLE_LOAD_FACTOR_LEVEL>
struct TTraits
{
    static constexpr std::size_t STACK_SIZE = 1 << STACK_SIZE_LOG2;
    static constexpr std::size_t MASK       = STACK_SIZE - 1;

    static constexpr std::size_t LOAD_FACTOR_LEVEL = TABLE_LOAD_FACTOR_LEVEL;

    using Type       = TableType;
    using Hasher     = TableHasher;
    using Comparator = TableComparator;

    using Bucket = TBucket<Type>;

    using BufferManager = buffer::manager::TMake<Bucket, STACK_SIZE>;

    using Traits = TTraits<Type, Hasher, Comparator, STACK_SIZE_LOG2, LOAD_FACTOR_LEVEL>;

    using      Iterator = iterator::TMake<Traits>;
    using ConstIterator = iterator::TMake<Traits>;

    using XorShifter = robin_details::XorShifter;
};

} // namespace robin_details::table

namespace robin::table
{

template <class Type,
          class Hasher,
          class Comparator,
          std::size_t STACK_SIZE_LOG2,
          std::size_t LOAD_FACTOR_LEVEL>
using TMake = TTable<robin_details::table::TTraits<Type,
                                                   Hasher,
                                                   Comparator,
                                                   STACK_SIZE_LOG2,
                                                   LOAD_FACTOR_LEVEL>>;
} // namespace robin::table

