#pragma once

#include "dmit/rt/function_register.hpp"

#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/core_library.hpp"
#include "dmit/rt/callable.hpp"

#include "dmit/vm/machine.hpp"
#include "dmit/vm/program.hpp"
#include "dmit/vm/memory.hpp"

#include "dmit/com/sha256.hpp"

#include <cstdint>

namespace dmit::rt
{

class Context
{

public:

    Context(const com::sha256::Seed& seed,
            const uint32_t stackSizeOp,
            const uint16_t stackSizeCall);

    void call(const vm::Program& program, const vm::program::Counter programCounter);

    void getProcessId ();
    void return_      ();

private:

    void run();

    std::vector<uint64_t> _stackAsU64;

    vm::StackOp _stack;
    vm::Memory  _memory;
    vm::Machine _machine;

    bool _isExiting = false;

    FunctionRegister _functionRegister;
    ProcessStack     _processStack;
    CoreLibrary      _coreLibrary;
};

} // namespace dmit::rt
