//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. This code is based on this implementation:
// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
//
#include "dmit/com/murmur.hpp"

#include <cstdint>
#include <limits>

namespace
{

template <int I>
uint64_t tRotL(const uint64_t value)
{
    return (value << I) | (value >> (std::numeric_limits<uint64_t>::digits - I));
}

uint64_t fmix(uint64_t k)
{
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccd;
    k ^= k >> 33;
    k *= 0xc4ceb9fe1a85ec53;
    k ^= k >> 33;

    return k;
}

} // namespace

namespace dmit::com::murmur
{

void hash(const uint8_t* const data,
          const uint64_t size,
          Hash& hash)
{
    const uint64_t sizeBy16 = size / sizeof(Hash);

    uint64_t& h1 = hash._halfL;
    uint64_t& h2 = hash._halfH;

    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    //----------
    // body

    const uint64_t * blocks = (const uint64_t*)(data);

    for(int i = 0; i < sizeBy16; i++)
    {
        uint64_t k1 = blocks[(i << 1) + 0];
        uint64_t k2 = blocks[(i << 1) + 1];

        k1 *= c1; k1  = tRotL<31>(k1); k1 *= c2; h1 ^= k1;

        h1 = tRotL<27>(h1); h1 += h2; h1 = h1 * 5 + 0x52dce729;

        k2 *= c2; k2  = tRotL<33>(k2); k2 *= c1; h2 ^= k2;

        h2 = tRotL<31>(h2); h2 += h1; h2 = h2 * 5 + 0x38495ab5;
    }

    //----------
    // tail

    const uint8_t* const tail = static_cast<const uint8_t*>(data + sizeBy16 * sizeof(Hash));

    uint64_t k1 = 0;
    uint64_t k2 = 0;

    switch(size & 15)
    {
        case 15: k2 ^= static_cast<uint64_t>(tail[14]) << 48;
        case 14: k2 ^= static_cast<uint64_t>(tail[13]) << 40;
        case 13: k2 ^= static_cast<uint64_t>(tail[12]) << 32;
        case 12: k2 ^= static_cast<uint64_t>(tail[11]) << 24;
        case 11: k2 ^= static_cast<uint64_t>(tail[10]) << 16;
        case 10: k2 ^= static_cast<uint64_t>(tail[ 9]) << 8;
        case  9: k2 ^= static_cast<uint64_t>(tail[ 8]) << 0;
                 k2 *= c2; k2  = tRotL<33>(k2); k2 *= c1; h2 ^= k2;

        case  8: k1 ^= static_cast<uint64_t>(tail[ 7]) << 56;
        case  7: k1 ^= static_cast<uint64_t>(tail[ 6]) << 48;
        case  6: k1 ^= static_cast<uint64_t>(tail[ 5]) << 40;
        case  5: k1 ^= static_cast<uint64_t>(tail[ 4]) << 32;
        case  4: k1 ^= static_cast<uint64_t>(tail[ 3]) << 24;
        case  3: k1 ^= static_cast<uint64_t>(tail[ 2]) << 16;
        case  2: k1 ^= static_cast<uint64_t>(tail[ 1]) << 8;
        case  1: k1 ^= static_cast<uint64_t>(tail[ 0]) << 0;
                 k1 *= c1; k1  = tRotL<31>(k1); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= size; h2 ^= size;

    h1 += h2;
    h2 += h1;

    h1 = fmix(h1);
    h2 = fmix(h2);

    h1 += h2;
    h2 += h1;
}

} // namespace dmit::com::murmur
