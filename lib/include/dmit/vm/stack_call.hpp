#pragma once

#include "dmit/vm/program.hpp"

#include "dmit/com/storage.hpp"

#include <cstdint>
#include <vector>

namespace dmit::vm
{

class StackCall
{

public:

    using Storage = com::TStorage<program::Counter>;

    StackCall() = delete;

    StackCall(Storage&);

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
