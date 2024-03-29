#pragma once

#include "dmit/ast/definition_role.hpp"
#include "dmit/ast/state.hpp"
#include "dmit/ast/node.hpp"

#include "dmit/wsm/wasm.hpp"

#include "dmit/com/tree_node.hpp"
#include "dmit/com/unique_id.hpp"

namespace dmit::ast::node::v_index
{

using Hasher     = com::tree::v_index::THasher     <Kind>;
using Comparator = com::tree::v_index::TComparator <Kind>;

VIndex makeVIndex(State::NodePool&, const VIndex);

com::UniqueId makeId(State::NodePool&, const VIndex);

DefinitionRole makeDefinitionRole(State::NodePool&, const VIndex);

std::optional<wsm::node::VIndex> makeWsm(State::NodePool&, const VIndex);

bool isInterface(const VIndex);

} // namespace dmit::ast::node::v_index
