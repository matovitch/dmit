#include "dmit/ast/v_index.hpp"

#include "dmit/ast/definition_role.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/wsm/wasm.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/tree_node.hpp"
#include "dmit/com/assert.hpp"

#include <optional>
#include <cstdint>

namespace dmit::ast::node::v_index
{

namespace
{

struct IdVisitor
{
    IdVisitor(State::NodePool& pool) : _pool{pool} {}

    template <com::TEnumIntegerType<Kind> KIND>
    com::UniqueId operator()(TIndex<KIND>)
    {
        DMIT_COM_ASSERT(!"[AST] Not implemented");
        return com::UniqueId{};
    }

    com::UniqueId operator()(TIndex<Kind::DCL_VARIABLE> dclVariableIdx)
    {
        return _pool.get(dclVariableIdx)._id;
    }

    com::UniqueId operator()(TIndex<Kind::DEF_CLASS> defClassIdx)
    {
        return _pool.get(defClassIdx)._id;
    }

    com::UniqueId operator()(TIndex<Kind::DEF_FUNCTION> functionIdx)
    {
        return _pool.get(functionIdx)._id;
    }

    com::UniqueId operator()(TIndex<Kind::MODULE> moduleIdx)
    {
        return _pool.get(moduleIdx)._id;
    }

    com::UniqueId operator()(TIndex<Kind::VIEW> viewIdx)
    {
        return _pool.get(viewIdx)._id;
    }

    State::NodePool& _pool;
};

struct DefinitionRoleVisitor
{
    DefinitionRoleVisitor(State::NodePool& pool) : _pool{pool} {}

    template <com::TEnumIntegerType<Kind> KIND>
    DefinitionRole operator()(TIndex<KIND>)
    {
        DMIT_COM_ASSERT(!"[AST] Not implemented");
        return DefinitionRole::LOCAL;
    }

    DefinitionRole operator()(TIndex<Kind::DEFINITION> definitionIdx)
    {
        return _pool.get(definitionIdx)._role;
    }

    State::NodePool& _pool;
};

struct WsmVisitor
{
    WsmVisitor(State::NodePool& pool) : _pool{pool} {}

    template <com::TEnumIntegerType<Kind> KIND>
    std::optional<wsm::node::VIndex> operator()(TIndex<KIND>)
    {
        DMIT_COM_ASSERT(!"[AST] Not implemented");
        return std::nullopt;
    }

    std::optional<wsm::node::VIndex> operator()(TIndex<Kind::DCL_VARIABLE> dclVariableIdx)
    {
        return _pool.get(dclVariableIdx)._asWsm;
    }

    std::optional<wsm::node::VIndex> operator()(TIndex<Kind::DEF_FUNCTION> functionIdx)
    {
        return _pool.get(functionIdx)._asWsm;
    }

    State::NodePool& _pool;
};

} // namespace

com::UniqueId makeId(State::NodePool& pool, const VIndex vIndex)
{
    IdVisitor idVisitor{pool};

    return std::visit(idVisitor, vIndex);
}

DefinitionRole makeDefinitionRole(State::NodePool& pool, const VIndex vIndex)
{
    DefinitionRoleVisitor defRoleVisitor{pool};

    return std::visit(defRoleVisitor, vIndex);
}

std::optional<wsm::node::VIndex> makeWsm(State::NodePool& pool, const VIndex vIndex)
{
    WsmVisitor wsmVisitor{pool};

    return std::visit(wsmVisitor, vIndex);
}

bool isInterface(const VIndex vIndex)
{
    return com::tree::v_index::isInterface<node::Kind>(vIndex);
}

} // namespace dmit::ast::node::v_index
