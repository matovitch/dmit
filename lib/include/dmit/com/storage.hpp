#pragma once

#include <type_traits>
#include <cstdint>

namespace dmit::com
{

template <class Type>
class TStorage
{
    using Bucket = std::aligned_storage_t< sizeof(Type),
                                          alignof(Type)>;
public:

    TStorage(uint64_t size) :
        _size{size},
        _buckets{new Bucket[size]}
    {}

    TStorage(const TStorage<Type>&) = delete;

    TStorage(TStorage<Type>&& storage) :
        _size    {storage._size},
        _buckets {storage._buckets}
    {
        storage._buckets = nullptr;
    }

    Type* data()
    {
        return reinterpret_cast<Type*>(_buckets);
    }

    const Type* data() const
    {
        return reinterpret_cast<Type*>(_buckets);
    }

    Type* begin()
    {
        return reinterpret_cast<Type*>(_buckets);
    }

    Type* end()
    {
        return reinterpret_cast<Type*>(_buckets) + _size;
    }

    const Type* begin() const
    {
        return reinterpret_cast<Type*>(_buckets);
    }

    const Type* end() const
    {
        return reinterpret_cast<Type*>(_buckets) + _size;
    }

    Type& operator[](const uint64_t index)
    {
        return reinterpret_cast<Type&>(_buckets[index]);
    }

    const Type& operator[](const uint64_t index) const
    {
        return reinterpret_cast<const Type&>(_buckets[index]);
    }

    bool empty() const
    {
        return _size == 0;
    }

    ~TStorage()
    {
        delete[] _buckets;
        _buckets = nullptr;
    }

    uint64_t _size;

private:

    Bucket* _buckets = nullptr;
};

} // namespace dmit::com
