#pragma once

#include "robin/details/buffer/abstract.hpp"
#include "robin/details/buffer/view.hpp"

#include <type_traits>
#include <cstdint>
#include <memory>

namespace robin_details::buffer
{

template <class Traits>
class THeap : public Traits::Abstract
{
    using Bucket = typename Traits::Bucket;
    using Heap   = typename Traits::Heap;

public:

    THeap(const std::size_t size) :
        _data{new Bucket[size]},
        _size{size}
    {}

    View makeView() override
    {
        return {reinterpret_cast<uint8_t*>(_data), _size};
    }

    std::unique_ptr<Heap> makeNext() const override
    {
        return std::make_unique<Heap>(((_size - 1) << 1) + 1);
    }

    ~THeap() override
    {
        delete[] _data;
    }

private:

    Bucket* const    _data;
    const std::size_t _size;
};

namespace heap
{

template <std::size_t,
          std::size_t>
struct TTraits;

template <std::size_t SIZEOF,
          std::size_t ALIGNOF>
using TMake = THeap<TTraits<SIZEOF, ALIGNOF>>;

template <std::size_t SIZEOF,
          std::size_t ALIGNOF>
struct TTraits
{
    using Abstract =        abstract::TMake <SIZEOF, ALIGNOF>;
    using Heap     =            heap::TMake <SIZEOF, ALIGNOF>;
    using Bucket   = std::aligned_storage_t <SIZEOF, ALIGNOF>;
};

} // namespace heap

} // namespace robin_details::buffer
