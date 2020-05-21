#include "dmit/rt/call_stack_pool.hpp"

#include "dmit/vm/stack_call.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace dmit::rt
{

CallStackPool::CallStackPool(const uint16_t stackSize) :
    _stackSize{stackSize}
{}

vm::StackCall& CallStackPool::make()
{
    _programCounters.emplace_back(_stackSize);

    _stacks.emplace_back(std::make_unique<vm::StackCall>(_programCounters.back().data(), _stackSize));

    return *(_stacks.back());
}

} // namespace dmit::rt
