#include "dmit/ast/state.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>
#include <variant>

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

} // namespace

void Builder::makeDeclaration(const prs::state::Tree& parseTree,
                              const dmit::prs::Reader& reader,
                              TNode<node::Kind::SCOPE_VARIANT>& declaration)
{}

void Builder::makeStatement(const prs::state::Tree& parseTree,
                            const dmit::prs::Reader& reader,
                            TNode<node::Kind::SCOPE_VARIANT>& statement)
{}

void Builder::makeExpression(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& reader,
                             TNode<node::Kind::SCOPE_VARIANT>& expression)
{}

void Builder::makeScope(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::SCOPE>& scope)
{
    _nodePool.make(reader.size(), scope._variants);

    uint32_t i = 0;

    while (reader.isValid())
    {
        auto&& variant = _nodePool.get(scope._variants[i]);

        auto parseNodeKind = reader.look()._kind;

        if (isDeclaration(parseNodeKind))
        {
            makeDeclaration(parseTree, reader, variant);
        }
        else if (isStatement(parseNodeKind))
        {
            makeStatement(parseTree, reader, variant);
        }
        else if (isExpression(parseNodeKind))
        {
            makeExpression(parseTree, reader, variant);
        }

        reader.advance();
        i++;
    }
}

void Builder::makeReturnType(const prs::state::Tree& parseTree,
                             dmit::prs::Reader& reader,
                             TNode<node::Kind::RETURN_TYPE>& returnType)
{
    returnType._option = node::TIndex<node::Kind::LEXEME>{};
    _nodePool.make (returnType._option.value());
    _nodePool.get  (returnType._option.value())._index = parseTree.range(reader.look())._start;
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
        auto&& annotaType = _nodePool.get(arguments._annotaTypes[i]);

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
    DMIT_COM_ASSERT(reader.look()._kind == ParseNodeKind::DECLAR_FUN);

    _nodePool.make(reader.size(), _state._functions);

    uint32_t i = 0;

    while (reader.isValid())
    {
        auto&& function = _nodePool.get(_state._functions[i]);
        auto&& functionReader = reader.makeSubReader();

        DMIT_COM_ASSERT(functionReader);

        makeFunction(parseTree, functionReader.value(), function);

        reader.advance();
        i++;
    }

    return _state;
}

} // namespace state
} // namespace ast
} // namespace dmit
