#include "dmit/ast/state.hpp"

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
           parseNodeKind == dmit::prs::state::tree::node::Kind::LIT_IDENTIFIER;
}

dmit::prs::Reader makeSubReaderFor(const dmit::prs::state::tree::node::Kind parseNodeKind,
                                   const dmit::prs::Reader& supReader)
{
    DMIT_COM_ASSERT(supReader.look()._kind == parseNodeKind);
    return supReader.makeSubReader();
}

} // namespace

State::State(std::shared_ptr<NodePool>& nodePool) : _nodePool{nodePool} {}

namespace state
{

Builder::Builder() : _nodePool{std::make_shared<State::NodePool>()}, _state{_nodePool} {}

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
    _nodePool->make(expAssign._operator);
    makeLexeme(reader, _nodePool->get(expAssign._operator));
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
    _nodePool->make(dclVariable._typeClaim);
    makeTypeClaim(reader, _nodePool->get(dclVariable._typeClaim));
}

void Builder::makeTypeClaim(dmit::prs::Reader& reader,
                            TNode<node::Kind::TYPE_CLAIM>& typeClaim)
{
    // Type
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool->make(typeClaim._type);
    makeIdentifier(reader, _nodePool->get(typeClaim._type));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Variable
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool->make(typeClaim._variable);
    makeIdentifier(reader, _nodePool->get(typeClaim._variable));
}

void Builder::makeDeclaration(const dmit::prs::Reader& supReader,
                              Declaration& declaration)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DCL_VARIABLE, supReader);

    node::TIndex<node::Kind::DCL_VARIABLE> dclVariable;
    _nodePool->make(dclVariable);
    makeDclVariable(reader, _nodePool->get(dclVariable));
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
        _nodePool->make(stmReturn);
        makeReturn(subReader, _nodePool->get(stmReturn));
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
    _nodePool->make(binop._operator);
    makeLexeme(reader, _nodePool->get(binop._operator));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // LHS
    if (!reader.isValidNext())
    {
        return makeExpression(reader, binop._lhs);
    }

    binop._lhs = node::TIndex<node::Kind::EXP_BINOP>{};
    auto& binopLhs = std::get<node::TIndex<node::Kind::EXP_BINOP>>(binop._lhs);
    _nodePool->make(binopLhs);
    makeBinop(reader, _nodePool->get(binopLhs));
}

void Builder::makeFunctionCall(dmit::prs::Reader& reader,
                               TNode<node::Kind::FUN_CALL>& funCall)
{
    _nodePool->make(funCall._arguments, reader.size() - 1);

    uint32_t i = funCall._arguments._size;

    while (reader.isValidNext())
    {
        makeExpression(reader, _nodePool->get(funCall._arguments[--i])._value);
        reader.advance();
    }

    _nodePool->make(funCall._callee);
    makeIdentifier(reader, _nodePool->get(funCall._callee));
}

void Builder::makeExpression(const dmit::prs::Reader& reader,
                             Expression& expression)
{
    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::LIT_IDENTIFIER)
    {
        node::TIndex<node::Kind::LIT_IDENTIFIER> identifier;
        _nodePool->make(identifier);
        makeIdentifier(reader, _nodePool->get(identifier));
        com::blit(identifier, expression);
    }
    else if (parseNodeKind == ParseNodeKind::LIT_INTEGER)
    {
        node::TIndex<node::Kind::LIT_INTEGER> integer;
        _nodePool->make(integer);
        makeInteger(reader, _nodePool->get(integer));
        com::blit(integer, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_BINOP)
    {
        node::TIndex<node::Kind::EXP_BINOP> binop;
        _nodePool->make(binop);
        auto subReader = makeSubReaderFor(ParseNodeKind::EXP_BINOP, reader);
        makeBinop(subReader, _nodePool->get(binop));
        com::blit(binop, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_ASSIGN)
    {
        node::TIndex<node::Kind::EXP_BINOP> assignment;
        _nodePool->make(assignment);
        auto subReader = makeSubReaderFor(ParseNodeKind::EXP_ASSIGN, reader);
        makeAssignment(subReader, _nodePool->get(assignment));
        com::blit(assignment, expression);
    }
    else if (parseNodeKind == ParseNodeKind::FUN_CALL)
    {
        node::TIndex<node::Kind::FUN_CALL> funCall;
        _nodePool->make(funCall);
        auto subReader = makeSubReaderFor(ParseNodeKind::FUN_CALL, reader);
        makeFunctionCall(subReader, _nodePool->get(funCall));
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
        _nodePool->make(subScope);
        makeScope(reader, _nodePool->get(subScope));
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

    _nodePool->make(scope._variants, reader.size());

    uint32_t i = scope._variants._size;

    while (reader.isValid())
    {
        makeScopeVariant(reader, _nodePool->get(scope._variants[--i]));
        reader.advance();
    }
}

void Builder::makeArguments(const dmit::prs::Reader& supReader,
                            TNode<node::Kind::FUN_DEFINITION>& function)
{
    DMIT_COM_ASSERT(supReader.look()._kind == ParseNodeKind::FUN_ARGUMENTS);
    auto reader = supReader.makeSubReader();

    _nodePool->make(function._arguments, reader.size() >> 1);

    uint32_t i = function._arguments._size;

    while (reader.isValid())
    {
        makeTypeClaim(reader, _nodePool->get(function._arguments[--i]));
        reader.advance();
    }
}

void Builder::makeInteger(const dmit::prs::Reader& reader,
                          TNode<node::Kind::LIT_INTEGER>& integer)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_INTEGER);
    _nodePool->make(integer._lexeme);
    makeLexeme(reader, _nodePool->get(integer._lexeme));
}

