#pragma once

#include "dmit/com/enum.hpp"

#include "dmit/fmt/formatable.hpp"

#include <functional>
#include <optional>
#include <variant>
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
        ANNOTA_TYPE   ,
        ARGUMENTS     ,
        ASSIGNMENT    ,
        BINOP         ,
        DECLAR_LET    ,
        FUN_CALL      ,
        FUNCTION      ,
        LEXEME        ,
        RETURN_TYPE   ,
        SCOPE         ,
        SCOPE_VARIANT ,
        STATEM_RETURN ,
        PROGRAM
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
    node::TRange<node::Kind::ANNOTA_TYPE> _annotaTypes;
};

template<>
struct TNode<node::Kind::ANNOTA_TYPE>
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

template <>
struct TNode<node::Kind::SCOPE>
{
    node::TRange<node::Kind::SCOPE_VARIANT> _variants;
};

using Declaration = std::variant<node::TIndex<node::Kind::DECLAR_LET >>;

using Statement = std::variant<node::TIndex<node::Kind::ASSIGNMENT    >,
                               node::TIndex<node::Kind::STATEM_RETURN >>;

using Expression = std::variant<node::TIndex<node::Kind::LEXEME   >,
                                node::TIndex<node::Kind::BINOP    >,
                                node::TIndex<node::Kind::FUN_CALL >>;

template <>
struct TNode<node::Kind::SCOPE_VARIANT>
{
    std::variant<Statement,
                 Declaration,
                 Expression,
                 node::TIndex<node::Kind::SCOPE>> _value;
};

template <>
struct TNode<node::Kind::STATEM_RETURN>
{
    Expression _expression;
};

template <>
struct TNode<node::Kind::BINOP>
{
    node::TIndex<node::Kind::LEXEME> _operator;

    Expression _lhs;
    Expression _rhs;
};

} // namespace ast
} // namespace dmit
