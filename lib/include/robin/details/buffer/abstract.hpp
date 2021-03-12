#pragma once

#include "robin/details/buffer/view.hpp"

#include <memory>

namespace robin_details::buffer
{

template <class Traits>
class TAbstract
{
    using Heap = typename Traits::Heap;

public:

    virtual                 View  makeView()       = 0;
    virtual std::unique_ptr<Heap> makeNext() const = 0;

    virtual ~TAbstract() {}
};

namespace heap
{

template <std::size_t,
          std::size_t>
struct TTraits;

} // namespace heap

template <class>
class THeap;

namespace abstract
{

template <std::size_t SIZEOF,
          std::size_t ALIGNOF>
struct TTraits
{
    using Heap = THeap<heap::TTraits<SIZEOF, ALIGNOF>>;
};

template <std::size_t SIZEOF,
          std::size_t ALIGNOF>
using TMake = TAbstract<TTraits<SIZEOF, ALIGNOF>>;

} // namespace abstract

} // namespace robin_details::buffer
