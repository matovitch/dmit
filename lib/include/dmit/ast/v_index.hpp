#pragma once

#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

namespace dmit::ast::node::v_index
{

struct HashVisitor
{
    template <com::TEnumIntegerType<Kind> KIND>
    std::size_t operator()(const TIndex<KIND> tIndex)
    {
        return (tIndex._value << 0x8) | KIND;
    }
};

struct Hasher
{
    std::size_t operator()(const VIndex vIndex) const
    {
        HashVisitor hashVisitor;

        return std::visit(hashVisitor, vIndex._variant);
    }
};

struct Comparator
{
    bool operator()(const VIndex lhs,
                    const VIndex rhs) const
    {
        return lhs._variant ==
               rhs._variant;
    }
};

com::UniqueId makeId(State::NodePool&, const VIndex);

bool isInterface(const VIndex);

uint32_t value(const VIndex);

} // namespace dmit::ast::node::v_index
