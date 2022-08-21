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

    TStorage(std::size_t size) :
        _buckets{new Bucket[size]}
    {}

    Type* data() const
    {
        return reinterpret_cast<Type*>(_buckets);
    }

    ~TStorage()
    {
        delete[] _buckets;
        _buckets = nullptr;
    }

private:

    Bucket* _buckets = nullptr;
};

} // namespace dmit::com
