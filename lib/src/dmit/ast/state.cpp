#include "dmit/ast/state.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/com/assert.hpp"

#include <type_traits>
#include <cstdint>
#include <variant>
#include <cstring>

namespace dmit
{

namespace ast
{

namespace state
{

namespace
{

bool isDeclaration(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::DCL_VARIABLE;
}

bool isStatement(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::STM_ASSIGN ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::STM_RETURN;
}

bool isExpression(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::FUN_CALL    ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::EXP_BINOP   ||
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
    auto readerOpt = supReader.makeSubReader();
    DMIT_COM_ASSERT(readerOpt);
    return readerOpt.value();
}

} // namespace


void Builder::makeAssignment(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& supReader,
                             TNode<node::Kind::STM_ASSIGN>& assignment)
{
    // TODO
}

void Builder::makeReturn(const prs::state::Tree& parseTree,
                               dmit::prs::Reader& reader,
                               TNode<node::Kind::STM_RETURN>& stmReturn)
{
    makeExpression(parseTree, reader, stmReturn._expression);
}

void Builder::makeDclVariable(const prs::state::Tree& parseTree,
                              dmit::prs::Reader& reader,
                              TNode<node::Kind::DCL_VARIABLE>& dclVariable)
{
    _nodePool.make(dclVariable._typeClaim);
    makeTypeClaim(parseTree, reader, _nodePool.get(dclVariable._typeClaim));
}

void Builder::makeTypeClaim(const prs::state::Tree& parseTree,
                            dmit::prs::Reader& reader,
                            TNode<node::Kind::TYPE_CLAIM>& typeClaim)
{
    // Variable
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool.make(typeClaim._variable);
    makeIdentifier(parseTree, reader, _nodePool.get(typeClaim._variable));
    reader.advance();

    // Type
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool.make(typeClaim._type);
    makeIdentifier(parseTree, reader, _nodePool.get(typeClaim._type));
    reader.advance();
}

void Builder::makeDeclaration(const prs::state::Tree& parseTree,
                              const dmit::prs::Reader& supReader,
                              Declaration& declaration)
{
    auto reader = makeSubReaderFor(ParseNodeKind::DCL_VARIABLE, supReader);

    node::TIndex<node::Kind::DCL_VARIABLE> dclVariable;
    _nodePool.make(dclVariable);
    makeDclVariable(parseTree, reader, _nodePool.get(dclVariable));
    blitVariant(dclVariable, declaration);
}

void Builder::makeStatement(const prs::state::Tree& parseTree,
                            const dmit::prs::Reader& reader,
                            Statement& statement)
{
    auto subReader = reader.makeSubReader();
    DMIT_COM_ASSERT(subReader);

    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::STM_ASSIGN)
    {
        node::TIndex<node::Kind::STM_ASSIGN> assignment;
        _nodePool.make(assignment);
        makeAssignment(parseTree, subReader.value(), _nodePool.get(assignment));
        blitVariant(assignment, statement);
    }
    else if (parseNodeKind == ParseNodeKind::STM_RETURN)
    {
        node::TIndex<node::Kind::STM_RETURN> stmReturn;
        _nodePool.make(stmReturn);
        makeReturn(parseTree, subReader.value(), _nodePool.get(stmReturn));
        blitVariant(stmReturn, statement);
    }
    else
    {
        DMIT_COM_ASSERT(false); // Should not get there
    }
}

void Builder::makeBinop(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::EXP_BINOP>& binop)
{
    // RHS
    makeExpression(parseTree, reader, binop._rhs);
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // Operator
    _nodePool.make (binop._operator);
    _nodePool.get  (binop._operator)._index = parseTree.range(reader.look())._start;
    reader.advance();
    DMIT_COM_ASSERT(reader.isValid());

    // LHS
    if (!reader.isValidNext())
    {
        return makeExpression(parseTree, reader, binop._lhs);
    }

    binop._lhs = node::TIndex<node::Kind::EXP_BINOP>{};
    auto& binopLhs = std::get<node::TIndex<node::Kind::EXP_BINOP>>(binop._lhs);
    _nodePool.make(binopLhs);
    makeBinop(parseTree, reader, _nodePool.get(binopLhs));
}

void Builder::makeExpression(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& reader,
                             Expression& expression)
{
    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::LIT_IDENTIFIER)
    {
        node::TIndex<node::Kind::LIT_IDENTIFIER> identifier;
        _nodePool.make(identifier);
        makeIdentifier(parseTree, reader, _nodePool.get(identifier));
        blitVariant(identifier, expression);
    }
    else if (parseNodeKind == ParseNodeKind::LIT_INTEGER)
    {
        node::TIndex<node::Kind::LIT_INTEGER> integer;
        _nodePool.make(integer);
        makeInteger(parseTree, reader, _nodePool.get(integer));
        blitVariant(integer, expression);
    }
    else if (parseNodeKind == ParseNodeKind::EXP_BINOP)
    {
        node::TIndex<node::Kind::EXP_BINOP> binop;
        _nodePool.make(binop);
        auto subReader = makeSubReaderFor(ParseNodeKind::EXP_BINOP, reader);
        makeBinop(parseTree, subReader, _nodePool.get(binop));
        blitVariant(binop, expression);
    }
    else
    {
        DMIT_COM_ASSERT(false); // Should not get there
    }
}

void Builder::makeScope(const prs::state::Tree& parseTree,
                        const dmit::prs::Reader& supReader,
                        TNode<node::Kind::SCOPE>& scope)
{
    auto reader = makeSubReaderFor(ParseNodeKind::SCOPE, supReader);

    _nodePool.make(reader.size(), scope._variants);

    uint32_t i = 0;

    while (reader.isValid())
    {
        auto& variant = _nodePool.get(scope._variants[i]);

        auto parseNodeKind = reader.look()._kind;

        if (isDeclaration(parseNodeKind))
        {
            blitVariant(Declaration{}, variant._value);
            makeDeclaration(parseTree, reader, std::get<Declaration>(variant._value));
        }
        else if (isStatement(parseNodeKind))
        {
            blitVariant(Statement{}, variant._value);
            makeStatement(parseTree, reader, std::get<Statement>(variant._value));
        }
        else if (isExpression(parseNodeKind))
        {
            blitVariant(Expression{}, variant._value);
            makeExpression(parseTree, reader, std::get<Expression>(variant._value));
        }
        else if (parseNodeKind == ParseNodeKind::SCOPE)
        {
            node::TIndex<node::Kind::SCOPE> subScope;
            _nodePool.make(subScope);
            makeScope(parseTree, reader, _nodePool.get(subScope));
            blitVariant(subScope, variant._value);
        }
        else
        {
            DMIT_COM_ASSERT(false); // Should not get there
        }

        reader.advance();
        i++;
    }
}

void Builder::makeReturnType(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& supReader,
                             TNode<node::Kind::FUN_RETURN>& returnType)
{
    DMIT_COM_ASSERT(supReader.look()._kind == ParseNodeKind::FUN_RETURN);
    auto readerOpt = supReader.makeSubReader();
    
    if (!readerOpt)
    {
        blitVariant(std::nullopt, returnType._option);
        return;
    }

    auto& reader = readerOpt.value();

    node::TIndex<node::Kind::LIT_IDENTIFIER> identifier;
    _nodePool.make(identifier);
    makeIdentifier(parseTree, reader, _nodePool.get(identifier));
    blitVariant(identifier, returnType._option);
}

void Builder::makeArguments(const prs::state::Tree& parseTree,
                            const dmit::prs::Reader& supReader,
                            TNode<node::Kind::FUN_ARGUMENTS>& arguments)
{
    DMIT_COM_ASSERT(supReader.look()._kind == ParseNodeKind::FUN_ARGUMENTS);
    auto readerOpt = supReader.makeSubReader();
    
    if (!readerOpt)
    {
        arguments._typeClaims._size = 0;
        return;
    }

    auto& reader = readerOpt.value();

    _nodePool.make(reader.size() >> 1, arguments._typeClaims);

    uint32_t i = 0;

    while (reader.isValid())
    {
        makeTypeClaim(parseTree, reader, _nodePool.get(arguments._typeClaims[i]));
        i++;
    }
}

void Builder::makeInteger(const prs::state::Tree& parseTree,
                          const dmit::prs::Reader& reader,
                          TNode<node::Kind::LIT_INTEGER>& integer)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_INTEGER);
    _nodePool.make(integer._lexeme);
    _nodePool.get (integer._lexeme)._index = parseTree.range(reader.look())._start;
}

