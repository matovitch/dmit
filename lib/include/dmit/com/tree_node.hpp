#pragma once

#include "dmit/com/enum.hpp"

#include <utility>
#include <cstdint>
#include <variant>
#include <limits>

namespace dmit::com::tree
{

template <class Kind>
struct TMetaNode
{
    template <template <TEnumIntegerType<Kind>> class TElem,
              template <class...> class TVector>
    struct TTVector
    {
        template <class>
        struct TType;

        template<TEnumIntegerType<Kind>... Kinds>
        struct TType<std::integer_sequence<TEnumIntegerType<Kind>, Kinds...>>
        {
            using Type = TVector<TElem<Kinds>...>;
        };

        using Type = typename TType<typename Kind::IntegerSequence>::Type;
    };

    template <TEnumIntegerType<Kind>>
    struct TIndex;

    struct Index
    {
        Index() : _value{std::numeric_limits<uint32_t>::max() >> 1} {}

        template <TEnumIntegerType<Kind> KIND>
        Index(const TIndex<KIND> index) :
            _isInterface {index._isInterface },
            _value       {index._value       }
        {}

        bool _isInterface :  1 = false;
        uint32_t _value   : 31;
    };

    template <TEnumIntegerType<Kind> KIND>
    struct TIndex
    {
        TIndex() : _value{std::numeric_limits<uint32_t>::max() >> 1} {}

        TIndex(const uint32_t value) : _value{value} {}

        TIndex(const Index index) :
            _isInterface {index._isInterface },
            _value       {index._value       }
        {}

        bool operator==(TIndex<KIND> tIndex) const
        {
            return _value == tIndex._value;
        }

        bool operator!=(TIndex<KIND> tIndex) const
        {
            return _value != tIndex._value;
        }

        bool _isInterface :  1 = false;
        uint32_t _value   : 31;
    };

    template <TEnumIntegerType<Kind> KIND>
    struct TRange
    {
        TIndex<KIND> operator[](uint32_t offset) const
        {
            TIndex<KIND> index{_index._value + offset};

            index._isInterface = _index._isInterface;

            return index;
        }

        TIndex<KIND> back() const
        {
            return this->operator[](_size - 1);
        }

        TIndex<KIND> _index;
        uint32_t _size;
    };

    template <TEnumIntegerType<Kind> KIND>
    struct TList
    {
        TIndex<KIND> _begin;
    };

    using VIndex = typename TTVector<TIndex, std::variant>::Type;
};

template <class Kind, TEnumIntegerType<Kind> KIND>
typename TMetaNode<Kind>::template TIndex<KIND> as(const typename TMetaNode<Kind>::Index index)
{
    return index;
}

template <class Kind, TEnumIntegerType<Kind> KIND>
typename TMetaNode<Kind>::template TIndex<KIND> as(const typename TMetaNode<Kind>::template TIndex<KIND> index)
{
    return index;
}

template <class Kind, TEnumIntegerType<Kind> KIND>
typename TMetaNode<Kind>::template TIndex<KIND> as(const typename TMetaNode<Kind>::VIndex& index)
{
    return std::get<typename TMetaNode<Kind>::template TIndex<KIND>>(index);
}

namespace v_index
{

template <class Kind>
struct THashVisitor
{
    template <com::TEnumIntegerType<Kind> KIND>
    std::size_t operator()(const typename TMetaNode<Kind>::template TIndex<KIND> tIndex)
    {
        return (tIndex._value << 0x8) | KIND;
    }
};

template <class Kind>
struct THasher
{
    std::size_t operator()(const typename TMetaNode<Kind>::VIndex vIndex) const
    {
        THashVisitor<Kind> hashVisitor;

        return std::visit(hashVisitor, vIndex);
    }
};

template <class Kind>
struct TComparator
{
    bool operator()(const typename TMetaNode<Kind>::VIndex lhs,
                    const typename TMetaNode<Kind>::VIndex rhs) const
    {
        return lhs == rhs;
    }
};

namespace
{

template <class Kind>
struct TIsInterfaceVisitor
{
    template <com::TEnumIntegerType<Kind> KIND>
    bool operator()(typename TMetaNode<Kind>::template TIndex<KIND> index)
    {
        return index._isInterface;
    }
};

template <class Kind>
struct TValueVisitor
{
    template <com::TEnumIntegerType<Kind> KIND>
    uint32_t operator()(typename TMetaNode<Kind>::template TIndex<KIND> index)
    {
        return index._value;
    }
};

} // namespace

template <class Kind>
bool isInterface(const typename TMetaNode<Kind>::VIndex vIndex)
{
    TIsInterfaceVisitor<Kind> isInterfaceVisitor;

    return std::visit(isInterfaceVisitor, vIndex);
}

template <class Kind>
uint32_t value(const typename TMetaNode<Kind>::VIndex vIndex)
{
    TValueVisitor<Kind> valueVisitor;

    return std::visit(valueVisitor, vIndex);
}

} // namespace v_index

} // namespace dmit::com::tree
