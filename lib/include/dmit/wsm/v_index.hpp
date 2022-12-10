#pragma once

#include "dmit/wsm/wasm.hpp"

#include <cstdint>
#include <variant>

namespace dmit::wsm::node::v_index
{

namespace
{

template <class Pool>
struct VisitorId
{
    VisitorId(Pool& pool) : _pool{pool} {}

    template <com::TEnumIntegerType<Kind> KIND>
    uint32_t operator()(TIndex<KIND>)
    {
        DMIT_COM_ASSERT(!"[AST] Not implemented");
        return 0;
    }

    uint32_t operator()(TIndex<Kind::FUNCTION> functionIdx)
    {
        return _pool.get(functionIdx)._id;
    }

    uint32_t operator()(TIndex<Kind::IMPORT> importIdx)
    {
        return _pool.get(importIdx)._id;
    }

    Pool& _pool;
};

} // namespace

template <class Pool>
uint32_t makeId(Pool& pool, const VIndex& vIndex)
{
    VisitorId visitorId{pool};

    return std::visit(visitorId, vIndex);
}

} // namespace dmit::wsm::node::v_index
