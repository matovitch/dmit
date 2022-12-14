#pragma once

#include "dmit/com/storage.hpp"

#include <vector>

namespace dmit::gen
{

com::TStorage<uint8_t> makeArchive(const std::vector<com::TStorage<uint8_t>>&);

} // namespace dmit::gen
