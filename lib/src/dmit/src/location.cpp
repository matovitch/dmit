#include "dmit/src/location.hpp"

#include "dmit/fmt/src/line_index.hpp"

#include <algorithm>

namespace dmit
{

namespace src
{

Location::Location(const LineIndex& lineIndex, const uint32_t offset)
{
    const auto& fit = std::lower_bound(lineIndex.offsets().begin(),
                                       lineIndex.offsets().end(),
                                       offset,
                                       std::greater<uint32_t>{});

    _line   = fit - lineIndex.offsets().begin();
    _column = (*(fit - 1)) - offset;
}

uint32_t Location::line   () const { return _line   ; }
uint32_t Location::column () const { return _column ; }

} // namespace src

} // namespace dmit
