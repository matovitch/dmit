#pragma once

#include "dmit/ast/definition_status.hpp"

#include "dmit/src/line_index.hpp"
#include "dmit/src/slice.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/enum.hpp"

#include <functional>
#include <optional>
#include <cstdint>
#include <utility>
#include <variant>
#include <vector>
#include <limits>

namespace dmit::ast
{

namespace node
{

struct Kind : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        DCL_IMPORT     ,
        DCL_VARIABLE   ,
        DEF_CLASS      ,
        DEF_FUNCTION   ,
        DEFINITION     ,
        EXP_BINOP      ,
        EXP_MONOP      ,
        EXPRESSION     ,
        FUN_CALL       ,
        LEXEME         ,
        LIT_DECIMAL    ,
        LIT_IDENTIFIER ,
        LIT_INTEGER    ,
        SCOPE          ,
        SCOPE_VARIANT  ,
        STM_RETURN     ,
        TYPE           ,
        TYPE_CLAIM     ,
        MODULE         ,
        VIEW           ,
        PARENT_PATH    ,
        SOURCE
    };

    using IntegerSequence = std::make_integer_sequence<uint8_t, SOURCE + 1>;

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Kind);
};

template <com::TEnumIntegerType<Kind>>
struct TIndex;

template <class>
struct TVariantHelper;

template<com::TEnumIntegerType<Kind>... Kinds>
struct TVariantHelper<std::integer_sequence<com::TEnumIntegerType<Kind>, Kinds...>>
{
    using Type = std::variant<TIndex<Kinds>...>;
};

struct Index
{
    Index() : _value{std::numeric_limits<uint32_t>::max() >> 1} {}

    template <com::TEnumIntegerType<Kind> KIND>
    Index(const TIndex<KIND> index) :
        _isInterface {index._isInterface },
        _value       {index._value       }
    {}

    bool _isInterface :  1 = false;
    uint32_t _value   : 31;
};

template <com::TEnumIntegerType<Kind> KIND>
struct TIndex
{
    TIndex() : _value{std::numeric_limits<uint32_t>::max() >> 1} {}

    TIndex(const uint32_t value) : _value{value} {}

    TIndex(const Index index) :
        _isInterface {index._isInterface },
        _value       {index._value       }
    {}

    bool _isInterface :  1 = false;
    uint32_t _value   : 31;
};

struct VIndex
{
    using Variant = typename TVariantHelper<Kind::IntegerSequence>::Type;

    VIndex() : _variant{} {}

    template <com::TEnumIntegerType<Kind> KIND>
    VIndex(const TIndex<KIND> index) : _variant{index} {}

    Variant _variant;
};

template <com::TEnumIntegerType<Kind> KIND>
TIndex<KIND> as(const Index index)
{
    return TIndex<KIND>{index._value};
}

template <com::TEnumIntegerType<Kind> KIND>
TIndex<KIND> as(const TIndex<KIND> index)
{
    return index;
}

template <com::TEnumIntegerType<Kind> KIND>
TIndex<KIND> as(const VIndex& index)
{
    return std::get<TIndex<KIND>>(index);
}

template <com::TEnumIntegerType<Kind> KIND>
bool operator==(TIndex<KIND> lhs,
                TIndex<KIND> rhs)
{
    return lhs._value == rhs._value;
}

template <com::TEnumIntegerType<Kind> KIND>
bool operator!=(TIndex<KIND> lhs,
                TIndex<KIND> rhs)
{
    return lhs._value != rhs._value;
}

template <com::TEnumIntegerType<Kind> KIND>
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
        return TIndex<KIND>{_index._value + _size - 1};
    }

    TIndex<KIND> _index;
    uint32_t _size;
};

namespace index
{

struct Hasher
{
    std::size_t operator()(const Index index) const
    {
        return index._value;
    }
};

struct Comparator
{
    bool operator()(const Index lhs,
                    const Index rhs) const
    {
        return lhs._value ==
               rhs._value;
    }
};

} // namesapce index

