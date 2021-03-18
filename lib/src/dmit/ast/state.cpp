#include "dmit/ast/state.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/com/assert.hpp"

#include <type_traits>
#include <cstdint>
#include <variant>
#include <cstring>

namespace dmit::ast::state
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
           parseNodeKind == dmit::prs::state::tree::node::Kind::LIT_IDENTIFIER;
}

template <class Variant, class BlipType>
void blitVariant(const BlipType& toBlip, Variant& variant)
{
    const Variant toBlipAsVariant = toBlip;
    std::memcpy(&variant, &toBlipAsVariant, sizeof(Variant));
}

dmit::prs::Reader makeSubReaderFor(const dmit::prs::state::tree::node::Kind parseNodeKind,
                                   const dmit::prs::Reader& supReader)
{
    DMIT_COM_ASSERT(supReader.look()._kind == parseNodeKind);
    return supReader.makeSubReader();
}

} // namespace

Builder::Builder() : _state{}, _nodePool{_state._nodePool} {}

void Builder::makeLexeme(const dmit::prs::Reader& reader,
                         TNode<node::Kind::LEXEME>& lexeme)
{
    lexeme._index  = reader.look()._start;
    lexeme._source = _state._source;
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
}

void Builder::makeTypeClaim(dmit::prs::Reader& reader,
                            TNode<node::Kind::TYPE_CLAIM>& typeClaim)
{
    // Type
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool.make(typeClaim._type);
    makeIdentifier(reader, _nodePool.get(typeClaim._type));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Variable
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
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
    blitVariant(dclVariable, declaration);
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
        blitVariant(stmReturn, statement);
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

void Builder::makeFunctionCall(dmit::prs::Reader& reader,
                               TNode<node::Kind::FUN_CALL>& funCall)
{
    _nodePool.make(reader.size() - 1, funCall._arguments);

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

    if (parseNodeKind == ParseNodeKind::LIT_IDENTIFIER)
    {
        node::TIndex<node::Kind::LIT_IDENTIFIER> identifier;
        _nodePool.make(identifier);
        makeIdentifier(reader, _nodePool.get(identifier));
        blitVariant(identifier, expression);
    }
    else if (parseNodeKind == ParseNodeKind::LIT_INTEGER)
    {
        node::TIndex<node::Kind::LIT_INTEGER> integer;
        _nodePool.make(integer);
        makeInteger(reader, _nodePool.get(integer));
        blitVariant(integer, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_BINOP)
    {
        node::TIndex<node::Kind::EXP_BINOP> binop;
        _nodePool.make(binop);
        auto subReader = makeSubReaderFor(ParseNodeKind::EXP_BINOP, reader);
        makeBinop(subReader, _nodePool.get(binop));
        blitVariant(binop, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_ASSIGN)
    {
        node::TIndex<node::Kind::EXP_BINOP> assignment;
        _nodePool.make(assignment);
        auto subReader = makeSubReaderFor(ParseNodeKind::EXP_ASSIGN, reader);
        makeAssignment(subReader, _nodePool.get(assignment));
        blitVariant(assignment, expression);
    }
    else if (parseNodeKind == ParseNodeKind::FUN_CALL)
    {
        node::TIndex<node::Kind::FUN_CALL> funCall;
        _nodePool.make(funCall);
        auto subReader = makeSubReaderFor(ParseNodeKind::FUN_CALL, reader);
        makeFunctionCall(subReader, _nodePool.get(funCall));
        blitVariant(funCall, expression);
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
        blitVariant(Declaration{}, scopeVariant._value);
        makeDeclaration(reader, std::get<Declaration>(scopeVariant._value));
    }
    else if (isStatement(parseNodeKind))
    {
        blitVariant(Statement{}, scopeVariant._value);
        makeStatement(reader, std::get<Statement>(scopeVariant._value));
    }
    else if (isExpression(parseNodeKind))
    {
        blitVariant(Expression{}, scopeVariant._value);
        makeExpression(reader, std::get<Expression>(scopeVariant._value));
    }
    else if (parseNodeKind == ParseNodeKind::SCOPE)
    {
        node::TIndex<node::Kind::SCOPE> subScope;
        _nodePool.make(subScope);
        makeScope(reader, _nodePool.get(subScope));
        blitVariant(subScope, scopeVariant._value);
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

    _nodePool.make(reader.size(), scope._variants);

    uint32_t i = scope._variants._size;

    while (reader.isValid())
    {
        makeScopeVariant(reader, _nodePool.get(scope._variants[--i]));
        reader.advance();
    }
}

void Builder::makeReturnType(const dmit::prs::Reader& supReader,
                             TNode<node::Kind::FUN_RETURN>& returnType)
{
    auto reader = makeSubReaderFor(ParseNodeKind::FUN_RETURN, supReader);

    if (!reader.isValid())
    {
        blitVariant(std::nullopt, returnType._option);
        return;
    }

    node::TIndex<node::Kind::LIT_IDENTIFIER> identifier;
    _nodePool.make(identifier);
    makeIdentifier(reader, _nodePool.get(identifier));
    blitVariant(identifier, returnType._option);
}

void Builder::makeArguments(const dmit::prs::Reader& supReader,
                            TNode<node::Kind::FUN_DEFINITION>& function)
{
    DMIT_COM_ASSERT(supReader.look()._kind == ParseNodeKind::FUN_ARGUMENTS);
    auto reader = supReader.makeSubReader();

    _nodePool.make(reader.size() >> 1, function._arguments);

    uint32_t i = function._arguments._size;

    while (reader.isValid())
    {
        makeTypeClaim(reader, _nodePool.get(function._arguments[--i]));
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
                             TNode<node::Kind::LIT_IDENTIFIER>& identifier)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool.make(identifier._lexeme);
    makeLexeme(reader, _nodePool.get(identifier._lexeme));
}

void Builder::makeFunction(const dmit::prs::Reader& supReader,
                           TNode<node::Kind::FUN_DEFINITION>& function)
{
    auto reader = makeSubReaderFor(ParseNodeKind::FUN_DEFINITION, supReader);

    // Body
    _nodePool.make(function._body);
    makeScope(reader, _nodePool.get(function._body));
    reader.advance();

    // Return type
    _nodePool.make(function._returnType);
    makeReturnType(reader, _nodePool.get(function._returnType));
    reader.advance();

    // Arguments
    makeArguments(reader, function);
    reader.advance();

    // Name
    _nodePool.make(function._name);
    makeIdentifier(reader, _nodePool.get(function._name));
}

State& Builder::operator()(const prs::state::Tree& parseTree)
{
    dmit::prs::Reader reader{parseTree};

    _nodePool.make(_state._program);
    _nodePool.make(_state._source);

    auto& program = _nodePool.get(_state._program);

    _nodePool.make(reader.size(), program._functions);

    uint32_t i = program._functions._size;

    while (reader.isValid())
    {
        makeFunction(reader, _nodePool.get(program._functions[--i]));
        reader.advance();
    }

    return _state;
}

} // namespace dmit::ast::state
