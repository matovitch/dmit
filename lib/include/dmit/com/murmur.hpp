#pragma once

#include "dmit/com/unique_id.hpp"

#include <cstdint>

namespace dmit::com::murmur
{

using Hash = UniqueId;

Hash& hash(const uint8_t* const data,
          const uint64_t size,
          Hash& hash);

Hash& combine(const Hash& lhs, Hash& rhs);

} // namespace dmit::com::murmur
