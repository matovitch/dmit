#pragma once

#include "dmit/com/storage.hpp"

namespace dmit::gen
{

com::TStorage<uint8_t> makeArchive(const com::TStorage<com::TStorage<uint8_t>>&);

} // namespace dmit::gen
