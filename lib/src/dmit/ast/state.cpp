#include "dmit/ast/state.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/com/assert.hpp"

namespace dmit
{

namespace ast
{

namespace state
{

void Builder::makeScope(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::SCOPE>& scope)
{}

void Builder::makeReturnType(const prs::state::Tree& parseTree,
                             dmit::prs::Reader& reader,
                             TNode<node::Kind::RETURN_TYPE>& returnType)
{}

void Builder::makeReturnTypeVoid(TNode<node::Kind::RETURN_TYPE>& returnType)
{}

void Builder::makeArguments(const prs::state::Tree& parseTree,
                            dmit::prs::Reader& reader,
                            TNode<node::Kind::ARGUMENTS>& arguments)
{}

void Builder::makeArgumentsEmpty(TNode<node::Kind::ARGUMENTS>& arguments)
{}

void Builder::makeFunction(const prs::state::Tree& parseTree,
                           dmit::prs::Reader& reader,
                           TNode<node::Kind::FUNCTION>& function)
{
    // Body
    DMIT_COM_ASSERT(reader.look()._kind == dmit::prs::state::tree::node::Kind::SCOPE);
    auto bodyReader = reader.makeSubReader();
    DMIT_COM_ASSERT(bodyReader);
    _state._nodePool.make(function._body);
    makeScope(parseTree, bodyReader.value(), _state._nodePool.get(function._body));
    reader.advance();

    // Return type
    DMIT_COM_ASSERT(reader.look()._kind == dmit::prs::state::tree::node::Kind::RETURN_TYPE);
    _state._nodePool.make(function._returnType);
    auto returnTypeReader = reader.makeSubReader();
    returnTypeReader ? makeReturnType     (parseTree,
                                           returnTypeReader.value(),
                                           _state._nodePool.get(function._returnType))
                     : makeReturnTypeVoid (_state._nodePool.get(function._returnType));
    reader.advance();

    // Arguments
    DMIT_COM_ASSERT(reader.look()._kind == dmit::prs::state::tree::node::Kind::ARG_LIST);
    _state._nodePool.make(function._arguments);
    auto argumentsReader = reader.makeSubReader();
    argumentsReader ? makeArguments      (parseTree,
                                          argumentsReader.value(),
                                          _state._nodePool.get(function._arguments))
                    : makeArgumentsEmpty (_state._nodePool.get(function._arguments));
    reader.advance();

    // Name
    const auto& parseNode = reader.look();
    DMIT_COM_ASSERT(reader.look()._kind == dmit::prs::state::tree::node::Kind::IDENTIFIER);
    _state._nodePool.make(function._name);
    _state._nodePool.get(function._name)._index = parseTree.range(parseNode)._start;
}

const State& Builder::operator()(const prs::state::Tree& parseTree)
{
    dmit::prs::Reader reader{parseTree};

    DMIT_COM_ASSERT(reader.isValid());
    DMIT_COM_ASSERT(reader.look()._kind == dmit::prs::state::tree::node::Kind::DECLAR_FUN);

    _state._nodePool.make(reader.size(), _state._program._functions);

    uint32_t i = 0;

    while (reader.isValid())
    {
        auto function = _state._nodePool.get(_state._program._functions[i]);
        auto functionReader = reader.makeSubReader();

        DMIT_COM_ASSERT(functionReader);

        makeFunction(parseTree, functionReader.value(), function);

        reader.advance();
        i++;
    }
}

} // namespace state
} // namespace ast
} // namespace dmit
