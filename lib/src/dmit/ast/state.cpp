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
    return parseNodeKind == dmit::prs::state::tree::node::Kind::DECLAR_LET;
}

bool isStatement(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::ASSIGNMENT ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::STATEM_RETURN;
}

bool isExpression(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::FUN_CALL   ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::COMPARISON ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::PRODUCT    ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::SUM        ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::IDENTIFIER;
}

bool isBinaryOperation(const dmit::prs::state::tree::node::Kind parseNodeKind)
{
    return parseNodeKind == dmit::prs::state::tree::node::Kind::COMPARISON ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::SUM        ||
           parseNodeKind == dmit::prs::state::tree::node::Kind::PRODUCT;
}

template <class Variant, class BlipType>
void blitVariant(const BlipType& toBlip, Variant& variant)
{
    const Variant toBlipAsVariant = toBlip;
    std::memcpy(&variant, &toBlipAsVariant, sizeof(Variant));
}

} // namespace


void Builder::makeAssignment(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& supReader,
                             TNode<node::Kind::ASSIGNMENT>& assignment)
{
    // TODO
}

void Builder::makeStatemReturn(const prs::state::Tree& parseTree,
                               dmit::prs::Reader& reader,
                               TNode<node::Kind::STATEM_RETURN>& statemReturn)
{
    makeExpression(parseTree, reader, statemReturn._expression);
}

void Builder::makeDeclaration(const prs::state::Tree& parseTree,
                              const dmit::prs::Reader& reader,
                              Declaration& declaration)
{
    // TODO
}

void Builder::makeStatement(const prs::state::Tree& parseTree,
                            const dmit::prs::Reader& reader,
                            Statement& statement)
{
    auto subReader = reader.makeSubReader();
    DMIT_COM_ASSERT(subReader);

    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::ASSIGNMENT)
    {
        node::TIndex<node::Kind::ASSIGNMENT> assignment;
        _nodePool.make(assignment);
        makeAssignment(parseTree, subReader.value(), _nodePool.get(assignment));
        blitVariant(assignment, statement);
    }
    else if (parseNodeKind == ParseNodeKind::STATEM_RETURN)
    {
        node::TIndex<node::Kind::STATEM_RETURN> statemReturn;
        _nodePool.make(statemReturn);
        makeStatemReturn(parseTree, subReader.value(), _nodePool.get(statemReturn));
        blitVariant(statemReturn, statement);
    }
    else
    {
        DMIT_COM_ASSERT(false); // Should not get there
    }
}

void Builder::makeBinop(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::BINOP>& binop)
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

    binop._lhs = node::TIndex<node::Kind::BINOP>{};
    auto& binopLhs = std::get<node::TIndex<node::Kind::BINOP>>(binop._lhs);
    _nodePool.make(binopLhs);
    makeBinop(parseTree, reader, _nodePool.get(binopLhs));
}

void Builder::makeExpression(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& reader,
                             Expression& expression)
{
    auto parseNodeKind = reader.look()._kind;

    if (parseNodeKind == ParseNodeKind::IDENTIFIER)
    {
        node::TIndex<node::Kind::LEXEME> lexeme;
        _nodePool.make (lexeme);
        _nodePool.get  (lexeme)._index = parseTree.range(reader.look())._start;
        blitVariant    (lexeme, expression);
    }
    else if (isBinaryOperation(parseNodeKind))
    {
        node::TIndex<node::Kind::BINOP> binop;
        _nodePool.make(binop);
        auto subReader = reader.makeSubReader();
        DMIT_COM_ASSERT(subReader);
        makeBinop(parseTree, subReader.value(), _nodePool.get(binop));
        blitVariant(binop, expression);
    }
    else
    {
        DMIT_COM_ASSERT(false); // Should not get there
    }
}

void Builder::makeScope(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::SCOPE>& scope)
{
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
            auto subReader = reader.makeSubReader();
            DMIT_COM_ASSERT(subReader);
            makeScope(parseTree, subReader.value(), _nodePool.get(subScope));
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
                             dmit::prs::Reader& reader,
                             TNode<node::Kind::RETURN_TYPE>& returnType)
{
    node::TIndex<node::Kind::LEXEME> lexeme;
    _nodePool.make (lexeme);
    _nodePool.get  (lexeme)._index = parseTree.range(reader.look())._start;
    blitVariant(lexeme, returnType._option);
}

void Builder::makeReturnTypeVoid(TNode<node::Kind::RETURN_TYPE>& returnType)
{
    returnType._option = std::nullopt;
}

void Builder::makeArguments(const prs::state::Tree& parseTree,
                            dmit::prs::Reader& reader,
                            TNode<node::Kind::ARGUMENTS>& arguments)
{
    _nodePool.make(reader.size() >> 1, arguments._annotaTypes);

    uint32_t i = 0;

    while (reader.isValid())
    {
        auto& annotaType = _nodePool.get(arguments._annotaTypes[i]);

        // Variable
        DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::IDENTIFIER);
        _nodePool.make (annotaType._variable);
        _nodePool.get  (annotaType._variable)._index = parseTree.range(reader.look())._start;
        reader.advance();

        // Type
        DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::IDENTIFIER);
        _nodePool.make (annotaType._type);
        _nodePool.get  (annotaType._type)._index = parseTree.range(reader.look())._start;
        reader.advance();
        i++;
    }
}

void Builder::makeArgumentsEmpty(TNode<node::Kind::ARGUMENTS>& arguments)
{
    arguments._annotaTypes._size = 0;
}

void Builder::makeFunction(const prs::state::Tree& parseTree,
                           dmit::prs::Reader& reader,
                           TNode<node::Kind::FUNCTION>& function)
{
    // Body
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::SCOPE);
    auto bodyReader = reader.makeSubReader();
    DMIT_COM_ASSERT(bodyReader);
    _nodePool.make(function._body);
    makeScope(parseTree, bodyReader.value(), _nodePool.get(function._body));
    reader.advance();

    // Return type
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::RETURN_TYPE);
    _nodePool.make(function._returnType);
    auto returnTypeReader = reader.makeSubReader();
    returnTypeReader ? makeReturnType     (parseTree,
                                           returnTypeReader.value(),
                                           _nodePool.get(function._returnType))
                     : makeReturnTypeVoid (_nodePool.get(function._returnType));
    reader.advance();

    // Arguments
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::ARG_LIST);
    _nodePool.make(function._arguments);
    auto argumentsReader = reader.makeSubReader();
    DMIT_COM_ASSERT(argumentsReader);
    argumentsReader ? makeArguments      (parseTree,
                                          argumentsReader.value(),
                                          _nodePool.get(function._arguments))
                    : makeArgumentsEmpty (_nodePool.get(function._arguments));
    reader.advance();

    // Name
    const auto& parseNode = reader.look();
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::IDENTIFIER);
    _nodePool.make (function._name);
    _nodePool.get  (function._name)._index = parseTree.range(parseNode)._start;
}

const State& Builder::operator()(const prs::state::Tree& parseTree)
{
    dmit::prs::Reader reader{parseTree};

    DMIT_COM_ASSERT(reader.isValid());
    _nodePool.make(reader.size(), _state._functions);

    uint32_t i = 0;

    while (reader.isValid())
    {
        DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::DECLAR_FUN);

        auto& function = _nodePool.get(_state._functions[i]);
        auto  functionReader = reader.makeSubReader();

        DMIT_COM_ASSERT(functionReader);

        makeFunction(parseTree, functionReader.value(), function);

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
