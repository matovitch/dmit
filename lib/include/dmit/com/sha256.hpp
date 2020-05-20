#pragma once

#include "dmit/com/unique_id.hpp"

#include <cstdint>

namespace dmit::com::sha256
{

struct State
{
    uint32_t _asUint32[8]
    {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    const UniqueId& uniqueId() const;
};

void transform(const uint8_t* const bytes, State& state);

void update(const uint8_t* const bytes, State& state);

class UniqueIdSequence
{

public:

    UniqueIdSequence(const UniqueId& seed);

    const UniqueId& operator()();

private:

    State _state;
};

} // namespace dmit::com::sha256
