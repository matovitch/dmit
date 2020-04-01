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

    using NodePool = node::TPool<0x10>;

    const State& operator()(const prs::state::Tree& parseTree);

    const NodePool& nodePool() const;

private:

    using ParseNodeKind = dmit::prs::state::tree::node::Kind;

    void makeInteger(const prs::state::Tree& parseTree,
                     const dmit::prs::Reader& reader,
                     TNode<node::Kind::LIT_INTEGER>& identifier);

    void makeIdentifier(const prs::state::Tree& parseTree,
                        const dmit::prs::Reader& reader,
                        TNode<node::Kind::LIT_IDENTIFIER>& identifier);

    void makeScope(const prs::state::Tree& parseTree,
                   dmit::prs::Reader& reader,
                   TNode<node::Kind::SCOPE>& scope);

    void makeReturnType(const prs::state::Tree& parseTree,
                        dmit::prs::Reader& reader,
                        TNode<node::Kind::FUN_RETURN>& returnType);

    void makeReturnTypeVoid(TNode<node::Kind::FUN_RETURN>& returnType);

    void makeArguments(const prs::state::Tree& parseTree,
                       dmit::prs::Reader& reader,
                       TNode<node::Kind::FUN_ARGUMENTS>& arguments);

    void makeArgumentsEmpty(TNode<node::Kind::FUN_ARGUMENTS>& arguments);

    void makeFunction(const prs::state::Tree& parseTree,
                      dmit::prs::Reader& reader,
                      TNode<node::Kind::FUN_DEFINITION>& function);

    void makeDeclaration(const prs::state::Tree& parseTree,
                         const dmit::prs::Reader& reader,
                         Declaration& declaration);

    void makeStatement(const prs::state::Tree& parseTree,
                       const dmit::prs::Reader& reader,
                       Statement& statement);

    void makeExpression(const prs::state::Tree& parseTree,
                        const dmit::prs::Reader& reader,
                        Expression& expression);

    void makeAssignment(const prs::state::Tree& parseTree,
                        const dmit::prs::Reader& supReader,
                        TNode<node::Kind::STM_ASSIGN>& assignment);

    void makeReturn(const prs::state::Tree& parseTree,
                    dmit::prs::Reader& reader,
                    TNode<node::Kind::STM_RETURN>& stmReturn);

    void makeBinop(const prs::state::Tree& parseTree,
                   dmit::prs::Reader& reader,
                   TNode<node::Kind::EXP_BINOP>& binop);

    void makeDclVariable(const prs::state::Tree& parseTree,
                         dmit::prs::Reader& reader,
                         TNode<node::Kind::DCL_VARIABLE>& dclVariable);

    void makeTypeClaim(const prs::state::Tree& parseTree,
                       dmit::prs::Reader& reader,
                       TNode<node::Kind::TYPE_CLAIM>& typeClaim);

    State    _state;
    NodePool _nodePool;
};

} // namespace state
} // namespace ast
} // namespace dmit
