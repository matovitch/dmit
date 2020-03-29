#pragma once

#include "dmit/com/enum.hpp"

#include "dmit/fmt/formatable.hpp"

#include <functional>
#include <vector>

namespace dmit
{

namespace ast
{

namespace node
{

struct Kind : com::TEnum<uint8_t>, fmt::Formatable
{
    enum : uint8_t
    {
        PROGRAM,
        FUNCTION,
        ARGUMENTS,
        TYPING_STATEMENT,
        RETURN_TYPE,
        SCOPE,
        LEXEME,
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Kind);
};

template <com::TEnumIntegerType<Kind> KIND>
struct TIndex
{
    using Type = uint32_t;

    Type _value;
};

template <com::TEnumIntegerType<Kind> KIND>
struct TRange
{
    node::TIndex<KIND> operator[](uint32_t offset) const
    {
        return node::TIndex<KIND>{_index._value + offset};
    }

    node::TIndex<KIND> _index;
    uint32_t _size;
};

} // namespace node

template <com::TEnumIntegerType<node::Kind> KIND>
struct TNode {};

template <>
struct TNode<node::Kind::PROGRAM>
{
    node::TRange<node::Kind::FUNCTION> _functions;
};

template <>
struct TNode<node::Kind::FUNCTION>
{
    node::TIndex<node::Kind::LEXEME      > _name;
    node::TIndex<node::Kind::ARGUMENTS   > _arguments;
    node::TIndex<node::Kind::RETURN_TYPE > _returnType;
    node::TIndex<node::Kind::SCOPE       > _body;
};

template <>
struct TNode<node::Kind::ARGUMENTS>
{
    node::TRange<node::Kind::TYPING_STATEMENT> _list;
};

template<>
struct TNode<node::Kind::TYPING_STATEMENT>
{
    node::TIndex<node::Kind::LEXEME > _variable;
    node::TIndex<node::Kind::LEXEME > _type;
};

template <>
struct TNode<node::Kind::RETURN_TYPE>
{
    std::optional<node::TIndex<node::Kind::LEXEME >> _option;
};

template <>
struct TNode<node::Kind::LEXEME>
{
    uint32_t _index;
};

} // namespace ast
} // namespace dmit
