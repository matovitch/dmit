#include "dmit/prs/reader.hpp"

#include <functional>
#include <optional>
#include <cstdint>

namespace dmit
{

namespace prs
{

Reader::Reader(const state::Tree& tree) :
    _nodes{tree.nodes()}
{}

reader::Head Reader::makeHead() const
{
    return reader::Head{static_cast<int32_t>(_nodes.get().size() - 1)};
}

std::optional<reader::Head> Reader::makeHead(const reader::Head head, const int32_t index) const
{
    const int32_t lowerBound = head._offset - _nodes.get()[head._offset]._size;
          int32_t offset     = head._offset - 1;

    for (int32_t i = 0; i < index; i++)
    {
        offset -= (_nodes.get()[offset]._size + 1);

        if (offset < lowerBound)
        {
            return std::nullopt;
        }
    }

    return reader::Head{offset};
}

const state::tree::Node& Reader::look(const reader::Head head) const
{
    return _nodes.get()[head._offset];
}

} // namespace prs
} // namespace dmit
