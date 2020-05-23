#pragma once

#include "dmit/vm/program.hpp"

#include <cstdint>

namespace dmit::vm
{

class StackCall
{

public:

    StackCall() = delete;

    StackCall(program::Counter* const head, const uint32_t size);

    void push(const program::Counter);

    void drop();

    program::Counter look() const;

    bool isEmpty() const;

private:

          program::Counter*       _head;
    const program::Counter* const _base;
    const program::Counter* const _tail;
};

} // namespace dmit::vm
