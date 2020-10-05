#include "dmit/rt/context.hpp"

#include "dmit/rt/process_stack.hpp"

#include "dmit/vm/stack_op.hpp"

#include "dmit/com/assert.hpp"

#include <memory>

namespace dmit::rt
{

Context::Context(Storage& storage) :
    _loop        {storage},
    _libraryCore {storage, _loop}
{}

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

    _machines          .push_back(std::make_unique<vm::Machine      >(_machineStorage));
    _processStacks     .push_back(std::make_unique<ProcessStack     >(processStackSize, seed));
    _functionRegisters .push_back(std::make_unique<FunctionRegister >());
}

vm::Machine& Storage::machine()
{
    DMIT_COM_ASSERT(!_machines.empty());
    return *(_machines.back());
}

ProcessStack& Storage::processStack()
{
    DMIT_COM_ASSERT(!_processStacks.empty());
    return *(_processStacks.back());
}

FunctionRegister& Storage::functionRegister()
{
    DMIT_COM_ASSERT(!_functionRegisters.empty());
    return *(_functionRegisters.back());
}

} // namespace context
} // namespace dmit::rt
