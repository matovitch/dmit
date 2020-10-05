#pragma once

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/library_core.hpp"
#include "dmit/rt/loop.hpp"

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/machine.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/sha256.hpp"

#include <memory>
#include <vector>

namespace dmit::rt
{

namespace context
{

class Storage
{

public:

    void make(const std::size_t machineStackSize,
              const std::size_t processStackSize,
              const com::sha256::Seed&);

    vm::Machine& machine();

    ProcessStack& processStack();

    FunctionRegister& functionRegister();

private:

    std::vector<std::unique_ptr<FunctionRegister >> _functionRegisters;
    std::vector<std::unique_ptr<ProcessStack     >> _processStacks;
    std::vector<std::unique_ptr<vm::Machine      >> _machines;
    vm::Machine::Storage                            _machineStorage;
};

} // namespace context

class Context
{

public:

    using Storage = context::Storage;

    Context(Storage&);

    void load(const vm::Program&,
              const vm::program::Counter);

    void run();

private:

    Loop             _loop;
    LibraryCore      _libraryCore;
};

} // namespace dmit::rt
