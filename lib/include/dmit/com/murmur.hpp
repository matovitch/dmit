#pragma once

#include "dmit/com/unique_id.hpp"

#include <cstdint>

namespace dmit::com::murmur
{

using Hash = UniqueId;

void hash(const uint8_t* const data,
          const uint64_t size,
          Hash& hash);

void combine(const Hash& lhs, Hash& rhs);

} // namespace dmit::com::murmur
