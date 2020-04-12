#include "dmit/vm/stack_op.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::vm
{

StackOp::StackOp(uint64_t* const head, const uint32_t size) :
    _head{head},
    _base{head - 1},
    _tail{head + size}
{}

uint64_t StackOp::look() const
{
    return *_head;
}

void StackOp::push(const uint64_t)
{
    _head++;
    DMIT_COM_ASSERT(_head < _tail);
}

void StackOp::drop()
{
    _head--;
    DMIT_COM_ASSERT(_head > _base);
}

int64_t StackOp::offset() const
{
    return static_cast<int64_t>(_base - _head);
}

} // namespace dmit::vm
