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
    com::blitDefault(lexeme._token);
}

void Builder::makeReturn(dmit::prs::Reader& reader,
                         TNode<node::Kind::STM_RETURN>& stmReturn)
{
    makeExpression(reader, stmReturn._expression);
}

void Builder::makeDclVariable(dmit::prs::Reader& reader,
                              TNode<node::Kind::DCL_VARIABLE>& dclVariable)
{
    makeTypeClaim(reader, _nodePool.makeGet(dclVariable._typeClaim));

    dclVariable._status = node::Status::ASTED;
}

void Builder::makeType(const dmit::prs::Reader& reader,
                       TNode<node::Kind::TYPE>& type)
{
    makeIdentifier(reader, _nodePool.makeGet(type._name));
}

void Builder::makeTypeClaim(dmit::prs::Reader& reader,
                            TNode<node::Kind::TYPE_CLAIM>& typeClaim)
{
    // Type
    makeType(reader, _nodePool.makeGet(typeClaim._type));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Variable
    makeIdentifier(reader, _nodePool.makeGet(typeClaim._variable));
}

void Builder::makeDeclaration(const dmit::prs::Reader& supReader,
                              Declaration& declaration)
{
    node::TIndex<node::Kind::DCL_VARIABLE> dclVariable;
    auto reader = supReader.makeSubReader();
    makeDclVariable(reader, _nodePool.makeGet(dclVariable));
    com::blit(dclVariable, declaration);
}

void Builder::makeStatement(const dmit::prs::Reader& reader,
                            Statement& statement)
{
    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::STM_RETURN)
    {
        node::TIndex<node::Kind::STM_RETURN> stmReturn;
        auto subReader = reader.makeSubReader();
        makeReturn(subReader, _nodePool.makeGet(stmReturn));
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
    binop._status = node::Status::ASTED;

    // RHS
    makeExpression(reader, binop._rhs);
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Operator
    makeLexeme(reader, _nodePool.makeGet(binop._operator));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // LHS
    if (!reader.isValidNext())
    {
        return makeExpression(reader, binop._lhs);
    }

    binop._lhs = node::TIndex<node::Kind::EXP_BINOP>{};
    auto& binopLhs = std::get<node::TIndex<node::Kind::EXP_BINOP>>(binop._lhs);
    makeBinop(reader, _nodePool.makeGet(binopLhs));
}

void Builder::makeAssignment(dmit::prs::Reader& reader,
                             TNode<node::Kind::EXP_BINOP>& expAssign)
{
    expAssign._status = node::Status::ASTED;

    // RHS
    makeExpression(reader, expAssign._rhs);
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Operator
    makeLexeme(reader, _nodePool.makeGet(expAssign._operator));
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // LHS
    makePattern(reader, expAssign._lhs);
}

void Builder::makePattern(const dmit::prs::Reader& reader,
                          Expression& expression)
{
    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::IDENTIFIER)
    {
        node::TIndex<node::Kind::PATTERN> pattern;
        makeIdentifier(reader, _nodePool.makeGet(_nodePool.makeGet(pattern)._variable));
        com::blit(pattern, expression);
    }
    else
    {
        DMIT_COM_ASSERT(!"[AST] Unknown pattern node");
    }
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

    makeIdentifier(reader, _nodePool.makeGet(funCall._callee));
}

void Builder::makeExpression(const dmit::prs::Reader& reader,
                             Expression& expression)
{
    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::IDENTIFIER)
    {
        node::TIndex<node::Kind::IDENTIFIER> identifier;
        makeIdentifier(reader, _nodePool.makeGet(identifier));
        com::blit(identifier, expression);
    }
    else if (parseNodeKind == ParseNodeKind::LIT_INTEGER)
    {
        node::TIndex<node::Kind::LIT_INTEGER> integer;
        makeInteger(reader, _nodePool.makeGet(integer));
        com::blit(integer, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_BINOP)
    {
        node::TIndex<node::Kind::EXP_BINOP> binop;
        auto subReader = reader.makeSubReader();
        makeBinop(subReader, _nodePool.makeGet(binop));
        com::blit(binop, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_ASSIGN)
    {
        node::TIndex<node::Kind::EXP_BINOP> assignment;
        auto subReader = reader.makeSubReader();
        makeAssignment(subReader, _nodePool.makeGet(assignment));
        com::blit(assignment, expression);
    }
    else if (parseNodeKind == ParseNodeKind::FUN_CALL)
    {
        node::TIndex<node::Kind::FUN_CALL> funCall;
        auto subReader = reader.makeSubReader();
        makeFunctionCall(subReader, _nodePool.makeGet(funCall));
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
        makeScope(reader, _nodePool.makeGet(subScope));
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
    auto reader = supReader.makeSubReader();

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
    makeLexeme(reader, _nodePool.makeGet(integer._lexeme));
}

void Builder::makeIdentifier(const dmit::prs::Reader& reader,
                             TNode<node::Kind::IDENTIFIER>& identifier)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::IDENTIFIER);
    makeLexeme(reader, _nodePool.makeGet(identifier._lexeme));

    identifier._status = node::Status::ASTED;
}

void Builder::makeMembers(const dmit::prs::Reader& supReader,
                          TNode<node::Kind::DEF_CLASS>& defClass)
{
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
    auto reader = supReader.makeSubReader();

    // Members
    makeMembers(reader, defClass);
    reader.advance();

    // Name
    makeIdentifier(reader, _nodePool.makeGet(defClass._name));

    defClass._status = node::Status::ASTED;
}

void Builder::makeFunction(const dmit::prs::Reader& supReader,
                           TNode<node::Kind::DEF_FUNCTION>& function)
{
    auto reader = supReader.makeSubReader();

    // Body
    if (reader.look()._kind == prs::state::tree::node::Kind::SCOPE)
    {
        makeScope(reader, _nodePool.makeGet(function._body));
        reader.advance();
    }
    else
    {
        com::blitDefault(function._body);
    }

    // Return type
    if (reader.look()._kind == prs::state::tree::node::Kind::IDENTIFIER)
    {
        makeType(reader, _nodePool.makeGet(function._returnType));
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
    makeIdentifier(reader, _nodePool.makeGet(function._name));

    function._status = node::Status::ASTED;

    com::blitDefault(function._asWsm);
}

void Builder::makeImport(const dmit::prs::Reader& supReader,
                         TNode<node::Kind::DCL_IMPORT>& import)
{
    makeExpression(supReader.makeSubReader(), import._path);

    import._status = node::Status::ASTED;
}

void Builder::makeDefinition(const dmit::prs::Reader& supReader,
                             TNode<node::Kind::DEFINITION>& definition)
{
    auto reader = supReader.makeSubReader();

    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::DEF_CLASS)
    {
        node::TIndex<node::Kind::DEF_CLASS> type;
        makeClass(reader, _nodePool.makeGet(type));
        com::blit(type, definition._value);
    }
    else if (parseNodeKind == ParseNodeKind::DEF_FUNCTION)
    {
        node::TIndex<node::Kind::DEF_FUNCTION> function;
        makeFunction(reader, _nodePool.makeGet(function));
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

    _nodePool.make(module._parentPath);
    _nodePool.make(module._definitions , reader.size());
    _nodePool.make(module._imports     , reader.size());

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
    _nodePool.make(module._modules     , indexModule      );

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

    _nodePool.make(_state._source);

    makeModule(reader, _nodePool.makeGet(_state._module));

    return _state;
}

} // namespace state
} // namespace dmit::ast
