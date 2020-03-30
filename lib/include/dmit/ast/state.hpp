#pragma once

#include "dmit/ast/node.hpp"
#include "dmit/ast/pool.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

namespace dmit
{

namespace ast
{

using State = TNode<node::Kind::PROGRAM>;

namespace state
{

class Builder
{

public:

    const State& operator()(const prs::state::Tree& parseTree);

private:

    using ParseNodeKind = dmit::prs::state::tree::node::Kind;

    void makeScope(const prs::state::Tree& parseTree,
                   dmit::prs::Reader& reader,
                   TNode<node::Kind::SCOPE>& scope);

    void makeReturnType(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::RETURN_TYPE>& returnType);

    void makeReturnTypeVoid(TNode<node::Kind::RETURN_TYPE>& returnType);

    void makeArguments(const prs::state::Tree& parseTree,
                       dmit::prs::Reader& reader,
                       TNode<node::Kind::ARGUMENTS>& arguments);

    void makeArgumentsEmpty(TNode<node::Kind::ARGUMENTS>& arguments);

    void makeFunction(const prs::state::Tree& parseTree,
                      dmit::prs::Reader& reader,
                      TNode<node::Kind::FUNCTION>& function);

    void makeDeclaration(const prs::state::Tree& parseTree,
                         const dmit::prs::Reader& reader,
                         TNode<node::Kind::SCOPE_VARIANT>& declaration);

    void makeStatement(const prs::state::Tree& parseTree,
                       const dmit::prs::Reader& reader,
                       TNode<node::Kind::SCOPE_VARIANT>& statement);

    void makeExpression(const prs::state::Tree& parseTree,
                        const dmit::prs::Reader& reader,
                        TNode<node::Kind::SCOPE_VARIANT>& expression);
    
    State _state;
    node::TPool<0x10> _nodePool;
};

} // namespace state
} // namespace ast
} // namespace dmit
