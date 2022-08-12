#include "dmit/ast/v_index.hpp"

#include "dmit/ast/state.hpp"

#include "dmit/com/unique_id.hpp"
#include "dmit/com/assert.hpp"

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

} // namespace

com::UniqueId makeId(State::NodePool& pool, const VIndex vIndex)
{
    IdVisitor idVisitor{pool};

    return std::visit(idVisitor, vIndex._variant);
}

} // namespace dmit::ast::node::v_index
