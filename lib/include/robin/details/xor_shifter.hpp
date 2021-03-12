/* Based upon https://gist.github.com/reedacartwright/958454beca5e33d696af0eba7cbd70eb */

#pragma once

#include <cstdint>

namespace robin_details
{

class XorShifter
{
    static constexpr uint64_t BASE_STATE_0 = 0xD2D45C2500000000;
    static constexpr uint64_t BASE_STATE_1 = 0x61C8864680B583EB;

public:

    uint64_t operator()()
    {
        _state_0 ^= (_state_0 << 5);
        _state_0 ^= (_state_0 >> 15);
        _state_0 ^= (_state_0 << 27);

        _state_1 += BASE_STATE_1;

        return _state_0 + (_state_1 ^ (_state_1 >> 27));
    }

private:

    uint64_t _state_0 = BASE_STATE_0;
    uint64_t _state_1 = BASE_STATE_1;
};

} // namespace robin_details