void Builder::makeIdentifier(const dmit::prs::Reader& reader,
                             TNode<node::Kind::LIT_IDENTIFIER>& identifier)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool->make(identifier._lexeme);
    makeLexeme(reader, _nodePool->get(identifier._lexeme));
}

void Builder::makeFunction(const dmit::prs::Reader& supReader,
                           TNode<node::Kind::FUN_DEFINITION>& function)
{
    auto reader = makeSubReaderFor(ParseNodeKind::FUN_DEFINITION, supReader);

    // Body
    _nodePool->make(function._body);
    makeScope(reader, _nodePool->get(function._body));
    reader.advance();

    // Return type
    if (reader.look()._kind == dmit::prs::state::tree::node::Kind::LIT_IDENTIFIER)
    {
        _nodePool->make(function._returnType);
        makeIdentifier(reader, _nodePool->get(function._returnType));
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
    _nodePool->make(function._name);
    makeIdentifier(reader, _nodePool->get(function._name));

    // Status
    (reader.isValidNext()) ? com::blit(FunctionStatus::EXPORTED , function._status)
                           : com::blit(FunctionStatus::LOCAL    , function._status);
}

void Builder::makeImport(const dmit::prs::Reader& supReader,
                         TNode<node::Kind::DCL_IMPORT>& import)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DCL_IMPORT, supReader);

    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool->make(import._moduleName);
    makeIdentifier(reader, _nodePool->get(import._moduleName));
}

void Builder::makeModule(dmit::prs::Reader& reader,
                         TNode<node::Kind::MODULE>& module)
{
    auto readerCopy = reader;

    _nodePool->make(module._functions , reader.size());
    _nodePool->make(module._imports   , reader.size());
    _nodePool->make(module._modules   , reader.size());

    uint32_t indexFunction = 0;
    uint32_t indexImport   = 0;
    uint32_t indexModule   = 0;

    while (reader.isValid())
    {
        const auto parseNodeKind = reader.look()._kind;

        if (parseNodeKind == dmit::prs::state::tree::node::Kind::LIT_IDENTIFIER)
        {
            _nodePool->make(module._name);
            makeIdentifier(reader, _nodePool->get(module._name));
        }
        else if (parseNodeKind == dmit::prs::state::tree::node::Kind::FUN_DEFINITION)
        {
            makeFunction(reader, _nodePool->get(module._functions[indexFunction++]));
        }
        else if (parseNodeKind == dmit::prs::state::tree::node::Kind::DCL_IMPORT)
        {
            makeImport(reader, _nodePool->get(module._imports[indexImport++]));
        }
        else if (parseNodeKind == dmit::prs::state::tree::node::Kind::MODULE)
        {
            indexModule++;
        }
        else
        {
            DMIT_COM_ASSERT(!"[AST] Unknown module element");
        }

        reader.advance();
    }

    _nodePool->trim(module._functions , indexFunction );
    _nodePool->trim(module._imports   , indexImport   );
    _nodePool->trim(module._modules   , indexModule   );

    while (readerCopy.isValid())
    {
        if (readerCopy.look()._kind == dmit::prs::state::tree::node::Kind::MODULE)
        {
            auto subReader = readerCopy.makeSubReader();
            makeModule(subReader, _nodePool->get(module._modules[--indexModule]));
        }

        readerCopy.advance();
    }
}

State& Builder::operator()(const prs::state::Tree& parseTree)
{
    dmit::prs::Reader reader{parseTree};

    _nodePool->make(_state._module);
    _nodePool->make(_state._source);

    auto& rootModule = _nodePool->get(_state._module);

    com::blitDefault(rootModule._name); // root module shall not be named

    makeModule(reader, rootModule);

    return _state;
}

} // namespace state
} // namespace dmit::ast
