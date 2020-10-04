#include "dmit/rt/context.hpp"

#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/stack_op.hpp"

#include "dmit/com/assert.hpp"

#include <memory>

namespace dmit::rt
{

Context::Context(Storage& storage) :
    _machine{storage.machineStorage()},
    _loop{_machine, storage.processStack(), _functionRegister},
    _libraryCore{_machine._stack, _machine._memory, storage.processStack(), _functionRegister, _loop}
{
    _libraryCore.recordsIn(_functionRegister);
}

void Context::load(const vm::Program&         program,
                   const vm::program::Counter programCounter)
{
    _loop.load(program, programCounter);
}

void Context::run()
{
    _loop.run();
}

namespace context
{

void Storage::make(const std::size_t machineStackSize,
                   const std::size_t processStackSize,
                   const com::sha256::Seed& seed)
{
    _machineStorage.make(machineStackSize);
    _processStacks.push_back(std::make_unique<ProcessStack>(processStackSize, seed));
}

vm::Machine::Storage& Storage::machineStorage()
{
    return _machineStorage;
}

ProcessStack& Storage::processStack()
{
    DMIT_COM_ASSERT(!_processStacks.empty());
    return *(_processStacks.back());
}

} // namespace context
} // namespace dmit::rt
