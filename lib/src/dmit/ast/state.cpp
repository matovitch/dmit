#include "dmit/ast/state.hpp"

#include "dmit/ast/definition_role.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/com/assert.hpp"
#include "dmit/com/blit.hpp"

#include <type_traits>
#include <cstdint>
#include <variant>
#include <cstring>
#include <memory>

namespace dmit::ast
{

namespace
{

bool isDeclaration(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::DCL_VARIABLE;
}

bool isStatement(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::STM_RETURN;
}

bool isExpression(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::FUN_CALL    ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::EXP_BINOP   ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::EXP_ASSIGN  ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::LIT_INTEGER ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::IDENTIFIER;
}

dmit::prs::Reader makeSubReaderFor(const dmit::prs::state::tree::node::Kind parseNodeKind,
                                   const dmit::prs::Reader& supReader)
{
    DMIT_COM_ASSERT(supReader.look()._kind == parseNodeKind);
    return supReader.makeSubReader();
}

} // namespace

State::State(NodePool& nodePool) : _nodePool{nodePool} {}

namespace state
{

Builder::Builder() : _state{_nodePool} {}

void Builder::makeLexeme(const dmit::prs::Reader& reader,
                         TNode<node::Kind::LEXEME>& lexeme)
{
    lexeme._index  = reader.look()._start;
    lexeme._source = _state._source;
}

void Builder::makeReturn(dmit::prs::Reader& reader,
                         TNode<node::Kind::STM_RETURN>& stmReturn)
{
    makeExpression(reader, stmReturn._expression);
}

void Builder::makeDclVariable(dmit::prs::Reader& reader,
                              TNode<node::Kind::DCL_VARIABLE>& dclVariable)
{
    _nodePool.make(dclVariable._typeClaim);
    makeTypeClaim(reader, _nodePool.get(dclVariable._typeClaim));

    dclVariable._status = node::Status::ASTED;
}

void Builder::makeType(const dmit::prs::Reader& reader,
                       TNode<node::Kind::TYPE>& type)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::IDENTIFIER);
    _nodePool.make(type._name);
    makeIdentifier(reader, _nodePool.get(type._name));
}

void Builder::makeTypeClaim(dmit::prs::Reader& reader,
                            TNode<node::Kind::TYPE_CLAIM>& typeClaim)
{
    // Type
    _nodePool.make(typeClaim._type);
    makeType(reader, _nodePool.get(typeClaim._type));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Variable
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::IDENTIFIER);
    _nodePool.make(typeClaim._variable);
    makeIdentifier(reader, _nodePool.get(typeClaim._variable));
}

void Builder::makeDeclaration(const dmit::prs::Reader& supReader,
                              Declaration& declaration)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DCL_VARIABLE, supReader);

    node::TIndex<node::Kind::DCL_VARIABLE> dclVariable;
    _nodePool.make(dclVariable);
    makeDclVariable(reader, _nodePool.get(dclVariable));
    com::blit(dclVariable, declaration);
}

void Builder::makeStatement(const dmit::prs::Reader& reader,
                            Statement& statement)
{
    auto subReader = reader.makeSubReader();
    DMIT_COM_ASSERT(subReader.isValid());

    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::STM_RETURN)
    {
        node::TIndex<node::Kind::STM_RETURN> stmReturn;
        _nodePool.make(stmReturn);
        makeReturn(subReader, _nodePool.get(stmReturn));
        com::blit(stmReturn, statement);
    }
    else
    {
        DMIT_COM_ASSERT(!"[AST] Unknown statement node");
    }
}

void Builder::makeBinop(dmit::prs::Reader& reader,
                        TNode<node::Kind::EXP_BINOP>& binop)
{
    // RHS
    makeExpression(reader, binop._rhs);
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Operator
    _nodePool.make(binop._operator);
    makeLexeme(reader, _nodePool.get(binop._operator));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // LHS
    if (!reader.isValidNext())
    {
        return makeExpression(reader, binop._lhs);
    }

    binop._lhs = node::TIndex<node::Kind::EXP_BINOP>{};
    auto& binopLhs = std::get<node::TIndex<node::Kind::EXP_BINOP>>(binop._lhs);
    _nodePool.make(binopLhs);
    makeBinop(reader, _nodePool.get(binopLhs));
}

void Builder::makeAssignment(dmit::prs::Reader& reader,
                             TNode<node::Kind::EXP_BINOP>& expAssign)
{
    // RHS
    makeExpression(reader, expAssign._rhs);
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Operator
    _nodePool.make(expAssign._operator);
    makeLexeme(reader, _nodePool.get(expAssign._operator));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // LHS
    makeExpression(reader, expAssign._lhs);
}

void Builder::makeFunctionCall(dmit::prs::Reader& reader,
                               TNode<node::Kind::FUN_CALL>& funCall)
{
    _nodePool.make(funCall._arguments, reader.size() - 1);

    uint32_t i = funCall._arguments._size;

    while (reader.isValidNext())
    {
        makeExpression(reader, _nodePool.get(funCall._arguments[--i])._value);
        reader.advance();
    }

    _nodePool.make(funCall._callee);
    makeIdentifier(reader, _nodePool.get(funCall._callee));
}

