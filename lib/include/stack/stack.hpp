#pragma once

#include <type_traits>
#include <cstdint>
#include <utility>
#include <vector>
#include <array>

namespace stack
{

template <class Type>
struct TStorage
{
    using Bucket = std::aligned_storage_t< sizeof(Type),
                                          alignof(Type)>;
    TStorage(uint32_t size) :
        _buckets{new Bucket[size]}
    {}

    TStorage(const TStorage<Type>&) = delete;

    TStorage(TStorage<Type>&& storage) :
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
        delete[] _buckets;
        _buckets = nullptr;
    }

    Bucket* _buckets = nullptr;
};

} // namespace stack

template <class Traits>
struct TStack
{
    using Type = typename Traits::Type;

    static constexpr uint32_t SIZE       = Traits::SIZE;
    static constexpr uint32_t SIZE_STACK = Traits::SIZE_STACK;

    template <class... Args>
    void push(Args&&... args)
    {
        if (_storages.empty())
        {
            if (_index < SIZE_STACK)
            {
                new (_stack.data() + _index) Type{std::forward<Args>(args)...};
                _index++;
                return;
            }
            else
            {
                _storages.emplace_back(1 << SIZE);

                for (uint32_t i = 0; i < SIZE_STACK; i++)
                {
                    new (_storages[0].data() + i) Type{std::move(reinterpret_cast<Type&>(_stack[i]))};
                    reinterpret_cast<Type&>(_stack[i]).~Type();
                }
            }
        }

        if ((_index & ((1 << SIZE) - 1)) == 0 && _index >= _maxIndex)
        {
            _storages.emplace_back(1 << SIZE);
            _maxIndex = _index;
        }

        new (_storages[_index >> SIZE].data() + (_index & ((1 << SIZE) - 1))) Type{std::forward<Args>(args)...};
        _index++;
    }

    void pop()
    {
        if (!_index)
        {
            return;
        }

        _index--;

        _storages.empty() ? reinterpret_cast<Type&>(_stack[_index])                      .~Type()
                          : _storages[_index >> SIZE].data()[_index & ((1 << SIZE) - 1)] .~Type();
    }

    Type* top()
    {
        if (!_index)
        {
            return nullptr;
        }

        return _storages.empty() ? reinterpret_cast<Type*>(_stack.data() + _index - 1)
                                 : _storages[(_index - 1) >> SIZE].data() + ((_index - 1) & ((1 << SIZE) - 1));
    }

    bool empty() const
    {
        return !_index;
    }

    ~TStack()
    {
        if (_storages.empty())
        {
            while (_index--)
            {
                reinterpret_cast<Type&>(_stack[_index]).~Type();
            }
        }
        else
        {
            while (_index--)
            {
                _storages[_index >> SIZE].data()[_index & ((1 << SIZE) - 1)].~Type();
            }
        }
    }

    uint32_t _index    = 0;
    uint32_t _maxIndex = 0;

    std::array  <typename stack::TStorage<Type>::Bucket, SIZE_STACK> _stack;
    std::vector <         stack::TStorage<Type>> _storages;
};

namespace stack
{

template <class TraitsType, uint32_t TRAITS_SIZE, uint32_t TRAITS_SIZE_STACK>
struct TTraits
{
    using Type = TraitsType;
    static constexpr uint32_t SIZE       = TRAITS_SIZE;
    static constexpr uint32_t SIZE_STACK = TRAITS_SIZE_STACK;
};

template <class Type, std::size_t SIZE, uint32_t SIZE_STACK>
using TMake = TStack<TTraits<Type, SIZE, SIZE_STACK>>;

} // namespace stack
