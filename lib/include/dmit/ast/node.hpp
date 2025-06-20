#pragma once

#include "dmit/ast/definition_role.hpp"

#include "dmit/wsm/wasm.hpp"

#include "dmit/lex/token.hpp"

#include "dmit/src/line_index.hpp"
#include "dmit/src/slice.hpp"

#include "dmit/com/reference.hpp"
#include "dmit/com/tree_node.hpp"
#include "dmit/com/tree_pool.hpp"
#include "dmit/com/unique_id.hpp"
#include "dmit/com/storage.hpp"
#include "dmit/com/enum.hpp"

#include "schmit/scheduler.hpp"

#include <filesystem>
#include <optional>
#include <cstdint>
#include <variant>
#include <vector>

#ifdef DMIT_SEM_CONTEXT_DEBUG
    using SchmitScheduler = schmit::TScheduler<1, std::string>;
#else
    using SchmitScheduler = schmit::TScheduler<1>;
#endif

using SchmitTaskNode = typename SchmitScheduler::TaskNode;

namespace dmit::ast
{

namespace node
{

struct Status : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        ASTED,
        IDENTIFIED,
        LOCKED,
        BOUND,
        WASMED
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Status);
};

struct Kind : com::TEnum<uint8_t>
{
    enum : uint8_t
    {
        ANY            ,
        DCL_IMPORT     ,
        DCL_VARIABLE   ,
        DEF_CLASS      ,
        DEF_FUNCTION   ,
        DEFINITION     ,
        EXP_BINOP      ,
        EXP_MONOP      ,
        FUN_CALL       ,
        LEXEME         ,
        IDENTIFIER     ,
        LIT_DECIMAL    ,
        LIT_INTEGER    ,
        PARENT_PATH    ,
        PATTERN        ,
        SCOPE          ,
        STM_RETURN     ,
        TYPE           ,
        TYPE_CLAIM     ,
        MODULE         ,
        VIEW           ,
        SOURCE
    };

    DMIT_COM_ENUM_IMPLICIT_FROM_INT(Kind);

    using IntegerSequence = std::make_integer_sequence<uint8_t, SOURCE + 1>;
};

template <com::TEnumIntegerType<Kind> KIND>
using TRange = typename com::tree::TMetaNode<Kind>::template TRange<KIND>;

template <com::TEnumIntegerType<Kind> KIND>
using TList = typename com::tree::TMetaNode<Kind>::template TList<KIND>;

template <com::TEnumIntegerType<Kind> KIND>
using TIndex = typename com::tree::TMetaNode<Kind>::template TIndex<KIND>;

using VIndex = typename com::tree::TMetaNode<Kind>::VIndex;

using Index = typename com::tree::TMetaNode<Kind>::Index;

using VIndexOrLock = std::variant<VIndex, SchmitTaskNode>;

template <com::TEnumIntegerType<Kind> KIND>
auto as = [](auto value) -> TIndex<KIND>
{
    return com::tree::as<Kind, KIND>(value);
};

} // namespace node

template <com::TEnumIntegerType<node::Kind> KIND>
struct TNode;

namespace node
{

template <uint8_t LOG2_SIZE>
using TPool = typename com::tree::TTMetaPool<Kind, TNode>::TPool<LOG2_SIZE>;

} // namespace node

template <>
struct TNode<node::Kind::PARENT_PATH>
{
    node::TIndex<node::Kind::PARENT_PATH> _next;

    node::VIndex _expression;
};

template <>
struct TNode<node::Kind::VIEW>
{
    node::TRange<node::Kind::MODULE> _modules;

    com::UniqueId _id;

    node::Status _status;
};

template <>
struct TNode<node::Kind::MODULE>
{
    std::optional<node::VIndex> _path;

    node::TRange<node::Kind::DEFINITION > _definitions;
    node::TRange<node::Kind::DCL_IMPORT > _imports;
    node::TRange<node::Kind::MODULE     > _modules;

    node::TIndex<node::Kind::MODULE> _parent;
    com::UniqueId                    _id;

    node::TList<node::Kind::PARENT_PATH> _parentPath;

