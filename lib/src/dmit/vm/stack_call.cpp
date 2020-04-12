#include "dmit/vm/stack_call.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::vm
{

StackCall::StackCall(uint32_t* const head, const uint32_t size) :
    _head{head},
    _base{head - 1},
    _tail{head + size}
{}

void StackCall::push(const uint32_t address)
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

uint32_t StackCall::look() const
{
    return *_head;
}

} // namespace dmit::vm
