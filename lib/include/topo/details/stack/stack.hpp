#pragma once

#include "topo/details/stack/chunk.hpp"

#include <type_traits>
#include <iterator>
#include <cstdint>
#include <memory>
#include <list>

namespace topo_details
{

template <class Traits>
class TStack
{
    using Type  = typename Traits::Type;
    using Chunk = typename Traits::Chunk;

    static constexpr std::size_t SIZE = Traits::SIZE;

public:

    TStack()
    {
        new (&_chunkSpace) Chunk(_stackSpace, Tag<SIZE>{});
        _chunkIt = _chunks.end();
    }

    template <class... Args>
    void push(Args&&... args)
    {
        if (!chunk().isFilled())
        {
            return chunk().push(args...);
        }

        if (_chunkIt == _chunks.end() && !_chunks.empty())
        {
            _chunkIt = _chunks.begin();
            return chunk().push(args...);
        }

        if (           _chunkIt  != _chunks.end() &&
             std::next(_chunkIt) != _chunks.end())
        {
            _chunkIt++;
            return chunk().push(args...);
        }

        const std::size_t size = chunk().size() ? chunk().size() << 1 : 1;
        auto ptr = new std::aligned_storage_t<sizeof(Type), alignof(Type)>[size];
        _chunks.emplace_back(std::make_unique<Chunk>(reinterpret_cast<Type*>(ptr), size));
        _chunkIt = std::prev(_chunks.end());

        return chunk().push(args...);
    }

    void pop()
    {
        if (!chunk().isEmpty())
        {
            chunk().pop();
        }

        if (chunk().isEmpty())
        {
            _chunkIt = (_chunkIt != _chunks.begin () &&
                        _chunkIt != _chunks.end   ()) ? std::prev(_chunkIt)
                                                      : _chunks.end();
        }
    }

    Type& top()
    {
        return chunk().top();
    }

    const Type& top() const
    {
        return chunk().top();
    }

    bool empty() const
    {
        return chunk().empty();
    }

    ~TStack()
    {
        for (auto& chunk : _chunks)
        {
            chunk->destroy();
            continue;
        }

        _chunkIt = _chunks.end();

        chunk().~Chunk();
    }

private:

    Chunk& chunk()
    {
        return (_chunkIt == _chunks.end()) ? *(reinterpret_cast<Chunk*>(&_chunkSpace))
                                           : **_chunkIt;
    }

    const Chunk& chunk() const
    {
        return (_chunkIt == _chunks.end()) ? *(reinterpret_cast<const Chunk*>(&_chunkSpace))
                                           : **_chunkIt;
    }

    typename std::list<std::unique_ptr<Chunk>>::iterator           _chunkIt;
    std::aligned_storage_t<       sizeof(Chunk ), alignof(Chunk )> _chunkSpace;
    std::aligned_storage_t<SIZE * sizeof(Type  ), alignof(Type  )> _stackSpace;
    std::list<std::unique_ptr<Chunk>>                              _chunks;
};

namespace stack
{

template <class TraitsType, std::size_t TRAITS_SIZE>
struct TTraits
{
    using Type  = TraitsType;
    using Chunk = stack::TChunk<Type>;

    static constexpr std::size_t SIZE = TRAITS_SIZE;
};

template <class Type, std::size_t SIZE>
using TMake = TStack<TTraits<Type, SIZE>>;

} // namespace stack

} // namespace topo_details
