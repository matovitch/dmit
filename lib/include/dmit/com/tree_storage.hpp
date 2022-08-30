#pragma once

#include "dmit/com/log2.hpp"

#include <type_traits>
#include <cstdint>
#include <memory>
#include <vector>
#include <array>

namespace dmit::com::tree
{

namespace storage
{

template <uint8_t TYPE_SIZE,
          uint8_t ALGN_SIZE,
          uint8_t LOG2_SIZE_RATIO>
class TChunk
{

public:

    using Storage      = std::aligned_storage_t<TYPE_SIZE, ALGN_SIZE>;
    using StorageArray = std::array<Storage, 1 << LOG2_SIZE_RATIO>;

    TChunk() : _storage{new StorageArray} {}

    Storage& get(const uint32_t index)
    {
        return (*_storage)[index];
    }

    const Storage& get(const uint32_t index) const
    {
        return (*_storage)[index];
    }

private:

    std::unique_ptr<StorageArray> _storage;
};

} // namespace storage

template <uint8_t TYPE_SIZE,
          uint8_t ALGN_SIZE,
          uint8_t LOG2_SIZE>
class TStorage
{
    static constexpr auto LOG2_SIZE_RATIO = LOG2_SIZE - log2(TYPE_SIZE);

    using Chunk = storage::TChunk<TYPE_SIZE,
                                  ALGN_SIZE,
                                  LOG2_SIZE_RATIO>;
public:

    uint32_t next() const
    {
        return _counter;
    }

    uint32_t make()
    {
        if ((_counter & ((1 << LOG2_SIZE_RATIO) - 1)) == 0 && (_counter >> LOG2_SIZE_RATIO) + 1 > _chunks.size())
        {
            _chunks.emplace_back();
        }

        return _counter++;
    }

    uint32_t make(const uint32_t size)
    {
        const uint32_t counter = _counter;

        _counter += size;

        if ((_counter >> LOG2_SIZE_RATIO) + 1 > _chunks.size())
        {
            _chunks.resize((_counter >> LOG2_SIZE_RATIO) + 1);
        }

        return counter;
    }

    void trim(const uint32_t size)
    {
        _counter -= size;
    }

    void clear()
    {
        _counter = 0;
    }

    typename Chunk::Storage& get(const uint32_t index)
    {
        return _chunks[index >> LOG2_SIZE_RATIO].get(index & ((1 << LOG2_SIZE_RATIO) - 1));
    }

    const typename Chunk::Storage& get(const uint32_t index) const
    {
        return _chunks[index >> LOG2_SIZE_RATIO].get(index & ((1 << LOG2_SIZE_RATIO) - 1));
    }

private:

    uint32_t _counter = 0;

    std::vector<Chunk> _chunks;
};

namespace storage
{

template <class Type, uint8_t LOG2_SIZE>
using TMake = TStorage<sizeof(Type),
                       alignof(Type),
                       LOG2_SIZE>;
};

} // namespace dmit::com::tree
