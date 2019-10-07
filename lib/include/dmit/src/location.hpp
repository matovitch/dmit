#pragma once

#include "dmit/src/line_index.hpp"

#include "dmit/fmt/formatable.hpp"

#include <cstdint>

namespace dmit
{

namespace src
{

class Location : fmt::Formatable
{

public:

    Location(const LineIndex&, const uint32_t);

    uint32_t line   () const;
    uint32_t column () const;

private:

    uint32_t _line;
    uint32_t _column;
};

} // namespace src

} // namespace dmit
