#pragma once

#include "robin/details/buffer/abstract.hpp"
#include "robin/details/buffer/stack.hpp"
#include "robin/details/buffer/heap.hpp"
#include "robin/details/buffer/view.hpp"

namespace robin_details::buffer
{

template <class Traits>
class TManager
{
    static constexpr std::size_t SIZEOF  = Traits::SIZEOF;

    using Abstract = typename Traits::Abstract;
    using Stack    = typename Traits::Stack;
    using Heap     = typename Traits::Heap;

public:

    TManager() : _ptrBuffer{&_stack} {}

    View makeView()
    {
        return  _ptrBuffer->makeView();
    }

    void makeNext()
    {
        _ptrHeapTmp = std::move(_ptrBuffer->makeNext());
        _ptrBuffer = _ptrHeapTmp.get();
    }

    void dropPrevious()
    {
        _ptrHeap = std::move(_ptrHeapTmp);
    }

private:

    Stack                 _stack;
    Abstract*             _ptrBuffer;
    std::unique_ptr<Heap> _ptrHeap;
    std::unique_ptr<Heap> _ptrHeapTmp;
};

namespace manager
{

template <std::size_t MANAGER_SIZE,
          std::size_t MANAGER_SIZEOF,
          std::size_t MANAGER_ALIGNOF>
struct TTraits
{
    static constexpr std::size_t SIZE    = MANAGER_SIZE;
    static constexpr std::size_t SIZEOF  = MANAGER_SIZEOF;
    static constexpr std::size_t ALIGNOF = MANAGER_ALIGNOF;

    using Stack    =    stack::TMake <SIZEOF, ALIGNOF, SIZE>;
    using Heap     =     heap::TMake <SIZEOF, ALIGNOF>;
    using Abstract = abstract::TMake <SIZEOF, ALIGNOF>;
};

template <class Type, std::size_t SIZE>
using TMake = TManager<TTraits<SIZE,
                               sizeof  (Type),
                               alignof (Type)>>;
} // namespace manager

} // namespace robin_details::buffer
