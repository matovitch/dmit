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

void Builder::makeDeclarLet(const prs::state::Tree& parseTree,
                            dmit::prs::Reader& reader,
                            TNode<node::Kind::DECLAR_LET>& declarLet)
{}

void Builder::makeAssignment(const prs::state::Tree& parseTree,
                            dmit::prs::Reader& reader,
                            TNode<node::Kind::ASSIGNMENT>& assignment)
{}

void Builder::makeExpression(const prs::state::Tree& parseTree,
                             const dmit::prs::Reader& reader,
                             TNode<node::Kind::EXPRESSION>& expression)
{}

void Builder::makeStatemReturn(const prs::state::Tree& parseTree,
                               dmit::prs::Reader& reader,
                               TNode<node::Kind::STATEM_RETURN>& statemReturn)
{}

void Builder::makeScope(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::SCOPE>& scope)
{
    _nodePool.make(reader.size(), scope._variants);

    uint32_t i = 0;

    while (reader.isValid())
    {
        auto variant       = _nodePool.get(scope._variants[i]);
        auto variantReader = reader.makeSubReader();

        auto parseNodeKind = reader.look()._kind;

        if (parseNodeKind == ParseNodeKind::DECLAR_LET)
        {
            DMIT_COM_ASSERT(variantReader);
            variant._value = node::TIndex<node::Kind::DECLAR_LET>{};
            makeDeclarLet(parseTree, variantReader.value(), _nodePool.get(std::get<node::TIndex<node::Kind::DECLAR_LET>>(variant._value)));
        }
        else if (parseNodeKind == ParseNodeKind::ASSIGNMENT)
        {
            DMIT_COM_ASSERT(variantReader);
            variant._value = node::TIndex<node::Kind::ASSIGNMENT>{};
            makeAssignment(parseTree, variantReader.value(), _nodePool.get(std::get<node::TIndex<node::Kind::ASSIGNMENT>>(variant._value)));
        }
        else if (parseNodeKind == ParseNodeKind::STATEM_RETURN)
        {
            DMIT_COM_ASSERT(variantReader);
            variant._value = node::TIndex<node::Kind::STATEM_RETURN>{};
            makeStatemReturn(parseTree, variantReader.value(), _nodePool.get(std::get<node::TIndex<node::Kind::STATEM_RETURN>>(variant._value)));
        }
        else
        {
            variant._value = node::TIndex<node::Kind::EXPRESSION>{};
            makeExpression(parseTree, reader, _nodePool.get(std::get<node::TIndex<node::Kind::EXPRESSION>>(variant._value)));
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
        auto annotaType = _nodePool.get(arguments._annotaTypes[i]);

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
        auto function = _nodePool.get(_state._functions[i]);
        auto functionReader = reader.makeSubReader();

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
