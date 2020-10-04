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

    vm::Machine::Storage& machineStorage();

    ProcessStack& processStack();

private:

    std::vector<std::unique_ptr<ProcessStack>> _processStacks;
    vm::Machine::Storage _machineStorage;
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

    FunctionRegister _functionRegister;
    vm::Machine _machine;
    Loop        _loop;
    LibraryCore _libraryCore;
};

} // namespace dmit::rt
