#pragma once

#include "robin/details/buffer/abstract.hpp"
#include "robin/details/buffer/heap.hpp"
#include "robin/details/buffer/view.hpp"

#include <type_traits>
#include <cstdint>
#include <memory>

namespace robin_details::buffer
{

template <class Traits>
class TStack : public Traits::Abstract
{
    static constexpr std::size_t SIZE = Traits::SIZE;

    using Bucket = typename Traits::Bucket;
    using Heap   = typename Traits::Heap;

public:

    View makeView() override
    {
        return {reinterpret_cast<uint8_t*>(_array.data()), SIZE + 1};
    }

    std::unique_ptr<Heap> makeNext() const override
    {
        return std::make_unique<Heap>((SIZE << 1) + 1);
    }

private:

    std::array<Bucket, SIZE + 1> _array;
};

namespace stack
{

template <std::size_t STACK_SIZEOF,
          std::size_t STACK_ALIGNOF,
          std::size_t STACK_SIZE>
struct TTraits
{
    static constexpr std::size_t SIZEOF  = STACK_SIZEOF;
    static constexpr std::size_t ALIGNOF = STACK_ALIGNOF;
    static constexpr std::size_t SIZE    = STACK_SIZE;

    using Abstract       =        abstract::TMake <SIZEOF, ALIGNOF>;
    using Heap           =            heap::TMake <SIZEOF, ALIGNOF>;
    using Bucket         = std::aligned_storage_t <SIZEOF, ALIGNOF>;
};

template <std::size_t SIZE,
          std::size_t SIZEOF,
          std::size_t ALIGNOF>
using TMake = TStack<TTraits<SIZE, SIZEOF, ALIGNOF>>;

} // namespace stack

} // namespace robin_details::buffer
