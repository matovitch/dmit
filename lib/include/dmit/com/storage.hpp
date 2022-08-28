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

    Type* data() const
    {
        return reinterpret_cast<Type*>(_buckets);
    }

    ~TStorage()
    {
        if (!_buckets)
        {
            return;
        }

        delete[] _buckets;
        _buckets = nullptr;
    }

    const uint64_t _size;

private:

    Bucket* _buckets = nullptr;
};

} // namespace dmit::com
