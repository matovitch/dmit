#pragma once

#include "dmit/fmt/formatable.hpp"

#include <cstdint>
#include <vector>

namespace dmit::src
{

namespace line_index
{

std::vector<uint32_t> makeOffsets(const std::vector<uint8_t>&);

} // namespace line_index

class LineIndex : fmt::Formatable
{

public:

    LineIndex() = default;

    void init(std::vector<uint32_t>&&);

    const std::vector<uint32_t>& offsets() const;

private:

    std::vector<uint32_t> _offsets;
};

} // namespace dmit::src
