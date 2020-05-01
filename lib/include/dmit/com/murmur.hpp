#pragma once

#include <cstdint>

namespace dmit::com::murmur
{

struct Hash
{
    uint64_t _halfL = 0x9f8113e93cc7050c;
    uint64_t _halfH = 0x49d82c385155ae97;
};

void hash(const uint8_t* const data,
          const uint64_t size,
          Hash& hash);

} // namespace dmit::com::murmur