    node::Status _status;
};

template <>
struct TNode<node::Kind::SOURCE>
{
    TNode(const std::filesystem::path  & path,
          const com::TStorage<uint8_t> & content) :
        _srcPath{path},
        _srcContent{content}
    {}

    std::filesystem::path                   _srcPath;
    com::TConstRef<com::TStorage<uint8_t> > _srcContent;
    std::vector<uint32_t                  > _srcOffsets;
    std::vector<uint32_t                  > _lexOffsets;
    std::vector<lex::Token                > _lexTokens;
};

template <>
struct TNode<node::Kind::DCL_IMPORT>
{
    node::VIndex _path;

    node::TIndex<node::Kind::MODULE> _parent;
    com::UniqueId                    _id;

    node::Status _status;
};

template <>
struct TNode<node::Kind::DEFINITION>
{
    DefinitionRole _role;

    node::VIndex _value;

    node::TIndex<node::Kind::MODULE> _parent;
};

template <>
struct TNode<node::Kind::DEF_CLASS>
{
    node::TIndex<node::Kind::IDENTIFIER > _name;
    node::TRange<node::Kind::TYPE_CLAIM > _members;

    node::TIndex<node::Kind::DEFINITION> _parent;

    com::UniqueId _id;

    node::Status _status;
};

template <>
struct TNode<node::Kind::DEF_FUNCTION>
{
    node::TIndex<node::Kind::IDENTIFIER   > _name;
    node::TRange<node::Kind::DCL_VARIABLE > _arguments;

    std::optional<node::TIndex<node::Kind::SCOPE >> _body;
    std::optional<node::TIndex<node::Kind::TYPE  >> _returnType;

    node::TIndex<node::Kind::DEFINITION> _parent;

    wsm::node::VIndex _asWsm;

    com::UniqueId _id;

    node::Status _status;
};

template<>
struct TNode<node::Kind::TYPE_CLAIM>
{
    node::TIndex<node::Kind::IDENTIFIER > _variable;
    node::TIndex<node::Kind::TYPE       > _type;
};

template <>
struct TNode<node::Kind::TYPE>
{
    node::TIndex<node::Kind::IDENTIFIER> _name;
};

template <>
struct TNode<node::Kind::IDENTIFIER>
{
    node::TIndex<node::Kind::LEXEME> _lexeme;

    node::VIndexOrLock _asVIndexOrLock;
    node::Status _status;
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

    node::TIndex<node::Kind::DEF_CLASS> _expectedType;

    static node::VIndexOrLock _asVIndexOrLock;
    static node::Status _status;
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
    node::TRange<node::Kind::ANY> _variants;

    node::VIndex _parent;

    com::UniqueId _id;
};

template <>
struct TNode<node::Kind::ANY>
{
    node::VIndex _value;
};

template <>
struct TNode<node::Kind::STM_RETURN>
{
    node::VIndex _expression;
};

template <>
struct TNode<node::Kind::EXP_BINOP>
{
    node::TIndex<node::Kind::LEXEME> _operator;

    node::VIndex _lhs;
    node::VIndex _rhs;

    node::TIndex<node::Kind::DEF_FUNCTION > _asFunction;

    node::VIndexOrLock _asVIndexOrLock;
    node::Status _status;
};

template <>
struct TNode<node::Kind::EXP_MONOP>
{
    node::TIndex<node::Kind::LEXEME> _operator;

    node::VIndex _expression;
};

template <>
struct TNode<node::Kind::PATTERN>
{
    node::TIndex<node::Kind::IDENTIFIER> _variable;
};

template<>
struct TNode<node::Kind::DCL_VARIABLE>
{
    node::TIndex<node::Kind::TYPE_CLAIM> _typeClaim;

    com::UniqueId _id;

    node::Status _status;

    wsm::node::TIndex<wsm::node::Kind::LOCAL> _asWsm;
};

template<>
struct TNode<node::Kind::FUN_CALL>
{
    node::TIndex<node::Kind::IDENTIFIER> _callee;
    node::TRange<node::Kind::ANY> _arguments;
};

} // namespace dmit::ast