void Builder::makeExpression(const dmit::prs::Reader& reader,
                             Expression& expression)
{
    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::IDENTIFIER)
    {
        node::TIndex<node::Kind::IDENTIFIER> identifier;
        _nodePool.make(identifier);
        makeIdentifier(reader, _nodePool.get(identifier));
        com::blit(identifier, expression);
    }
    else if (parseNodeKind == ParseNodeKind::LIT_INTEGER)
    {
        node::TIndex<node::Kind::LIT_INTEGER> integer;
        _nodePool.make(integer);
        makeInteger(reader, _nodePool.get(integer));
        com::blit(integer, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_BINOP)
    {
        node::TIndex<node::Kind::EXP_BINOP> binop;
        _nodePool.make(binop);
        auto subReader = makeSubReaderFor(ParseNodeKind::EXP_BINOP, reader);
        makeBinop(subReader, _nodePool.get(binop));
        com::blit(binop, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_ASSIGN)
    {
        node::TIndex<node::Kind::EXP_BINOP> assignment;
        _nodePool.make(assignment);
        auto subReader = makeSubReaderFor(ParseNodeKind::EXP_ASSIGN, reader);
        makeAssignment(subReader, _nodePool.get(assignment));
        com::blit(assignment, expression);
    }
    else if (parseNodeKind == ParseNodeKind::FUN_CALL)
    {
        node::TIndex<node::Kind::FUN_CALL> funCall;
        _nodePool.make(funCall);
        auto subReader = makeSubReaderFor(ParseNodeKind::FUN_CALL, reader);
        makeFunctionCall(subReader, _nodePool.get(funCall));
        com::blit(funCall, expression);
    }
    else
    {
        DMIT_COM_ASSERT(!"[AST] Unknown expression node");
    }
}

void Builder::makeScopeVariant(const dmit::prs::Reader& reader,
                               TNode<node::Kind::SCOPE_VARIANT>& scopeVariant)
{
    auto parseNodeKind = reader.look()._kind;

    if (isDeclaration(parseNodeKind))
    {
        com::blit(Declaration{}, scopeVariant._value);
        makeDeclaration(reader, std::get<Declaration>(scopeVariant._value));
    }
    else if (isStatement(parseNodeKind))
    {
        com::blit(Statement{}, scopeVariant._value);
        makeStatement(reader, std::get<Statement>(scopeVariant._value));
    }
    else if (isExpression(parseNodeKind))
    {
        com::blit(Expression{}, scopeVariant._value);
        makeExpression(reader, std::get<Expression>(scopeVariant._value));
    }
    else if (parseNodeKind == ParseNodeKind::SCOPE)
    {
        node::TIndex<node::Kind::SCOPE> subScope;
        _nodePool.make(subScope);
        makeScope(reader, _nodePool.get(subScope));
        com::blit(subScope, scopeVariant._value);
    }
    else
    {
        DMIT_COM_ASSERT(!"[AST] Unknown scope variant node");
    }
}

void Builder::makeScope(const dmit::prs::Reader& supReader,
                        TNode<node::Kind::SCOPE>& scope)
{
    auto reader = makeSubReaderFor(ParseNodeKind::SCOPE, supReader);

    _nodePool.make(scope._variants, reader.size());

    uint32_t i = scope._variants._size;

    while (reader.isValid())
    {
        makeScopeVariant(reader, _nodePool.get(scope._variants[--i]));
        reader.advance();
    }
}

void Builder::makeArguments(const dmit::prs::Reader& supReader,
                            TNode<node::Kind::DEF_FUNCTION>& function)
{
    DMIT_COM_ASSERT(supReader.look()._kind == ParseNodeKind::FUN_ARGUMENTS);
    auto reader = supReader.makeSubReader();

    _nodePool.make(function._arguments, reader.size() >> 1);

    uint32_t i = function._arguments._size;

    while (reader.isValid())
    {
        makeDclVariable(reader, _nodePool.get(function._arguments[--i]));
        reader.advance();
    }
}

void Builder::makeInteger(const dmit::prs::Reader& reader,
                          TNode<node::Kind::LIT_INTEGER>& integer)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_INTEGER);
    _nodePool.make(integer._lexeme);
    makeLexeme(reader, _nodePool.get(integer._lexeme));
}

void Builder::makeIdentifier(const dmit::prs::Reader& reader,
                             TNode<node::Kind::IDENTIFIER>& identifier)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::IDENTIFIER);
    _nodePool.make(identifier._lexeme);
    makeLexeme(reader, _nodePool.get(identifier._lexeme));

    identifier._status = node::Status::ASTED;
}

void Builder::makeMembers(const dmit::prs::Reader& supReader,
                          TNode<node::Kind::DEF_CLASS>& defClass)
{
    DMIT_COM_ASSERT(supReader.look()._kind == ParseNodeKind::CLS_MEMBERS);
    auto reader = supReader.makeSubReader();

    _nodePool.make(defClass._members, reader.size() >> 1);

    uint32_t i = defClass._members._size;

    while (reader.isValid())
    {
        makeTypeClaim(reader, _nodePool.get(defClass._members[--i]));
        reader.advance();
    }
}

