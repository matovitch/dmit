#pragma once

#include "dmit/prs/tree.hpp"

#include <functional>
#include <optional>
#include <cstdint>

namespace dmit
{

namespace prs
{

namespace reader
{

struct Head
{
    int32_t _offset;
};

} // namespace reader

class Reader
{

public:

    Reader(const state::Tree& tree);

    reader::Head makeHead() const;

    std::optional<reader::Head> makeHead(const reader::Head head, const int32_t index) const;

    const state::tree::Node& look(const reader::Head head) const;

private:

    const std::reference_wrapper<const std::vector<state::tree::Node>> _nodes;
};

} // namespace prs
} // namespace dmit
