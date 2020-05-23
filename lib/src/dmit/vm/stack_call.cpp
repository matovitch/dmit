#include "dmit/vm/stack_call.hpp"

#include "dmit/vm/program.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::vm
{

StackCall::StackCall(program::Counter* const head, const uint32_t size) :
    _head{head},
    _base{head - 1},
    _tail{head + size}
{}

void StackCall::push(const program::Counter address)
{
    *_head = address;
    _head++;

    DMIT_COM_ASSERT(_head < _tail);
}

void StackCall::drop()
{
    _head--;

    DMIT_COM_ASSERT(_head > _base);
}

program::Counter StackCall::look() const
{
    return *_head;
}

bool StackCall::isEmpty() const
{
    return _head - 1 == _base;
}

} // namespace dmit::vm