void Builder::makeClass(const dmit::prs::Reader& supReader,
                        TNode<node::Kind::DEF_CLASS>& defClass)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DEF_CLASS, supReader);

    // Members
    makeMembers(reader, defClass);
    reader.advance();

    // Name
    _nodePool.make(defClass._name);
    makeIdentifier(reader, _nodePool.get(defClass._name));

    defClass._status = node::Status::ASTED;
}

void Builder::makeFunction(const dmit::prs::Reader& supReader,
                           TNode<node::Kind::DEF_FUNCTION>& function)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DEF_FUNCTION, supReader);

    // Body
    _nodePool.make(function._body);
    makeScope(reader, _nodePool.get(function._body));
    reader.advance();

    // Return type
    if (reader.look()._kind == dmit::prs::state::tree::node::Kind::IDENTIFIER)
    {
        _nodePool.make(function._returnType);
        makeType(reader, _nodePool.get(function._returnType));
        reader.advance();
    }
    else
    {
        com::blitDefault(function._returnType);
    }

    // Arguments
    makeArguments(reader, function);
    reader.advance();

    // Name
    _nodePool.make(function._name);
    makeIdentifier(reader, _nodePool.get(function._name));

    function._status = node::Status::ASTED;
}

void Builder::makeImport(const dmit::prs::Reader& supReader,
                         TNode<node::Kind::DCL_IMPORT>& import)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DCL_IMPORT, supReader);

    makeExpression(reader, import._path);

    import._status = node::Status::ASTED;
}

void Builder::makeDefinition(const dmit::prs::Reader& supReader,
                             TNode<node::Kind::DEFINITION>& definition)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DEFINITION, supReader);

    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::DEF_CLASS)
    {
        node::TIndex<node::Kind::DEF_CLASS> type;
        _nodePool.make(type);
        makeClass(reader, _nodePool.get(type));
        com::blit(type, definition._value);
    }
    else if (parseNodeKind == ParseNodeKind::DEF_FUNCTION)
    {
        node::TIndex<node::Kind::DEF_FUNCTION> function;
        _nodePool.make(function);
        makeFunction(reader, _nodePool.get(function));
        com::blit(function, definition._value);
    }
    else
    {
        DMIT_COM_ASSERT(!"[AST] Unknown definition kind");
    }

    // Status
    (reader.isValidNext()) ? com::blit(DefinitionRole::EXPORTED , definition._role)
                           : com::blit(DefinitionRole::LOCAL    , definition._role);

}

void Builder::makeModule(dmit::prs::Reader& reader,
                         TNode<node::Kind::MODULE>& module)
{
    auto readerCopy = reader;

    com::blitDefault(module._path);
    com::blitDefault(module._parentPath);

    _nodePool.make(module._definitions , reader.size());
    _nodePool.make(module._imports     , reader.size());
    _nodePool.make(module._modules     , reader.size());

    uint32_t indexDefinitions = 0;
    uint32_t indexImport      = 0;
    uint32_t indexModule      = 0;

    while (reader.isValid())
    {
        const auto parseNodeKind = reader.look()._kind;

        if (parseNodeKind == dmit::prs::state::tree::node::Kind::IDENTIFIER ||
            parseNodeKind == dmit::prs::state::tree::node::Kind::EXP_BINOP)
        {
            module._path = node::TIndex<node::Kind::FUN_CALL>{0};
            makeExpression(reader, module._path.value());
        }
        else if (parseNodeKind == ParseNodeKind::DEFINITION)
        {
            makeDefinition(reader, _nodePool.get(module._definitions[indexDefinitions++]));
        }
        else if (parseNodeKind == ParseNodeKind::DCL_IMPORT)
        {
            makeImport(reader, _nodePool.get(module._imports[indexImport++]));
        }
        else if (parseNodeKind == ParseNodeKind::MODULE)
        {
            indexModule++;
        }
        else
        {
            DMIT_COM_ASSERT(!"[AST] Unknown module element");
        }

        reader.advance();
    }

    _nodePool.trim(module._definitions , indexDefinitions );
    _nodePool.trim(module._imports     , indexImport      );
    _nodePool.trim(module._modules     , indexModule      );

    while (readerCopy.isValid())
    {
        if (readerCopy.look()._kind == dmit::prs::state::tree::node::Kind::MODULE)
        {
            auto subReader = readerCopy.makeSubReader();
            makeModule(subReader, _nodePool.get(module._modules[--indexModule]));
        }

        readerCopy.advance();
    }

    module._status = node::Status::ASTED;
}

State& Builder::operator()(const prs::state::Tree& parseTree)
{
    dmit::prs::Reader reader{parseTree};

    _nodePool.make(_state._module);
    _nodePool.make(_state._source);

    auto& rootModule = _nodePool.get(_state._module);

    makeModule(reader, rootModule);

    return _state;
}

} // namespace state
} // namespace dmit::ast
