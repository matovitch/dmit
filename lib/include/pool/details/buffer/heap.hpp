#pragma once

#include "pool/details/buffer/abstract.hpp"

#include <type_traits>
#include <cstdint>

namespace pool_details
{

namespace buffer
{

template <class Traits>
class THeap : public Traits::Abstract
{
    using TypeStorage = typename Traits::TypeStorage;
    using Type        = typename Traits::Type;

public:

    THeap(const std::size_t size) :
        _memory{new TypeStorage[size]}
    {
        _head = reinterpret_cast<Type*>(_memory);
        _tail = reinterpret_cast<Type*>(_memory) + size;
    }

    THeap(THeap<Traits>&& toMove) :
        _head{toMove._head},
        _tail{toMove._tail},
        _memory{toMove._memory}
    {
        toMove._head   = nullptr;
        toMove._tail   = nullptr;
        toMove._memory = nullptr;
    }

    Type* allocate() override
    {
        if (_head == _tail)
        {
            return nullptr;
        }

        return _head++;
    }

    void clean()
    {
        auto temp = reinterpret_cast<Type*>(_memory);

        while (temp != _head)
        {
            temp->~Type();
            temp++;
        }

        _head = reinterpret_cast<Type*>(_memory);
    }

    ~THeap()
    {
        clean();
        delete[] _memory;
        _memory = nullptr;
    }

private:
          Type        * _head;
    const Type        * _tail;
          TypeStorage * _memory;
};

namespace heap
{

template <class HeapType>
struct TTraits
{
    using Type        = HeapType;
    using Abstract    = TAbstract<Type>;
    using TypeStorage = std::aligned_storage_t<sizeof  (Type),
                                               alignof (Type)>;
};

template <class Type>
using TMake = THeap<TTraits<Type>>;

} // namespace heap

} // namespace buffer

} // namespace pool_details