namespace v_index
{

struct HashVisitor
{
    template <com::TEnumIntegerType<Kind> KIND>
    std::size_t operator()(const TIndex<KIND> tIndex)
    {
        return (tIndex._value << 0x8) | KIND;
    }
};

struct Hasher
{
    std::size_t operator()(const VIndex vIndex) const
    {
        HashVisitor hashVisitor;

        return std::visit(hashVisitor, vIndex._variant);
    }
};

struct Comparator
{
    bool operator()(const VIndex lhs,
                    const VIndex rhs) const
    {
        return lhs._variant ==
               rhs._variant;
    }
};

} // namespace v_index

} // namespace node

using Declaration = std::variant<node::TIndex<node::Kind::DCL_VARIABLE>>;

using Statement = std::variant<node::TIndex<node::Kind::STM_RETURN>>;

using Definition = std::variant<node::TIndex<node::Kind::DEF_CLASS>,
                                node::TIndex<node::Kind::DEF_FUNCTION>>;

using Expression = std::variant<node::TIndex<node::Kind::LIT_IDENTIFIER >,
                                node::TIndex<node::Kind::LIT_DECIMAL    >,
                                node::TIndex<node::Kind::LIT_INTEGER    >,
                                node::TIndex<node::Kind::EXP_BINOP      >,
                                node::TIndex<node::Kind::EXP_MONOP      >,
                                node::TIndex<node::Kind::FUN_CALL       >>;

using ScopeVariant = std::variant<Statement,
                                  Declaration,
                                  Expression,
                                  node::TIndex<node::Kind::SCOPE>>;

template <com::TEnumIntegerType<node::Kind> KIND>
struct TNode {};

template <>
struct TNode<node::Kind::PARENT_PATH>
{
    Expression _expression;

    std::optional<node::TIndex<node::Kind::PARENT_PATH>> _next;
};

template <>
struct TNode<node::Kind::VIEW>
{
    node::TRange<node::Kind::MODULE> _modules;

    com::UniqueId _id;
};

template <>
struct TNode<node::Kind::MODULE>
{
    std::optional<Expression> _path;

    node::TRange<node::Kind::DEFINITION > _definitions;
    node::TRange<node::Kind::DCL_IMPORT > _imports;
    node::TRange<node::Kind::MODULE     > _modules;

    node::TIndex<node::Kind::MODULE> _parent;
    com::UniqueId                    _id;

    std::optional<node::TIndex<node::Kind::PARENT_PATH>> _parentPath;
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
struct TNode<node::Kind::DCL_IMPORT>
{
    Expression _path;

    node::TIndex<node::Kind::MODULE> _parent;
    com::UniqueId                    _id;
};

template <>
struct TNode<node::Kind::DEFINITION>
{
    DefinitionStatus _status;

    Definition _value;
};

template <>
struct TNode<node::Kind::DEF_CLASS>
{
    node::TIndex<node::Kind::LIT_IDENTIFIER > _name;
    node::TRange<node::Kind::TYPE_CLAIM     > _members;

    com::UniqueId _id;
};

template <>
struct TNode<node::Kind::DEF_FUNCTION>
{
    node::TIndex<node::Kind::LIT_IDENTIFIER > _name;
    node::TRange<node::Kind::TYPE_CLAIM     > _arguments;
    node::TIndex<node::Kind::SCOPE          > _body;

    std::optional<node::TIndex<node::Kind::TYPE>> _returnType;

    com::UniqueId _id;
};

template<>
struct TNode<node::Kind::TYPE_CLAIM>
{
    node::TIndex<node::Kind::LIT_IDENTIFIER > _variable;
    node::TIndex<node::Kind::TYPE           > _type;
};

template <>
struct TNode<node::Kind::TYPE>
{
    node::TIndex<node::Kind::LIT_IDENTIFIER> _name;

    node::VIndex _asVIndex;
};

template <>
struct TNode<node::Kind::LIT_IDENTIFIER>
{
    node::TIndex<node::Kind::LEXEME> _lexeme;
};

template <>
struct TNode<node::Kind::LIT_DECIMAL>
{
    node::TIndex<node::Kind::LEXEME> _lexeme;

    std::optional<double> _value;
};

template <>
struct TNode<node::Kind::LIT_INTEGER>
{
    node::TIndex<node::Kind::LEXEME> _lexeme;
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

template <>
struct TNode<node::Kind::EXP_MONOP>
{
    node::TIndex<node::Kind::LEXEME> _operator;

    Expression _expression;
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