void Builder::makeIdentifier(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& reader,
                             TNode<node::Kind::LIT_IDENTIFIER>& identifier)
{
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::LIT_IDENTIFIER);
    _nodePool.make(identifier._lexeme);
    _nodePool.get (identifier._lexeme)._index = parseTree.range(reader.look())._start;
}

void Builder::makeFunction(const prs::state::Tree& parseTree,
                           const dmit::prs::Reader& supReader,
                           TNode<node::Kind::FUN_DEFINITION>& function)
{
    auto reader = makeSubReaderFor(ParseNodeKind::FUN_DEFINITION, supReader);

    // Body
    _nodePool.make(function._body);
    makeScope(parseTree, reader, _nodePool.get(function._body));
    reader.advance();

    // Return type
    _nodePool.make(function._returnType);
    makeReturnType(parseTree, reader, _nodePool.get(function._returnType));
    reader.advance();

    // Arguments
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::FUN_ARGUMENTS);
    _nodePool.make(function._arguments);
    makeArguments(parseTree, reader, _nodePool.get(function._arguments));
    reader.advance();

    // Name
    _nodePool.make(function._name);
    makeIdentifier(parseTree, reader, _nodePool.get(function._name));
}

const State& Builder::operator()(const prs::state::Tree& parseTree)
{
    dmit::prs::Reader reader{parseTree};

    DMIT_COM_ASSERT(reader.isValid());
    _nodePool.make(reader.size(), _state._functions);

    uint32_t i = 0;

    while (reader.isValid())
    {
        makeFunction(parseTree, reader, _nodePool.get(_state._functions[i]));
        reader.advance();
        i++;
    }

    return _state;
}

const Builder::NodePool& Builder::nodePool() const
{
    return _nodePool;
}

} // namespace state
} // namespace ast
} // namespace dmit
