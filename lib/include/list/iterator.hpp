#pragma once

#include "list/cell.hpp"

template <class>
class TList;

namespace list
{

template <class>
class TIterator;

template <class Cell>
bool operator==(const TIterator<Cell>&,
                const TIterator<Cell>&);

template <class Cell>
bool operator!=(const TIterator<Cell>&,
                const TIterator<Cell>&);

template <class Cell>
class TIterator
{
    template <class>
    friend class ::TList; // namespace prefix needed for clang

    friend bool operator==<Cell>(const TIterator&,
                                 const TIterator&);

    friend bool operator!=<Cell>(const TIterator&,
                                 const TIterator&);
public:

    TIterator() = default;

    TIterator(Cell* cellPtr) :
        _cellPtr{cellPtr}
    {}

    TIterator(const TIterator& iterator) = default;

    TIterator& operator=(const TIterator& iterator) = default;

    // Using auto in place of Cell::Type as inner type can't be
    // forward declared and it caused inclomplete type errors

          auto& operator*()       { return _cellPtr->value; }
    const auto& operator*() const { return _cellPtr->value; }

          auto* operator->()       { return &(_cellPtr->value); }
    const auto* operator->() const { return &(_cellPtr->value); }

    TIterator& operator++()
    {
        _cellPtr = _cellPtr->_next;

        return *this;
    }

    TIterator operator++(int)
    {
        TIterator tmp{*this};

        operator++();

        return tmp;
    }

private:

    Cell* _cellPtr = nullptr;
};

template <class Cell>
bool operator==(const TIterator<Cell>& lhs,
                const TIterator<Cell>& rhs)
{
    return lhs._cellPtr ==
           rhs._cellPtr;
}

template <class Cell>
bool operator!=(const TIterator<Cell>& lhs,
                const TIterator<Cell>& rhs)
{
    return lhs._cellPtr !=
           rhs._cellPtr;
}

namespace iterator
{

template <class Type>
using TMake = TIterator<TCell<Type>>;

} // namespace iterator

namespace const_iterator
{

template <class Type>
using TMake = TIterator<const TCell<Type>>;

} // namespace const_iterator

} // namespace list
