#pragma once

#include <cstdint>

namespace dmit::vm
{

class StackCall
{

public:

    StackCall(uint32_t* const head, const uint32_t size);

    void push(const uint32_t);

    void drop();

    uint32_t look() const;

private:

          uint32_t*       _head;
    const uint32_t* const _base;
    const uint32_t* const _tail;
};

} // namespace dmit::vm
