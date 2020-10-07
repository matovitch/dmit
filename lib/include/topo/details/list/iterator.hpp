#pragma once

#include "topo/details/list/cell.hpp"

namespace topo_details
{

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
    friend class ::topo_details::TList; // namespace prefix needed for clang

    using Type = typename Cell::Type;

    friend bool operator==<Cell>(const TIterator&,
                                 const TIterator&);

    friend bool operator!=<Cell>(const TIterator&,
                                 const TIterator&);
public:

    TIterator() = default;

    TIterator(Cell* cellPtr) :
        _cellPtr{cellPtr}
    {}

    TIterator(const TIterator& iterator) :
        _cellPtr{iterator._cellPtr}
    {}

    TIterator operator=(const TIterator& iterator)
    {
        _cellPtr = iterator._cellPtr;

        return *this;
    }
          Type& operator*()       { return _cellPtr->value; }
    const Type& operator*() const { return _cellPtr->value; }

          Type* operator->()       { return &(_cellPtr->value); }
    const Type* operator->() const { return &(_cellPtr->value); }

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

} // namespace topo_details
