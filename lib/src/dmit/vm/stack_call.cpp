#include "dmit/vm/stack_call.hpp"

#include "dmit/vm/program.hpp"

#include "dmit/com/assert.hpp"

#include <cstdint>

namespace dmit::vm
{

StackCall::StackCall(Storage& storage) :
    _head{storage.data()},
    _base{_head - 1},
    _tail{_head + storage._size}
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
