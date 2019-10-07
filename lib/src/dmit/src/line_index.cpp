#include "dmit/src/line_index.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace dmit
{

namespace src
{

static const uint8_t K_LINE_DELIMITER = static_cast<uint8_t>('\n');

namespace line_index
{

std::vector<uint32_t> makeOffsets(const std::vector<uint8_t>& bytes)
{
    std::vector<uint32_t> offsets;

    offsets.push_back(bytes.size() + 1);

    for (std::size_t i = 0; i < bytes.size(); i++)
    {
        if (bytes[i] == K_LINE_DELIMITER)
        {
            offsets.push_back(bytes.size() - i);
        }
    }

    offsets.push_back(0);

    return offsets;
}

} // namespace line_index

void LineIndex::init(std::vector<uint32_t>&& offsets)
{
    _offsets = std::move(offsets);
}

const std::vector<uint32_t>& LineIndex::offsets() const
{
    return _offsets;
}

} // namespace src

} // namespace dmit