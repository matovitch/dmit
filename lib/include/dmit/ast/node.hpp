#pragma once

#include "dmit/ast/integer.hpp"

#include "dmit/src/line_index.hpp"
#include "dmit/src/slice.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/enum.hpp"

#include "dmit/fmt/formatable.hpp"

#include <functional>
#include <optional>
#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

namespace dmit::ast
{

namespace node
{

struct Kind : com::TEnum<uint8_t>, fmt::Formatable
{
    enum : uint8_t
    {
        DCL_VARIABLE   ,
        EXP_BINOP      ,
        EXPRESSION     ,
        FUN_CALL       ,
        FUN_DEFINITION ,
        FUN_RETURN     ,
        LEXEME         ,
        LIT_IDENTIFIER ,
        LIT_INTEGER    ,
        SCOPE          ,
        SCOPE_VARIANT  ,
        STM_RETURN     ,
        TYPE_CLAIM     ,
        PROGRAM        ,
        SOURCE
    };

    using IntegerSequence = std::make_integer_sequence<uint8_t, SOURCE>;

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Kind);
};

template <com::TEnumIntegerType<Kind> KIND>
struct TIndex
{
    uint32_t _value;
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

template<class>
struct TVariantHelper;

template<com::TEnumIntegerType<Kind>... Kinds>
struct TVariantHelper<std::integer_sequence<com::TEnumIntegerType<Kind>, Kinds...>>
{
    using Type = std::variant<TIndex<Kinds>...>;
};

using Location = typename TVariantHelper<Kind::IntegerSequence>::Type;

} // namespace node

using Declaration = std::variant<node::TIndex<node::Kind::DCL_VARIABLE>>;

using Statement = std::variant<node::TIndex<node::Kind::STM_RETURN>>;

using Expression = std::variant<node::TIndex<node::Kind::LIT_IDENTIFIER >,
                                node::TIndex<node::Kind::LIT_INTEGER    >,
                                node::TIndex<node::Kind::EXP_BINOP      >,
                                node::TIndex<node::Kind::FUN_CALL       >>;

using ScopeVariant = std::variant<Statement,
                                  Declaration,
                                  Expression,
                                  node::TIndex<node::Kind::SCOPE>>;

template <com::TEnumIntegerType<node::Kind> KIND>
struct TNode {};

template <>
struct TNode<node::Kind::PROGRAM>
{
    node::TRange<node::Kind::FUN_DEFINITION> _functions;
};

template <>
struct TNode<node::Kind::SOURCE>
{
    std::vector<uint8_t    > _srcPath;
    std::vector<uint8_t    > _srcContent;
    std::vector<uint32_t   > _srcOffsets;
    std::vector<uint32_t   > _lexOffsets;
    std::vector<lex::Token > _lexTokens;
};

template <>
struct TNode<node::Kind::FUN_DEFINITION>
{
    node::TIndex<node::Kind::LIT_IDENTIFIER > _name;
    node::TRange<node::Kind::TYPE_CLAIM     > _arguments;
    node::TIndex<node::Kind::FUN_RETURN     > _returnType;
    node::TIndex<node::Kind::SCOPE          > _body;
};

template<>
struct TNode<node::Kind::TYPE_CLAIM>
{
    node::TIndex<node::Kind::LIT_IDENTIFIER> _variable;
    node::TIndex<node::Kind::LIT_IDENTIFIER> _type;
};

template <>
struct TNode<node::Kind::FUN_RETURN>
{
    std::optional<node::TIndex<node::Kind::LIT_IDENTIFIER>> _option;
};

template <>
struct TNode<node::Kind::LIT_IDENTIFIER>
{
    node::TIndex<node::Kind::LEXEME> _lexeme;

    std::optional<node::Location> _parentScope;
};

template <>
struct TNode<node::Kind::LIT_INTEGER>
{
    node::TIndex<node::Kind::LEXEME> _lexeme;

    Integer _value;
};

template <>
struct TNode<node::Kind::LEXEME>
{
    node::TIndex<node::Kind::SOURCE> _source;
    uint32_t _index;

    mutable std::optional<lex::Token> _token; // cache
};

template <>
struct TNode<node::Kind::SCOPE>
{
    node::TRange<node::Kind::SCOPE_VARIANT> _variants;

    std::optional<node::Location> _parentScope;
};

template <>
struct TNode<node::Kind::SCOPE_VARIANT>
{
    ScopeVariant _value;
};

template <>
struct TNode<node::Kind::STM_RETURN>
{
    Expression _expression;
};

template <>
struct TNode<node::Kind::EXP_BINOP>
{
    node::TIndex<node::Kind::LEXEME> _operator;

    Expression _lhs;
    Expression _rhs;
};

template<>
struct TNode<node::Kind::DCL_VARIABLE>
{
    node::TIndex<node::Kind::TYPE_CLAIM> _typeClaim;
};

template<>
struct TNode<node::Kind::FUN_CALL>
{
    node::TIndex<node::Kind::LIT_IDENTIFIER > _callee;

    node::TRange<node::Kind::EXPRESSION> _arguments;
};

template<>
struct TNode<node::Kind::EXPRESSION>
{
    Expression _value;
};

} // namespace dmit::ast
