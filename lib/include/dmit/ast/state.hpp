#pragma once

#include "dmit/ast/node.hpp"
#include "dmit/ast/pool.hpp"

#include "dmit/prs/reader.hpp"
#include "dmit/prs/tree.hpp"

#include "dmit/fmt/formatable.hpp"

#include <memory>

namespace dmit::ast
{

struct State : fmt::Formatable
{
    using NodePool = node::TPool<0x10>;

    State(NodePool&);

    NodePool&                        _nodePool;
    node::TIndex<node::Kind::MODULE> _module;
    node::TIndex<node::Kind::SOURCE> _source;
};

namespace state
{

class Builder
{

public:

    Builder();

    State& operator()(const prs::state::Tree& parseTree);

private:

    using ParseNodeKind = dmit::prs::state::tree::node::Kind;

    void makeLexeme(const dmit::prs::Reader& reader,
                    TNode<node::Kind::LEXEME>& lexeme);

    void makeInteger(const dmit::prs::Reader& reader,
                     TNode<node::Kind::LIT_INTEGER>& identifier);

    void makeIdentifier(const dmit::prs::Reader& reader,
                        TNode<node::Kind::LIT_IDENTIFIER>& identifier);

    void makeScopeVariant(const dmit::prs::Reader& reader,
                          TNode<node::Kind::SCOPE_VARIANT>& scopeVariant);

    void makeScope(const dmit::prs::Reader& supReader,
                   TNode<node::Kind::SCOPE>& scope);

    void makeMembers(const dmit::prs::Reader& supReader,
                     TNode<node::Kind::TYP_DEFINITION>& type);

    void makeArguments(const dmit::prs::Reader& supReader,
                       TNode<node::Kind::FUN_DEFINITION>& function);

    void makeFunction(const dmit::prs::Reader& supReader,
                      TNode<node::Kind::FUN_DEFINITION>& function);

    void makeType(const dmit::prs::Reader& supReader,
                  TNode<node::Kind::TYP_DEFINITION>& type);

    void makeImport(const dmit::prs::Reader& supReader,
                    TNode<node::Kind::DCL_IMPORT>& import);

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

    void makeDefinition(const dmit::prs::Reader& supReader,
                        TNode<node::Kind::DEFINITION>& definition);

    void makeModule(dmit::prs::Reader& reader,
                    TNode<node::Kind::MODULE>& module);

    State::NodePool _nodePool;
    State           _state;
};

} // namespace state
} // namespace dmit::ast
