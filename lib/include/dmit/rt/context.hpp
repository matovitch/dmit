#pragma once

#include "dmit/rt/function_register.hpp"
#include "dmit/rt/process_stack.hpp"
#include "dmit/rt/library_core.hpp"
#include "dmit/rt/loop.hpp"

#include "dmit/vm/stack_op.hpp"
#include "dmit/vm/machine.hpp"
#include "dmit/vm/memory.hpp"

namespace dmit::rt
{

class Context
{

public:

    Context(ProcessStack         &,
            vm::Machine::Storage &);

    void run();

private:

    FunctionRegister _functionRegister;
    vm::Memory       _memory;

    vm::Machine _machine;
    Loop        _loop;
    LibraryCore _libraryCore;
};

} // namespace dmit::rt
