#pragma once

#include "dmit/ast/node.hpp"
#include "dmit/ast/pool.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/fmt/formatable.hpp"

namespace dmit
{

namespace ast
{

struct State : fmt::Formatable
{
    using NodePool = node::TPool<0x10>;

    TNode<node::Kind::PROGRAM> _program;
    NodePool                   _nodePool;
};

namespace state
{

class Builder
{

public:

    Builder();

    const State& operator()(const prs::state::Tree& parseTree);

private:

    using ParseNodeKind = dmit::prs::state::tree::node::Kind;

    void makeInteger(const dmit::prs::Reader& reader,
                     TNode<node::Kind::LIT_INTEGER>& identifier);

    void makeIdentifier(const dmit::prs::Reader& reader,
                        TNode<node::Kind::LIT_IDENTIFIER>& identifier);

    void makeScope(const dmit::prs::Reader& supReader,
                   TNode<node::Kind::SCOPE>& scope);

    void makeReturnType(const dmit::prs::Reader& supReader,
                        TNode<node::Kind::FUN_RETURN>& returnType);

    void makeArguments(const dmit::prs::Reader& supReader,
                       TNode<node::Kind::FUN_DEFINITION>& function);

    void makeFunction(const dmit::prs::Reader& supReader,
                      TNode<node::Kind::FUN_DEFINITION>& function);

    void makeDeclaration(const dmit::prs::Reader& reader,
                         Declaration& declaration);

    void makeStatement(const dmit::prs::Reader& reader,
                       Statement& statement);

    void makeExpression(const dmit::prs::Reader& reader,
                        Expression& expression);

    void makeAssignment(dmit::prs::Reader& reader,
                        TNode<node::Kind::EXP_BINOP>& assignment);

    void makeReturn(dmit::prs::Reader& reader,
                    TNode<node::Kind::STM_RETURN>& stmReturn);

    void makeBinop(dmit::prs::Reader& reader,
                   TNode<node::Kind::EXP_BINOP>& binop);

    void makeDclVariable(dmit::prs::Reader& reader,
                         TNode<node::Kind::DCL_VARIABLE>& dclVariable);

    void makeTypeClaim(dmit::prs::Reader& reader,
                       TNode<node::Kind::TYPE_CLAIM>& typeClaim);

    void makeFunctionCall(dmit::prs::Reader& reader,
                          TNode<node::Kind::FUN_CALL>& funCall);

    State            _state;
    State::NodePool& _nodePool;
};

} // namespace state
} // namespace ast
} // namespace dmit
