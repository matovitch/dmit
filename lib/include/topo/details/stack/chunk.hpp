#pragma once

#include <type_traits>
#include <utility>
#include <cstdint>

namespace topo_details
{

template <std::size_t SIZE>
struct Tag {};

namespace stack
{

template <class Type>
class TChunk
{

public:

    template <std::size_t SIZE>
    TChunk(std::aligned_storage_t<SIZE * sizeof(Type), alignof(Type)>& stackSpace, Tag<SIZE>) :
        _head(reinterpret_cast<Type*>(&stackSpace)),
        _data(reinterpret_cast<Type*>(&stackSpace)),
        _tail(reinterpret_cast<Type*>(&stackSpace) + SIZE)
    {}

    TChunk(Type* ptr, std::size_t size) :
        _head(ptr),
        _data(ptr),
        _tail(ptr + size)
    {}

    bool isFilled() const
    {
        return _head == _tail;
    }

    bool isEmpty() const
    {
        return _head == _data;
    }

    template <class... Args>
    void push(Args&&... args)
    {
        new (static_cast<void*>(_head)) Type(std::forward<Args>(args)...);
         _head++;
    }

    void pop()
    {
        _head--;
        _head->~Type();
    }

    Type& top()
    {
        return *(_head - 1);
    }

    const Type& top() const
    {
        return *(_head - 1);
    }

    bool empty() const
    {
        return _head == _data;
    }

    std::size_t size() const
    {
        return _tail - _data;
    }

    void destroy()
    {
        this->~TChunk();

        delete[] reinterpret_cast<std::aligned_storage_t<sizeof(Type), alignof(Type)>*>(_data);

        _data = nullptr;
        _head = nullptr;
    }

    ~TChunk()
    {
        for (auto ptr = _data; ptr < _head; ptr++)
        {
            ptr->~Type();
        }
    }

private:

    Type* _head;
    Type* _data;
    Type* _tail;
};

} // namespace stack

} // namespace topo_details
