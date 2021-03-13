#pragma once

#include <type_traits>
#include <cstdint>

namespace schmit_details
{

namespace coroutine
{

template <std::size_t SIZE>
class TStack
{
    // Note, the offset is needed to make room for
    // the return address used upon coroutine exit
    static constexpr std::size_t OFFSET = 0x08;
    static constexpr std::size_t ALIGN  = 0x10;

    static constexpr std::size_t ALIGNED_SIZE = SIZE - SIZE % ALIGN;

public:

    uint8_t* base()
    {
        return reinterpret_cast<uint8_t*>(&_memory) + ALIGNED_SIZE - OFFSET;
    }

private:

    std::aligned_storage_t<ALIGNED_SIZE, ALIGN> _memory;
};

} // namespace coroutine
} // namespace schmit_details
