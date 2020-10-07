#pragma once

#include "topo/details/buffer/abstract.hpp"
#include "topo/details/buffer/stack.hpp"
#include "topo/details/buffer/heap.hpp"

#include "topo/details/stack/stack.hpp"

#include <cstdint>
#include <memory>

namespace topo_details
{

namespace pool
{

template <class Traits>
class TAllocator
{
    using AbstractBuffer = typename Traits::AbstractBuffer;
    using    StackBuffer = typename Traits::   StackBuffer;
    using     HeapBuffer = typename Traits::    HeapBuffer;

    static constexpr std::size_t SIZE = Traits::SIZE;

    template <class Type>
    using TStack = typename Traits::template TStack<Type>;

public:

    using Type = typename Traits::Type;

    TAllocator() :
        _bufferPtr{&_stackBuffer},
        _size{SIZE}
    {}

    Type* allocate()
    {
        if (!_recycleds.empty())
        {
            Type* const ptr = _recycleds.top();
            _recycleds.pop();
            ptr->~Type();
            return ptr;
        }

        if (Type* const ptr = _bufferPtr->allocate())
        {
            return ptr;
        }

        _heapBuffers.push(_size ? _size <<= 1 : 1);
        _bufferPtr = &(_heapBuffers.top());

        return _bufferPtr->allocate();
    }

    void recycle(Type* ptr)
    {
        _recycleds.push(ptr);
    }

private:

    AbstractBuffer*    _bufferPtr;
    StackBuffer        _stackBuffer;
    TStack<Type*>      _recycleds;
    TStack<HeapBuffer> _heapBuffers;

    std::size_t _size;
};

namespace allocator
{

template <class TraitsType, std::size_t TRAITS_SIZE>
struct TTraits
{
    static constexpr std::size_t SIZE = TRAITS_SIZE;

    using Type = TraitsType;

    using    StackBuffer = buffer::stack ::TMake <Type, SIZE>;
    using     HeapBuffer = buffer::heap  ::TMake <Type>;
    using AbstractBuffer = buffer::TAbstract     <Type>;

    template <class Type>
    using TStack = stack::TMake<Type, SIZE>;
};

template <class Type, std::size_t SIZE>
using TMake = TAllocator<TTraits<Type, SIZE>>;

} // namespace allocator

} // namespace pool

} // namespace topo_details
