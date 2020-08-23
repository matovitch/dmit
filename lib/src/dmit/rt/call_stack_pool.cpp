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

vm::StackCall::Storage& CallStackPool::make()
{
    _stackStorages.emplace_back(std::make_unique<vm::StackCall::Storage>(_stackSize));

    return *(_stackStorages.back());
}

} // namespace dmit::rt
