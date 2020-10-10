#pragma once

#include "list/iterator.hpp"
#include "list/cell.hpp"

template <class>
class TList;

namespace list
{

template <class Traits>
void steal(TList<Traits>& lhs,
           TList<Traits>& rhs,
           TIterator<typename Traits::Cell>& it)
{
    using Cell = typename Traits::Cell;

    Cell* const cellPtr = it._cellPtr;

    Cell*& prev = cellPtr->_prev;
    Cell*& next = cellPtr->_next;

    if (next)
    {
        next->_prev = prev;
    }

    if (prev)
    {
        prev->_next = next;
    }
    else
    {
        rhs._head = rhs._head->_next;
    }

    if (lhs._head)
    {
        lhs._head->_prev = cellPtr;
    }

    prev = nullptr;
    next = lhs._head;

    lhs._head = cellPtr;
}

} // namespace list
