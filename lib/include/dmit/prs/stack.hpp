#pragma once

#include <functional>
#include <optional>
#include <cstdint>

namespace dmit::prs
{

struct Stack
{
    uint32_t _treeSize;
    uint32_t _readerOffset;
    uint32_t _childCount = 0;

    std::optional<std::reference_wrapper<Stack>> _parent;
};

} // namespace dmit::prs
