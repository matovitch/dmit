#pragma once

#include <cstdint>

namespace dmit::vm
{

class StackOp
{

public:

    StackOp(uint64_t* const head, const uint32_t size);

    void push(const uint64_t);

    void drop();

    uint64_t look() const;

    int64_t offset() const;

private:

          uint64_t*       _head;
    const uint64_t* const _tail;
    const uint64_t* const _base;
};

} // namespace dmit::vm
