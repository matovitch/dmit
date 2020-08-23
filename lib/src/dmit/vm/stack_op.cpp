#include "dmit/vm/stack_op.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::vm
{

StackOp::StackOp(Storage& storage) :
    _head{storage.data()},
    _base{_head - 1},
    _tail{_head + storage._size}
{}

uint64_t StackOp::look() const
{
    return *(_head - 1);
}

void StackOp::push(const uint64_t value)
{
    *_head = value;
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

const uint8_t* StackOp::asBytes() const
{
    return reinterpret_cast<const uint8_t*>(_head - 1);
}

} // namespace dmit::vm
