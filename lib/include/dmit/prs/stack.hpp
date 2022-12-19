#pragma once

#include "dmit/prs/tree.hpp"

#include "dmit/com/reference.hpp"

#include <cstdint>

namespace dmit::prs
{

struct Stack
{
    bool _isErrorPushed;
    uint32_t _treeSize;
    uint32_t _readerOffset;
    uint32_t _childCount = 0;
    std::optional<state::tree::node::Kind> _treeNodeKindOpt;
    dmit::com::TOptionRef<Stack> _parent;
};

} // namespace dmit::prs
