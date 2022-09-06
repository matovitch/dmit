#pragma once

#include <type_traits>
#include <cstdint>
#include <utility>
#include <vector>
#include <array>

namespace stack
{

template <class Type, uint32_t SIZE>
struct TChunk
{
    using Bucket = std::aligned_storage_t< sizeof(Type),
                                          alignof(Type)>;

    using Storage = std::array<Bucket, 1 << SIZE>;

    TChunk() :
        _storage{new Storage}
    {}

    TChunk(const TChunk<Type, SIZE>&) = delete;

    TChunk(TChunk<Type, SIZE>&& chunk) :
        _storage{chunk._storage}
    {
        chunk._storage = nullptr;
    }

    Type* data() const
    {
        return reinterpret_cast<Type*>(_storage);
    }

    ~TChunk()
    {
        delete _storage;
        _storage = nullptr;
    }

    Storage* _storage = nullptr;
};

} // namespace stack

template <class Traits>
struct TStack
{
    using Type = typename Traits::Type;

    static constexpr uint32_t SIZE       = Traits::SIZE;
    static constexpr uint32_t SIZE_STACK = Traits::SIZE_STACK;

    template <class... Args>
    uint32_t push(Args&&... args)
    {
        if (SIZE_STACK && _chunks.empty())
        {
            if (_index < SIZE_STACK)
            {
                new (_stack.data() + _index) Type{std::forward<Args>(args)...};
                return _index++;
            }
            else
            {
                _chunks.emplace_back();

                for (uint32_t i = 0; i < SIZE_STACK; i++)
                {
                    new (_chunks[0].data() + i) Type{std::move(reinterpret_cast<Type&>(_stack[i]))};
                    reinterpret_cast<Type&>(_stack[i]).~Type();
                }
            }
        }

        if ((_index & ((1 << SIZE) - 1)) == 0 && (_index >> SIZE) + 1 > _chunks.size())
        {
            _chunks.emplace_back();
        }

        new (_chunks[_index >> SIZE].data() + (_index & ((1 << SIZE) - 1))) Type{std::forward<Args>(args)...};

        return _index++;
    }

    template <class... Args>
    uint32_t push(uint32_t size, Args&&... args)
    {
        const uint32_t index = _index;

        _index += size;

        if (SIZE_STACK && _chunks.empty())
        {
            if (_index < SIZE_STACK)
            {
                return index;
            }
            else
            {
                _chunks.emplace_back();

                for (uint32_t i = 0; i < SIZE_STACK; i++)
                {
                    new (_chunks[0].data() + i) Type{std::move(reinterpret_cast<Type&>(_stack[i]))};
                    reinterpret_cast<Type&>(_stack[i]).~Type();
                }
            }
        }

        if ((_index >> SIZE) + 1 > _chunks.size())
        {
            _chunks.resize((_index >> SIZE) + 1);
        }

        for (uint32_t i = index; i < _index; i++)
        {
            new (_chunks[i >> SIZE].data() + (i & ((1 << SIZE) - 1))) Type{std::forward<Args>(args)...};
        }

        return index;
    }

    void pop()
    {
        _index--;

        SIZE_STACK &&_chunks.empty() ? reinterpret_cast<Type&>(_stack[_index])                    .~Type()
                                     : _chunks[_index >> SIZE].data()[_index & ((1 << SIZE) - 1)] .~Type();
    }

    Type& top()
    {
        return SIZE_STACK && _chunks.empty() ? reinterpret_cast<Type&>(_stack.data()[_index - 1])
                                             : _chunks[(_index - 1) >> SIZE].data()[(_index - 1) & ((1 << SIZE) - 1)];
    }

    Type& get(uint32_t index)
    {
        return SIZE_STACK &&_chunks.empty() ? reinterpret_cast<Type&>(_stack.data()[index])
                                            : _chunks[index >> SIZE].data()[index & ((1 << SIZE) - 1)];
    }

    const Type& get(uint32_t index) const
    {
        return SIZE_STACK && _chunks.empty() ? reinterpret_cast<const Type&>(_stack.data()[index])
                                             : _chunks[index >> SIZE].data()[index & ((1 << SIZE) - 1)];
    }

    uint32_t size() const
    {
        return _index;
    }

    bool empty() const
    {
        return !_index;
    }

    void trim(uint32_t size)
    {
        if (SIZE_STACK && _chunks.empty())
        {
            for (uint32_t i = 0; i < size; i++)
            {
                reinterpret_cast<Type&>(_stack[_index - i - 1]).~Type();
            }
        }
        else
        {
            for (uint32_t i = 0; i < size; i++)
            {
                _chunks[(_index - i - 1) >> SIZE].data()[(_index - i - 1) & ((1 << SIZE) - 1)].~Type();
            }
        }

        _index -= size;
    }

    void clear()
    {
        if (SIZE_STACK && _chunks.empty())
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
                _chunks[_index >> SIZE].data()[_index & ((1 << SIZE) - 1)].~Type();
            }
        }
    }

    ~TStack()
    {
        clear();
    }

    uint32_t _index = 0;

    std::array  <typename stack::TChunk<Type, SIZE>::Bucket, SIZE_STACK > _stack;
    std::vector <         stack::TChunk<Type, SIZE>                     > _chunks;
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

template <class Type, std::size_t SIZE, uint32_t SIZE_STACK = 0>
using TMake = TStack<TTraits<Type, SIZE, SIZE_STACK>>;

} // namespace stack
