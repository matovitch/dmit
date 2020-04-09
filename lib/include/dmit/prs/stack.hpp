#pragma once

#include "dmit/prs/tree.hpp"

#include "dmit/com/option_reference.hpp"

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
    dmit::com::OptionRef<Stack> _parent;
};

} // namespace dmit::prs
