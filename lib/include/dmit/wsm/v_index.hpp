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

    uint32_t operator()(TIndex<Kind::TYPE_FUNC> typeFuncIdx)
    {
        return _pool.get(typeFuncIdx)._id;
    }

    uint32_t operator()(TIndex<Kind::LOCAL> localIdx)
    {
        return _pool.get(localIdx)._id;
    }

    Pool& _pool;
};

template <class Pool>
struct VisitorSymbol
{
    VisitorSymbol(Pool& pool) : _pool{pool} {}

    template <com::TEnumIntegerType<Kind> KIND>
    uint32_t operator()(TIndex<KIND>)
    {
        DMIT_COM_ASSERT(!"[AST] Not implemented");
        return 0;
    }

    uint32_t operator()(TIndex<Kind::FUNCTION> functionIdx)
    {
        return _pool.get(functionIdx)._symbol;
    }

    uint32_t operator()(TIndex<Kind::IMPORT> importIdx)
    {
        return _pool.get(importIdx)._symbol;
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

template <class Pool>
int32_t makeSymbol(Pool& pool, const VIndex& vIndex)
{
    VisitorSymbol visitorSymbol{pool};

    return std::visit(visitorSymbol, vIndex);
}

} // namespace dmit::wsm::node::v_index
