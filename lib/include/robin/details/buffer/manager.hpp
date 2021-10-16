#pragma once

#include "robin/details/buffer/abstract.hpp"
#include "robin/details/buffer/stack.hpp"
#include "robin/details/buffer/heap.hpp"
#include "robin/details/buffer/view.hpp"

#include <cstdint>
#include <memory>
#include <vector>

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

    View makeView()
    {
        return  _index ? _ptrHeaps[_index - 1]->makeView() :
                                         _stack.makeView() ;
    }

    void makeNext()
    {
        if (_ptrHeaps.empty())
        {
            _ptrHeaps.emplace_back(_stack.makeNext());
        }

        if (_index == _ptrHeaps.size())
        {
            _ptrHeaps.emplace_back(_ptrHeaps.back()->makeNext());
        }

        _index = _ptrHeaps.size();
    }

    void makePrev()
    {
        _index ? _index-- : 0;
    }

private:

    uint8_t                            _index = 0;
    Stack                              _stack;
    std::vector<std::unique_ptr<Heap>> _ptrHeaps;
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
